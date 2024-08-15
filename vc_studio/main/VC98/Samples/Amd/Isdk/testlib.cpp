/******************************************************************************

 Copyright (c) 1999 Advanced Micro Devices, Inc.

 LIMITATION OF LIABILITY:  THE MATERIALS ARE PROVIDED *AS IS* WITHOUT ANY
 EXPRESS OR IMPLIED WARRANTY OF ANY KIND INCLUDING WARRANTIES OF MERCHANTABILITY,
 NONINFRINGEMENT OF THIRD-PARTY INTELLECTUAL PROPERTY, OR FITNESS FOR ANY
 PARTICULAR PURPOSE.  IN NO EVENT SHALL AMD OR ITS SUPPLIERS BE LIABLE FOR ANY
 DAMAGES WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF PROFITS,
 BUSINESS INTERRUPTION, LOSS OF INFORMATION) ARISING OUT OF THE USE OF OR
 INABILITY TO USE THE MATERIALS, EVEN IF AMD HAS BEEN ADVISED OF THE POSSIBILITY
 OF SUCH DAMAGES.  BECAUSE SOME JURISDICTIONS PROHIBIT THE EXCLUSION OR LIMITATION
 OF LIABILITY FOR CONSEQUENTIAL OR INCIDENTAL DAMAGES, THE ABOVE LIMITATION MAY
 NOT APPLY TO YOU.

 AMD does not assume any responsibility for any errors which may appear in the
 Materials nor any responsibility to support or update the Materials.  AMD retains
 the right to make changes to its test specifications at any time, without notice.

 NO SUPPORT OBLIGATION: AMD is not obligated to furnish, support, or make any
 further information, software, technical information, know-how, or show-how
 available to you.

 So that all may benefit from your experience, please report  any  problems
 or  suggestions about this software to 3dsdk.support@amd.com

 AMD Developer Technologies, M/S 585
 Advanced Micro Devices, Inc.
 5900 E. Ben White Blvd.
 Austin, TX 78741
 3dsdk.support@amd.com

******************************************************************************

 test3d.cpp

  Excercises the functions of the library, performing some validity testing.
  Note that this code does NOT exhaustively test any of the SDK functions,
  and no formal verification of the code has been completed.

******************************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <float.h>
#include <math.h>

#include "amatrix.h"
#include "avector.h"
#include "atrans.h"
#include "aquat.h"
#include "amath.h"
#include "adsp.h"

void testComplexFIR();


/******************************************************************************
    Equality Tests (within epsilon is close enough - 23 bits)
 ******************************************************************************/
inline bool eq (float a, float b) { return _fabs (a-b) <= FLT_EPSILON*2; }

bool eq (const D3DVECTOR &a, const D3DVECTOR &b)
{
    return eq (a.x, b.x) && eq (a.y, b.y) && eq (a.z, b.z);
}

bool eq (const D3DTLVERTEX &a, const D3DTLVERTEX &b)
{
    return eq (a.sx, b.sx) && eq (a.sy, b.sy) && eq (a.sz, b.sz) && eq (a.rhw, b.rhw);
}

bool eq (const D3DRMQUATERNION &a, const D3DRMQUATERNION &b)
{
    return eq (a.s, b.s) && eq (a.v.y, b.v.y) && eq (a.v.z, b.v.z) && eq (a.v.x, b.v.x);
}

bool eq (const D3DMATRIX &a, const D3DMATRIX &b)
{
    return eq (a._11, b._11)
        && eq (a._12, b._12)
        && eq (a._13, b._13)
        && eq (a._14, b._14)
        && eq (a._21, b._21)
        && eq (a._22, b._22)
        && eq (a._23, b._23)
        && eq (a._24, b._24)
        && eq (a._31, b._31)
        && eq (a._32, b._32)
        && eq (a._33, b._33)
        && eq (a._34, b._34)
        && eq (a._41, b._41)
        && eq (a._42, b._42)
        && eq (a._43, b._43)
        && eq (a._44, b._44);
}


bool eq (const float *a, const float *b)
{
    return eq (a[0], b[0])
        && eq (a[1], b[1])
        && eq (a[2], b[2])
        && eq (a[3], b[3])
        && eq (a[4], b[4])
        && eq (a[5], b[5])
        && eq (a[6], b[6])
        && eq (a[7], b[7])
        && eq (a[8], b[8]);
}


/******************************************************************************
  Routine:  test
 ******************************************************************************/
void test (bool t, char *sub, char *fmt = 0, ...)
{
    if (t)
        printf ("passed:   ");
    else
        printf ("*FAILED*: ");
    printf (sub);
    if (!t && fmt)
    {
        va_list arg;
        va_start (arg, fmt);
        vfprintf (stdout, fmt, arg);
        va_end (arg);
    }
    printf ("\n");
}


/******************************************************************************
  Routine:  ulps
  Comment:  computes float "ulp"s, which are a unit of precision.  The
            algorithm discards the lower 21 bits of the mantissa, and then
            strips the exponent by way of throwing away the upper dword of
            data (a float is 64 bits, discarding 21 bits leaves you 43 bits,
            and the exponent is high 11 bits, leaving the low 32 bits as the
            mantissa).
            The difference in the two numbers is then converted to a floating
            point value and divided by 256 (to account for the 8 byte exponent
            of the float result).
            This yields the float-ulp difference in the two values.
            Note that for large ulps (>= 1e6 or so), the result will not be
            accurate.  The max ulp value is 2^24.
 ******************************************************************************/
float ulps (double x, double y)
{
    if (y > x)
    {
        double t = x;
        x = y;
        y = t;
    }
#if 1
    signed int err;
    __asm
    {
        mov     eax,DWORD PTR [x]
        mov     ebx,DWORD PTR [x+4]
        shrd    eax,ebx,21      // extract bits 21:53 of x
        mov     edx,DWORD PTR [y]
        mov     ecx,DWORD PTR [y+4]
        shrd    edx,ecx,21      // extract bits 21:53 of y
        sub     eax,edx         // subtract them
        mov     [err],eax
    }
#else
    unsigned err;
    double t = x - y;
    __asm
    {
        mov     eax,DWORD PTR[t]
        mov     edx,DWORD PTR[t+4]
        shrd    eax,edx,21
        mov     [err],eax
    }
#endif

    return float (double (err) / 256.0);
}


int precbits (double x, double y)
{
    int prec (24);
    float ref = (float)x;
    float val = (float)y;

    // Capture the mantissa bits, and compute their absolute difference
    unsigned delta = ((*(unsigned*)&ref) & 0x007fffff) - ((*(unsigned*)&val) & 0x007fffff);
    if (int (delta) < 0)
        delta = unsigned (-int (delta));

    // Compute the bits of precision in the delta
    while (prec != 0 && delta != 0)
    {
        --prec;
        delta >>= 1;
    }

//    return (1 << (24 - prec)) >> 1;
    return prec;
}



/******************************************************************************
  Routine:  test_precision
 ******************************************************************************/
void test_precision (const char *name,
                     float (*const func) (float),
                     double (*const ref_func) (double),
                     float from, float to, int steps)
{
    assert (from < to);

    // Iteration controls and parameters
    const int   precision (24);
    const float step ((to - from) / (float)steps);

    // Statistical data
    float min_prec (FLT_MAX);
    float min_val  (0);
    int   min_bits (24);
    float max_prec (0);
    float max_val  (0);
    int   max_bits (0);

    for (float f = from; f <= to; f += step)
    {
        float prec = ulps (ref_func (f), func (f));
        int   bits = precbits (ref_func (f), func (f));

        prec = (float)fabs(prec);

        // Accumulate statistics
        if (prec <= min_prec)
        {
            min_prec = prec;
            min_val  = f;
            min_bits = bits;
        }
        if (prec >= max_prec)
        {
            max_prec = prec;
            max_val  = f;
            max_bits = bits;
        }
    }

    printf ("%s\t(%3.1f, %3.1f)\t\t(%1.2f, %1.2f)\tbits (%d, %d)\n",
            name,
            from, to,
            min_prec, max_prec,
            min_bits, max_bits);
}



/******************************************************************************
  Routine:  test_precision2 - test precision of two input functions
 ******************************************************************************/
void test_precision2 (const char *name,
                      float (*const func) (float, float),
                      double (*const ref_func) (double, double),
                      float from, float to,
                      float from2, float to2, int steps)
{
    assert (from < to);
    assert (from2 < to2);

    // Iteration controls and parameters
    const int   precision (24);
    const float step ((to - from) / (float)steps);
    const float step2((to2 - from2) / (float)steps);

    // Statistical data
    float min_prec (FLT_MAX);
    float max_prec (FLT_MIN);
    float min_val  (from);
    float max_val  (from);

    for (float f2 = from2; f2 <= to2; f2 += step)
    {
        for (float f = from; f <= to; f += step)
        {
            float prec = ulps (ref_func (f, f2), func (f, f2));

            // Accumulate statistics
            if (prec < min_prec)
            {
                min_prec = prec;
                min_val  = f;
            }
            if (prec > max_prec)
            {
                max_prec = prec;
                max_val  = f;
            }
        }
    }

    printf ("%s\tulp range from %g to %g over (%g, %g)\n",
            name,
            min_prec, max_prec, from, to);
}



/******************************************************************************
  Routine:  test_amatix
 ******************************************************************************/
void test_amatrix ()
{
// static test data
static D3DMATRIX mat = { 1,  2,  3,  4,
                         5,  6,  7,  8,
                         9,  10, 11, 12,
                         13, 14, 15, 16 };
static D3DMATRIX mat2= { 1, 0, 0, 0,
                         0, 1, 0, 0,
                         0, 0, 1, 0,
                         0, 0, 0, 1 };

    D3DMATRIX   r4;
    float       a3[9], b3[9], r3[9];

//--- Test _glMul_3x3
    glMul_3x3 (r3, a3, b3);
    test (true, "glMul_3x3");
    _glMul_3x3 (r3, a3, b3);
    test (true, "_glMul_3x3");


//--- Test _glMul_4x4
    glMul_4x4 ((float *)&r4, (float *)&mat, (float *)&mat2);
    test (eq (r4, mat), "glMul_4x4");
    _glMul_4x4 ((float *)&r4, (float *)&mat, (float *)&mat2);
    test (eq (r4, mat), "_glMul_4x4");


//--- Test _D3DMul_3x3
//    _D3DMul_3x3 (r3, a3, b3);
//    test (true, "_D3DMul_3x3");

//--- Test _D3DMul_4x4
    D3DMul_4x4 (r4, mat, mat2);
    test (eq (r4, mat), "D3DMul_4x4");
    _D3DMul_4x4 (r4, mat, mat2);
    test (eq (r4, mat), "_D3DMul_4x4");
}


/******************************************************************************
  Routine:  test_vector (fully tested)
 ******************************************************************************/
void test_avector ()
{
static D3DVECTOR vec000 = { 0.0, 0.0, 0.0 };
static D3DVECTOR vec012 = { 0.0, 1.0, 2.0 };
static D3DVECTOR vec111 = { 1.0, 1.0, 1.0 };
static D3DVECTOR vec123 = { 1.0, 2.0, 3.0 };
static D3DVECTOR vec149 = { 1.0, 4.0, 9.0 };
static D3DVECTOR vec234 = { 2.0, 3.0, 4.0 };
static D3DVECTOR vec246 = { 2.0, 4.0, 6.0 };
static D3DVECTOR vec100 = { 1.0, 0.0, 0.0 };
static D3DVECTOR vec010 = { 0.0, 1.0, 0.0 };
static D3DVECTOR vec001 = { 0.0, 0.0, 1.0 };

    D3DVECTOR res;
    float f;

//--- Test _add_vect
    add_vect  (&res, &vec123, &vec123);
    test (eq (res, vec246), "add_vect", " (%f, %f, %f)", res.x, res.y, res.z);

    _add_vect  (&res, &vec123, &vec123);
    test (eq (res, vec246), "_add_vect", " (%f, %f, %f)", res.x, res.y, res.z);

//--- Test _sub_vect
    sub_vect  (&res, &vec123, &vec123);
    test (eq (res, vec000), "sub_vect", " (%f, %f, %f)", res.x, res.y, res.z);

    _sub_vect  (&res, &vec123, &vec123);
    test (eq (res, vec000), "_sub_vect", " (%f, %f, %f)", res.x, res.y, res.z);

//--- Test _mult_vect
    mult_vect (&res, &vec123, &vec123);
    test (eq (res, vec149), "mult_vect", " (%f, %f, %f)", res.x, res.y, res.z);

    _mult_vect (&res, &vec123, &vec123);
    test (eq (res, vec149), "_mult_vect", " (%f, %f, %f)", res.x, res.y, res.z);

//--- Test _scale_vect
    scale_vect(&res, &vec123, 2.0);
    test (eq (res, vec246), "scale_vect", " (%f, %f, %f)", res.x, res.y, res.z);

    _scale_vect(&res, &vec123, 2.0);
    test (eq (res, vec246), "_scale_vect", " (%f, %f, %f)", res.x, res.y, res.z);

//-- Test _mag_vect
    f = mag_vect (&vec111);
    test (eq (f, (float)sqrt (3.0f)), "mag_vect", " (%f)", f);

    f = _mag_vect (&vec111);
    test (eq (f, _sqrt (3.0f)), "_mag_vect", " (%f)", f);

//--- Test _norm_vect
    norm_vect (&res, &vec246);
    f = mag_vect (&res);
    test (eq (f, 1.0f), "norm_vect", " (%f)", f);

    _norm_vect (&res, &vec246);
    f = _mag_vect (&res);
    test (eq (f, 1.0f), "_norm_vect", " (%f)", f);

//--- Test _dot_vect
    f = dot_vect  (&vec100, &vec010);
    test (eq (f, 0.0f), "dot_vect", " (%f)", f);

    f = _dot_vect  (&vec100, &vec010);
    test (eq (f, 0.0f), "_dot_vect", " (%f)", f);

//--- Test _cross_vect (X cross Y == Z)
    cross_vect(&res, &vec100, &vec010);
    test (eq (res, vec001), "cross_vect", " (%f, %f, %f)", res.x, res.y, res.z);

    _cross_vect(&res, &vec100, &vec010);
    test (eq (res, vec001), "_cross_vect", " (%f, %f, %f)", res.x, res.y, res.z);

//  a_mag_vect();      /* eax -> mm0        */
//  a_dot_vect();      /* eax * edx -> mm0  */
}


/******************************************************************************
  Routine:  test_atrans (fully tested)
 ******************************************************************************/
void test_atrans ()
{
    D3DTLVERTEX res, expected, pt, pts[10], ress[10];
    D3DMATRIX   mtx;
    float res3[3], pt3[3], expected3[3], pts3[3*10], ress3[3*10], mtx3[9];
    int i;
    bool success;

//--- Test _D3DTrans_4x4
    mtx._11 = 1;    mtx._12 = 0;    mtx._13 = 0;    mtx._14 = 0;
    mtx._21 = 0;    mtx._22 = 0;    mtx._23 = 1;    mtx._24 = 0;
    mtx._31 = 0;    mtx._32 = 1;    mtx._33 = 0;    mtx._34 = 0;
    mtx._41 =10;    mtx._42 =10;    mtx._43 =10;    mtx._44 = 1;
    pt.sx = 1;
    pt.sy = 2;
    pt.sz = 3;
    pt.rhw = 1;
    expected.sx = 11;
    expected.sy = 13;
    expected.sz = 12;
    expected.rhw = 1;
    D3DTrans_4x4 (res, pt, mtx);
    test (eq (res, expected), "D3DTrans_4x4");
    _D3DTrans_4x4 (res, pt, mtx);
    test (eq (res, expected), "_D3DTrans_4x4");

//--- Test _D3DTrans_a4x4
    for (i = 0; i < 10; i++)
        pts[i] = pt;
    D3DTrans_a4x4 (ress, pts, mtx, sizeof (pts) / sizeof (D3DTLVERTEX));
    success = true;
    for (i = 0; success && i < 10; i++)
        success = eq (ress[i], expected);
    test (success, "D3DTrans_a4x4");
    _D3DTrans_a4x4 (ress, pts, mtx, sizeof (pts) / sizeof (D3DTLVERTEX));

    success = true;
    for (i = 0; success && i < 10; i++)
        success = eq (ress[i], expected);
    test (success, "_D3DTrans_a4x4");

//--- Test _glTrans_4x4
    mtx._11 = 1;    mtx._12 = 0;    mtx._13 = 0;    mtx._14 = 10;
    mtx._21 = 0;    mtx._22 = 0;    mtx._23 = 1;    mtx._24 = 10;
    mtx._31 = 0;    mtx._32 = 1;    mtx._33 = 0;    mtx._34 = 10;
    mtx._41 = 0;    mtx._42 = 0;    mtx._43 = 0;    mtx._44 = 1;
    glTrans_4x4 ((float *)&res, (float *)&pt, (float *)&mtx);
    test (eq (res, expected), "glTrans_4x4");
    _glTrans_4x4 ((float *)&res, (float *)&pt, (float *)&mtx);
    test (eq (res, expected), "_glTrans_4x4");

//--- Test _glTrans_a4x4
    for (i = 0; i < 10; i++)
        pts[i] = pt;
    glTrans_a4x4 ((float *)&ress, (float *)&pts, (float *)&mtx, sizeof (pts) / sizeof (D3DTLVERTEX));
    success = true;
    for (i = 0; success && i < 10; i++)
        success = eq (ress[i], expected);
    test (success, "glTrans_a4x4");
    _glTrans_a4x4 ((float *)&ress, (float *)&pts, (float *)&mtx, sizeof (pts) / sizeof (D3DTLVERTEX));
    success = true;
    for (i = 0; success && i < 10; i++)
        success = eq (ress[i], expected);
    test (success, "_glTrans_a4x4");

//--- Test D3DTrans_3x3
    pt3[0] = 1;
    pt3[1] = 2;
    pt3[2] = 3;
    expected3[0] = 1;
    expected3[1] = 3;
    expected3[2] = 2;
    mtx3[0] = 1;    mtx3[1] = 0;    mtx3[2] = 0;
    mtx3[3] = 0;    mtx3[4] = 0;    mtx3[5] = 1;
    mtx3[6] = 0;    mtx3[7] = 1;    mtx3[8] = 0;
    D3DTrans_3x3(res3, pt3, mtx3);
    test (eq (res3[0], expected3[0]) && eq (res3[1], expected3[1]) &&eq (res3[2], expected3[2]),
          "D3DTrans_3x3");
    _D3DTrans_3x3(res3, pt3, mtx3);
    test (eq (res3[0], expected3[0]) && eq (res3[1], expected3[1]) &&eq (res3[2], expected3[2]),
          "_D3DTrans_3x3");

//--- Test D3DTrans_a3x3
    for (i = 0; i < 10; i++)
    {
        const int idx = i*3;
        pts3[idx+0] = pt3[0];
        pts3[idx+1] = pt3[1];
        pts3[idx+2] = pt3[2];
    }
    D3DTrans_a3x3 (ress3, pts3, mtx3, sizeof (pts3) / sizeof (float[3]));
    success = true;
    for (i = 0; success && i < 10; i++)
    {
        const int idx = i * 3;
        success = eq (ress3[idx+0], expected3[0]) && eq (ress3[idx+1], expected3[1]) &&eq (ress3[idx+2], expected3[2]);
    }
    test (success, "D3DTrans_a3x3");

//--- Test _D3DTrans_a3x3
    _D3DTrans_a3x3 (ress3, pts3, mtx3, sizeof (pts3) / sizeof (float[3]));
    success = true;
    for (i = 0; success && i < 10; i++)
    {
        const int idx = i * 3;
        success = eq (ress3[idx+0], expected3[0]) && eq (ress3[idx+1], expected3[1]) &&eq (ress3[idx+2], expected3[2]);
    }
    test (success, "_D3DTrans_a3x3");


//--- Test _glTrans_3x3
    // pt3 && expected3 set above
    mtx3[0] = 1;    mtx3[1] = 0;    mtx3[2] = 0;
    mtx3[3] = 0;    mtx3[4] = 0;    mtx3[5] = 1;
    mtx3[6] = 0;    mtx3[7] = 1;    mtx3[8] = 0;
    glTrans_3x3(res3, pt3, mtx3);
    test (eq (res3[0], expected3[0]) && eq (res3[1], expected3[1]) &&eq (res3[2], expected3[2]),
          "glTrans_3x3");
    _glTrans_3x3(res3, pt3, mtx3);
    test (eq (res3[0], expected3[0]) && eq (res3[1], expected3[1]) &&eq (res3[2], expected3[2]),
          "_glTrans_3x3");

//--- Test glTrans_a3x3
    glTrans_a3x3 (ress3, pts3, mtx3, sizeof (pts3) / sizeof (float[3]));
    success = true;
    for (i = 0; success && i < 10; i++)
    {
        const int idx = i * 3;
        success = eq (ress3[idx+0], expected3[0]) && eq (ress3[idx+1], expected3[1]) &&eq (ress3[idx+2], expected3[2]);
    }
    test (success, "glTrans_a3x3");

    _glTrans_a3x3 (ress3, pts3, mtx3, sizeof (pts3) / sizeof (float[3]));
    success = true;
    for (i = 0; success && i < 10; i++)
    {
        const int idx = i * 3;
        success = eq (ress3[idx+0], expected3[0]) && eq (ress3[idx+1], expected3[1]) &&eq (ress3[idx+2], expected3[2]);
    }
    test (success, "_glTrans_a3x3");

//  _jpeg_fdct(&);

}


#define PI 3.1415927
/******************************************************************************
  Routine:  test_quat
 ******************************************************************************/
void test_quat ()
{
    const D3DRMQUATERNION q1 = { .0f, { .1f, .2f, .3f } };
    const D3DRMQUATERNION q2 = { .1f, { .2f, .3f, .4f } };
    const D3DRMQUATERNION q3 = { .1f, { .3f, .5f, .7f } };
    const D3DRMQUATERNION qx = { (float)sin(45.0*PI/180.0), { (float)sin(45.0*PI/180.0), 0.0f, 0.0f } };
    const D3DRMQUATERNION qy = { (float)sin(45.0*PI/180.0), { 0.0f, (float)sin(45.0*PI/180.0), 0.0f } };
    const D3DRMQUATERNION qxy= { 0.5f, { 0.5f, 0.5f, 0.5f } };
    const float angleXY[3]   = { 90.0f, 0.0f, 90.0f };  // should match qxy
    D3DRMQUATERNION r, a, b;
    D3DMATRIX mtx, mtx2;
    D3DVECTOR v1, v2;
    float angles[4];

//--- Test _add_quat
    add_quat (&r, &q1, &q2);
    test (eq (r, q3), "add_quat");
    _add_quat (&r, &q1, &q2);
    test (eq (r, q3), "_add_quat");

//--- Test sub_quat
    sub_quat (&r, &q3, &q2);
    test (eq (r, q1), "sub_quat");
    _sub_quat (&r, &q3, &q2);
    test (eq (r, q1), "_sub_quat");

//--- Test D3DMat2quat
    // 90 degree rotation around X axis (a simple test, but somewhat effective)
    mtx._11 = 1;    mtx._12 = 0;    mtx._13 = 0;    mtx._14 = 0;
    mtx._21 = 0;    mtx._22 = 0;    mtx._23 = -1;   mtx._24 = 0;
    mtx._31 = 0;    mtx._32 = 1;    mtx._33 = 0;    mtx._34 = 0;
    mtx._41 = 0;    mtx._42 = 0;    mtx._43 = 0;    mtx._44 = 1;
    D3DMat2quat (&a, &mtx);
    test (eq (a, qx), "D3DMat2quat");
    quat2D3DMat (&mtx2, &a);
    test (eq (mtx, mtx2), "quat2D3DMat");
    _quat2D3DMat (&mtx2, &a);
    test (eq (mtx, mtx2), "_quat2D3DMat");

#if 0
    printf ("%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n",
            mtx2._11, mtx2._12, mtx2._13, mtx2._14,
            mtx2._21, mtx2._22, mtx2._23, mtx2._24,
            mtx2._31, mtx2._32, mtx2._33, mtx2._34,
            mtx2._41, mtx2._42, mtx2._43, mtx2._44);
#endif

//--- Test glMat2quat
    // 90 degree rotation around X axis
    mtx._11 = 1;    mtx._12 = 0;    mtx._13 = 0;    mtx._14 = 0;
    mtx._21 = 0;    mtx._22 = 0;    mtx._23 = 1;    mtx._24 = 0;
    mtx._31 = 0;    mtx._32 = -1;   mtx._33 = 0;    mtx._34 = 0;
    mtx._41 = 0;    mtx._42 = 0;    mtx._43 = 0;    mtx._44 = 1;
    glMat2quat (&a, (float *)&mtx);
    test (eq(a,qx), "glMat2quat");
    quat2glMat ((float *)&mtx2, &a);
    test (eq(mtx, mtx2), "quat2glMat");

//--- Test mult_quat
    mult_quat (&a, &qx, &qy);
//    printf ("%f {%f, %f, %f}\n", a.s, a.v.x, a.v.y, a.v.z);
    test (eq (a, qxy), "mult_quat");

    _mult_quat (&b, &qx, &qy);
    test (eq (a, b), "_mult_quat");

//--- Test norm_quat
    norm_quat (&r, &q2);
    test (eq (r.s*r.s + r.v.x*r.v.x + r.v.y*r.v.y + r.v.z*r.v.z, 1.0f), "norm_quat");
    _norm_quat(&r, &q2);
    test (eq (r.s*r.s + r.v.x*r.v.x + r.v.y*r.v.y + r.v.z*r.v.z, 1.0f), "_norm_quat");

//--- Test euler2quat
    euler2quat  (&a, angleXY);
//    printf ("%f {%f, %f, %f}\n", a.s, a.v.x, a.v.y, a.v.z);
    test (eq (a, qxy), "euler2quat");

    _euler2quat (&a, angleXY);
    test (eq (a, qxy), "_euler2quat");
    printf ("--->Failure acceptable, due to precision of sin/cos routines\n");

    euler2quat2 (&a, angleXY);
    test (eq (a, qxy), "euler2quat2");

//--- Test quat2axis_angle
    quat2axis_angle (angles, &a);
    test (true, "quat2axis_angle");
    _quat2axis_angle (angles, &a);
    test (true, "_quat2axis_angle");

//--- Test axis_angle2quat
    axis_angle2quat (&a, angles);
    test (true, "axis_angel2quat");
    _axis_angle2quat (&a, angles);
    test (true, "_axis_angle2quat");

//--- Test slerp_quat
    slerp_quat (&r, &a, &b, 0.5f);
    test (true, "slerp_quat");
    _slerp_quat (&r, &a, &b, 0.5f);
    test (true, "_slerp_quat");


//--- Test trans_quat
    trans_quat (&v2, &v1, &a);
    test (true, "trans_quat");
    _trans_quat (&v2, &v1, &a);
    test (true, "_trans_quat");
}


/******************************************************************************
  Routine:  test_math (fully tested)
 ******************************************************************************/
#define make_test(f,l,h,s) test_precision (#f, _##f, f, l,h,s)
#define make_test2(f,l,h,l2,h2,s) test_precision (#f, _##f, f, l,h,l2,h2,s)
void test_math ()
{
    const float pi_4 = 3.1415927f / 4.0f;
    float angle=1.52f;
    float a, b;

    printf ("ulp magnitude is related to bit precision, with lower being more precise.\n"
            "The ulp will be between (1 << (23 - bits)) and (1 << (24 - bits))\n");
    printf ("name\trange\t\t\tulp (min, max)\tprecision(max, min)\n");

//--- Test transcendentals
    make_test (tan, -pi_4,pi_4,3600*2);
    make_test (cos, -pi_4,pi_4,3600*2);
    make_test (sin, -pi_4,pi_4,3600*2);
    make_test (atan, -1.0f,1.0f,3600*2);
    make_test (acos, -1.0f,1.0f,3600*2);
    make_test (asin, -1.0f,1.0f,3600*2);

//--- Test exponentials
    make_test (log,   0.01f,1000.0f,10000);
    make_test (log10, 0.01f,1000.0f,10000);
    make_test (exp, -20.0f,20.0f,10000);
    make_test (sqrt, 0.0f,10000.0f,10000);

//--- Test rounding, etc.
    make_test (fabs,  -100.0f,100.0f,1000*2);
    make_test (ceil,  -100.0f,100.0f,1000*2);
    make_test (floor, -100.0f,100.0f,1000*2);

//--- Test _frexp
    double  fdx=16.4;
    int     fix, fix2;
    a = (float)frexp (fdx,&fix);
    b = _frexp ((float)fdx,&fix2);
    test (eq (a, b) && fix == fix2, "_frexp", " %f+E%d == %f+E%d", a, fix, b, fix2);

//--- Test _ldexp
    fdx = 4.0f;
    fix = 3;
    a = (float)ldexp (fdx, fix);
    b = _ldexp ((float)fdx, fix);
    test (eq (a, b), "_ldexp", " %f == %f", a, b);

//--- Test _modf
    double number = 15.5;
    double  whole=0;
    a=(float)modf (number,&whole);

    float _number = (float)number;
    float _whole = (float)whole;
    b = _modf ((float)number,&_whole);
    test (eq (a, b) && whole == (float)_whole, "_modf", " <%f,%f> == <%f,%f>", a, (float)whole, b, _whole);

//--- Test _fmod
    a = (float)fmod (-10.0f, 2.5f);
    b = (float)_fmod (-10.0f, 2.5f);
    test (eq (a, b), "_fmod", " %f == %f", a, b);


//--- Test _sincos
    float sc[2];
    _sincos (angle,&sc[0]);
    a = _cos (angle);
    b = _sin (angle);
    test (eq (sc[0], a) && eq (sc[1], b), "sincos", " <%f,%f> == <%f,%f>", a, b, sc[0], sc[1]);

    a = (float)pow(2,8);
    b = _pow(2,8);
    test (eq (a, b), "_pow", " %f == %f", a, b);
}


/******************************************************************************
  Routine:  test_dsp
 ******************************************************************************/
void test_dsp ()
{
#if 1
    const   int nsamps = 5;
    float   a;//, b;
    float   mm[2], out[nsamps*2];
    static  float samples[nsamps] = { 0.0f, 0.5f, -0.35f, -0.1f, 0.2f };
    static  float bsamples[nsamps] = { 1.0f, 1.5f, 1.0f-0.35f, 1.0f-0.1f, 1.2f };
    static  float sqsamples[nsamps] = { 0.0f, 0.25f, 0.35f * 0.35f, 0.01f, 0.2f * 0.2f };

#if 0
float   _iir    (float input, int n, float *coef, float *history);
float   _fir    (float input, int n, float *coef, float *history);
void    _firBlock(int n, float *input, float *output, int m, float *history);
void    _convolve(int n, float *input1, float *input2, float *output);
void    _fftInit(int, COMPLEX *, int);
void    _fft    (int, COMPLEX *, COMPLEX *, int);
#endif

//--- Test _average
    a = _average (nsamps, samples);
    test (true, "_average", " %f", a);

//--- Test _rms
    a = _rms (nsamps, samples);
    test (true, "_rms", " %f", a);

//--- Test _bias
    memcpy (out, samples, nsamps * sizeof (float));
    _bias (nsamps, out, -1.0f);
    test (0 == memcmp (out, bsamples, nsamps + sizeof (float)), "_bias");

//--- Test _v_mult
    _v_mult (nsamps, samples, samples, out);
    test (0 == memcmp (out, sqsamples, nsamps + sizeof (float)), "_v_mult");

//--- Test _minmax
    _minmax (nsamps, samples, mm);
    test (eq (mm[1], -0.35f) && eq (mm[0], 0.5f), "_minmax", " %f, %f", mm[0], mm[1]);
//  a = _bessel (???);

// fastcall routines - all of these are called by the above routines, so they
//                     don't need to be checked.
//
//  (registers are in same order as corresponding parameters above)
//  a_average();    // ecx * eax -> mm0
//  a_rms();        // ecx * eax -> mm0
//  a_v_mult(); // ecx * eax * edx * edi -> ()
//  a_minmax(); // ecx * eax -> mm0 (max|min)
//  a_bessel(); // mm0 -> mm0

#if 0
    static short samps[8] = { 0, 0x0f00, 0x0ff0, 0xf000, 0x7fff, 0xffff, 0x8000, 0x0f0f };

    _cvt_16bit (7, samps, out);
    for (int i = 0; i < 7; i++)
    {
        printf ("%d -> %f\n", samps[i], out[i]);
    }
#endif

    // _complexFIR test
    testComplexFIR();
#endif
}

#if 1

/*
** testComplexBlockFIR(): call the asm K6-2 real/complexFIR() routine and display timing stats.
**	NB: The "cyclesPerMAC" value requires that the constant MHZ is set to the
**	clock rate of the target machine (see firbal.h)
*/
#define	NDATA 100			// number of data elements to generate
#define NTAPS 36			// number of real taps in FIR
#define LOOP_COUNT 10L		// number of calls to target FIR per run
void testComplexFIR()
{
    CFifoData   fifoData;
	int         nCoeff = NTAPS, nData = 100;
	int         i, err;
	int         bPass = 0;
	
/*
initialize for tiny matlab test sequence:
	x = -1+j4, 2+5j, -3+6j
	c = 1
	output = x = -1+j4, 2+5j, -3+6j

	c = 1+1j -2+3j  4-5j  6+7j
	output = -5+3j  -13-4j  -12+20j

*/
	static COMPLEX complexData[NDATA] = {
		{-1.0, 4.0 },
		{ 2.0, 5.0 },
		{ -3.0, 6.0 }
	};

	
	static COMPLEX coefs[36] = {
		{ 1.0, 1.0 },
		{ -2.0, 3.0 },
		{ 4.0, -5.0 },
		{ 6.0, 7.0 }
	};

    // results using the following data and coefficients
	// can be computed in MatLAB with this code:
    /*
        % complexFIR test routine
        echo
        c=[1:36]+j*[-35:0]; % synthetic complex 36-tap filter
        d=[1:100]+j*[-50:49]; % synthetic complex data
        f=filter(c,1,d) % filter...
    */
	for (i = 0; i<NDATA; i++)
	{
		complexData[i].real = (float)(1+i);  // 1..100
		complexData[i].imag = (float)(-50+i); // -50..49
	}

	for (i=0; i<36; i++)
	{
		coefs[i].real = (float)(1+i);  // 1..36
		coefs[i].imag = (float)(-35+i); // -35..0
	}
    nData = 100;

    if (err = _initCFifoData (&fifoData, NTAPS, (float *)coefs))
    {
    	printf("_complexFIR: failed _initCFifoData()\n");
        return;
    }

    for (i=0; i<LOOP_COUNT; i++)
    {
        _complexFIR (complexData, nData, &fifoData);

        // This is a spot-check based on MatLAB computed results.
        // Note that the LOOP_COUNT iteration causes this to settle, minimizing the
        // effects of the uninitialized history buffer.
        if (i==0)
            bPass = (complexData[99].real == 74790.0 && complexData[99].imag == -38766.0);
	}
    _releaseCFifoData (&fifoData);
    test (bPass != 0, "_complexFIR");
}

#endif


void init_idsp();
void init_imath();
void init_iquat();

/******************************************************************************
  Routine:  main
 ******************************************************************************/
int main (int argc, char* argv[])
{
    init_idsp();
    init_imath();
    init_iquat();

    printf ("\n*** matrix routines ***\n");
    test_amatrix();

    printf ("\n*** vector routines ***\n");
    test_avector();

    printf ("\n*** translation routines ***\n");
    test_atrans();

    printf ("\n*** quaternion routines ***\n");
    test_quat();

    printf ("\n*** math routines ***\n");
    test_math();

    printf ("\n*** dsp routines ***\n");
    test_dsp();

    printf ("\n*** fin ***\n");

    return 0;
}

// eof
