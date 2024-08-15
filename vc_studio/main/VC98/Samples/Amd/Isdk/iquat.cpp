//;******************************************************************************
//;
//; Copyright (c) 1999 Advanced Micro Devices, Inc.
//;
//; LIMITATION OF LIABILITY:  THE MATERIALS ARE PROVIDED *AS IS* WITHOUT ANY
//; EXPRESS OR IMPLIED WARRANTY OF ANY KIND INCLUDING WARRANTIES OF MERCHANTABILITY,
//; NONINFRINGEMENT OF THIRD-PARTY INTELLECTUAL PROPERTY, OR FITNESS FOR ANY
//; PARTICULAR PURPOSE.  IN NO EVENT SHALL AMD OR ITS SUPPLIERS BE LIABLE FOR ANY
//; DAMAGES WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF PROFITS,
//; BUSINESS INTERRUPTION, LOSS OF INFORMATION) ARISING OUT OF THE USE OF OR
//; INABILITY TO USE THE MATERIALS, EVEN IF AMD HAS BEEN ADVISED OF THE POSSIBILITY
//; OF SUCH DAMAGES.  BECAUSE SOME JURISDICTIONS PROHIBIT THE EXCLUSION OR LIMITATION
//; OF LIABILITY FOR CONSEQUENTIAL OR INCIDENTAL DAMAGES, THE ABOVE LIMITATION MAY
//; NOT APPLY TO YOU.
//;
//; AMD does not assume any responsibility for any errors which may appear in the
//; Materials nor any responsibility to support or update the Materials.  AMD retains
//; the right to make changes to its test specifications at any time, without notice.
//;
//; NO SUPPORT OBLIGATION: AMD is not obligated to furnish, support, or make any
//; further information, software, technical information, know-how, or show-how
//; available to you.
//;
//; So that all may benefit from your experience, please report  any  problems
//; or  suggestions about this software to 3dsdk.support@amd.com
//;
//; AMD Developer Technologies, M/S 585
//; Advanced Micro Devices, Inc.
//; 5900 E. Ben White Blvd.
//; Austin, TX 78741
//; 3dsdk.support@amd.com
//;
//;******************************************************************************
//;
//; QUAT.ASM
//;
//; AMD3D 3D library code: Quaternion operations.
//;
//; See quat_lib.c for algorithm details and quat.h for function prototypes.
//;
//; Quaternions are represented as four float values, <x,y,z,w>
//;
//;******************************************************************************

#include <math.h>
#include "amath.h"
#include "aquat.h"

#pragma warning (disable:4799)
#pragma warning (disable:4730)

//;******************************************************************************
//; Defines:  Parameter offsets for all routines
//;******************************************************************************

#define MATRIX_D_00 0
#define MATRIX_D_01 1*4
#define MATRIX_D_02 2*4
#define MATRIX_D_03 3*4
#define MATRIX_D_10 4*4
#define MATRIX_D_11 5*4
#define MATRIX_D_12 6*4
#define MATRIX_D_13 7*4
#define MATRIX_D_20 8*4
#define MATRIX_D_21 9*4
#define MATRIX_D_22 10*4
#define MATRIX_D_23 11*4
#define MATRIX_D_30 12*4
#define MATRIX_D_31 13*4
#define MATRIX_D_32 14*4
#define MATRIX_D_33 15*4

// A "magic" type to allow initialization with any type
template <class T>
union __m64t
{
    T       t;
    __m64   m64;

    operator __m64 ()       { return m64; }
    operator __m64 () const { return m64; }
};

// An instantiation for initializing with a pair of integers
typedef __m64t<int[2]> __m64i;

#define _m_from_floats(f) (*(__m64 *)(f))
#define _m_to_float(f)  ((f).m64_f32[0])
#define _m_from_float(f)  _m_from_int (*(int *)&(f))


//;******************************************************************************
//; Some handy constants
//;******************************************************************************

// 3DNow! constants
const __m64i    NEG2     = { 0x80000000, 0 };

__m64     TWOS;
__m64     ONES;
__m64     HALFS;
__m64     DEG2RAD2;
__m64     RAD2DEG2;

void init_iquat()
{
	TWOS.m64_f32[0] = 2.0f;
	TWOS.m64_f32[1] = 2.0f;
	ONES.m64_f32[0] = 1.0f;
	ONES.m64_f32[1] = 1.0f;
	HALFS.m64_f32[0] = 0.5f;
	HALFS.m64_f32[1] = 0.5f;
	DEG2RAD2.m64_f32[0] = 0.0087266462599716478846184538f;
	DEG2RAD2.m64_f32[1] = 0.0087266462599716478846184538f;
	RAD2DEG2.m64_f32[0] = 114.59155902616464175359630962f;
	RAD2DEG2.m64_f32[1] = 114.59155902616464175359630962f;
}

// scalar constants
const float DELTA_1 = -0.999f;
const float DELTA_2 = 0.999f;
const float HALF_PI = 1.570796f;

//;******************************************************************************
//; Routine:  void _add_quat (D3DRMQUATERNION *r, const D3DRMQUATERNION *a, const D3DRMQUATERNION *b)
//; Input:    r - resultant quaternion
//;           a - input quaternion
//;           b - input quaternion
//; Output:   r[i] = a[i] + b[i]
//; Uses:     eax,edx,mm0,mm1
//; Comment:
//;   Quaternion addition is defined as (aw + bw, ax + bx, ay + by, az + bz)
//;   This routine is heavily memory i/o bound, and uses the load-execute
//;   instruction model (efficient on K7, not too bad on K6).
//;******************************************************************************
void _add_quat (D3DRMQUATERNION *r, const D3DRMQUATERNION *a, const D3DRMQUATERNION *b)
{
    _m_femms();
    ((__m64 *)r)[0] = _m_pfadd (((__m64 *)a)[0], ((__m64 *)b)[0]);
    ((__m64 *)r)[1] = _m_pfadd (((__m64 *)a)[1], ((__m64 *)b)[1]);
    _m_femms();
}


//;******************************************************************************
//; Routine:  void _sub_quat (D3DRMQUATERNION *r, const D3DRMQUATERNION *a, const D3DRMQUATERNION *b)
//; Input:    r - resultant quaternion
//;           a - input quaternion
//;           b - input quaternion
//; Output:   r[i] = a[i] - b[i]
//; Uses:     eax, edx, mm0, mm1
//; Comment:
//;   Quaternion subtraction is defined as (aw - bw, ax - bx, ay - by, az - bz)
//;   This routine is heavily memory i/o bound, and uses the load-execute
//;   instruction model (efficient on K7, not too bad on K6).
//;******************************************************************************
void _sub_quat (D3DRMQUATERNION *r, const D3DRMQUATERNION *a, const D3DRMQUATERNION *b)
{
    _m_femms();
    ((__m64 *)r)[0] = _m_pfsub (((__m64 *)a)[0], ((__m64 *)b)[0]);
    ((__m64 *)r)[1] = _m_pfsub (((__m64 *)a)[1], ((__m64 *)b)[1]);
    _m_femms();
}


//;******************************************************************************
//; Routine:  void _mult_quat (D3DRMQUATERNION *r, const D3DRMQUATERNION *a, const D3DRMQUATERNION *b)
//; Input:    r - resultant quaternion
//;           a - input quaternion
//;           b - input quaternion
//; Output:   r = a * b, using the definition of quaternion multiplication
//; Uses:     eax, ecx, edx, mm0-mm7
//; Comment:
//;   Quaternion multiplication is defined as:
//;   rw = aw * bw - ax * bx - ay * by - az * bz
//;   rx = ax * bw + bx * aw + ay * bz - az * by;
//;   ry = ay * bw + by * aw + az * bx - ax * bz;
//;   rz = az * bw + bz * aw + ax * by - ay * bx;
//;
//;   Each x,y,z is equal to sqrt(-1) (quaternions are a kind of complex number),
//;   and multiplication is defined such that xy = -z, xz = -y, and yz = -x.  The 'w'
//;   value is a real number.  The above relations come from algebraic multiplication
//;   of the quaternions.
//;
//;******************************************************************************
void _mult_quat (D3DRMQUATERNION *r, const D3DRMQUATERNION *a, const D3DRMQUATERNION *b)
{
    __m64 r0, r1, r2, r3, r4, r5, r6, r7;

    _m_femms();
    __m64 *pa = (__m64 *)a;
    __m64 *pb = (__m64 *)b;
    __m64 *pr = (__m64 *)r;

    r0 = pa[0];                     //; mm0 = ax|aw
    r1 = pb[0];                     //; mm1 = bx|bw
    r0 = _m_pfmul (r0, r1);         //; mm0 = ax*bx|aw*bw

    r2 = _m_from_floats (&a->v.y);  //; mm2 = az|ay
    r3 = _m_from_floats (&b->v.y);  //; mm3 = bz|by

    r5 = r1;                        //; mm5 = bx|bw
    r4 = r0;                        //; mm4 = ax*bx|aw*bw
    r0 = _m_punpckhdq (r0, r0);     //; mm0 = ax*bx|ax*bx

    r2 = _m_pfmul (r2, r3);         //; mm2 = az*bz|ay*by
    r4 = _m_pfsub (r4, r0);        //; mm4 = ???|aw*bw-ax*bx

    r5 = _m_punpckhdq (r5, r5);     //; mm5 = bx|bx
    r6 = r3;                        //; mm6 = bz|by

    r2 = _m_pfacc (r2, r2);         //; mm2 = ???|az*bz+ay*by
    r5 = _m_punpckldq (r5, r1);     //; mm5 = bw|bx

    r5 = _m_pfmul (r5, pa[0]);      //; mm5 = ax*bw|aw*bx
    r6 = _m_punpckhdq (r6, r6);     //; mm6 = bz|bz

    r4 = _m_pfsub (r4, r2);        //; mm4 = ???|rw
    r7 = pa[1];                     //; mm7 = az|ay

    r5 = _m_pfacc (r5, r5);         //; mm5 = ???|ax*bw+aw*bx
    r6 = _m_punpckldq (r6, r3);     //; mm6 = by|bz

    r6 = _m_pfmul (r6, r7);         //; mm6 = az*by|ay*bz
    r->s = _m_to_float (r4);        //; mm4 = ???|rw

    r2 = pa[0];                     //; mm2 = ???|aw
    r0 = _m_punpckldq (r1, r1);     //; mm0 = bw|bw
    r2 = _m_punpckldq (r2, r2);     //; mm2 = aw|aw

    r0 = _m_pfmul (r0, r7);         //; mm0 = az*bw|ay*bw

    r7 = _m_punpckhdq (r7, _m_from_floats (&a->s));   //; mm7 = ax|az
    r1 = _m_punpckhdq (r1, r1);     //; mm1 = bx|bx


    r->v.x = _m_to_float (_m_pfsub (_m_pfadd (r5, r6),
                                    _m_punpckhdq (r6, r6)));

    // Packed together version of the last part of this routine, just
    // to stress out the code generator a little. :-)
    pr[1] = _m_pfsub (_m_pfadd (_m_pfadd (_m_pfmul (r2, r3),
                                          r0),
                                _m_pfmul (r7,
                                          _m_punpckldq (r1, r3))),
                      _m_pfmul (_m_punpckhdq (r3, pb[1]),
                                _m_punpckldq (_m_from_float (a->v.x),
                                              _m_from_floats (&a->v.x))));
    _m_femms();
}



//;******************************************************************************
//; Routine:  void _norm_quat (D3DRMQUATERNION *r, const D3DRMQUATERNION *a)
//; Input:    r - resultant quaternion
//;           a - input quaternion
//; Output:   r = normalize(a) such that (x^2 + y^2 + z^2 + w^2 = 1)
//; Uses:     eax,mm0-mm4
//; Comment:
//;   Normal quaternion form is (x^2 + y^2 + z^2 + w^2 = 1).
//;******************************************************************************
void _norm_quat (D3DRMQUATERNION *r, const D3DRMQUATERNION *a)
{
    _m_femms();

    __m64 r0, r1, r2, r3;
    __m64 *const pa = (__m64 *)a;
    __m64 *const pr = (__m64 *)r;

    r0 = pa[0];                     // r0 = (x|w)
    r1 = pa[1];                     // r1 = (z|y)
    r2 = r0;                        // save (x|w)
    r0 = _m_pfmul (r0, r0);         // r0 = (x^2|w^2)
    r3 = r1;                        // save (z|y)
    r1 = _m_pfmul (r1, r1);         // r1 = (z^2|y^2)
    r1 = _m_pfadd (r1, r0);         // r1 = (x^2 + z^2 | w^2 + y^2)
    r1 = _m_pfacc (r1, r1);         // r1 = x^2 + y^2 + z^2 + w^2


    r0 = _m_pfrsqrt (r1);          // r0 = 1/sqrt(r1)
    r0 = _m_pfrcpit2 (_m_pfrsqit1 (_m_pfmul (r0, r0),
                                   r1),
                      r0);         // r0 = 1/sqrt(r0) 24 bits



    r0 = _m_punpckldq (r0, r0);     // r0 = (1/sqrt(mm0)|1/sqrt(mm0))
    pr[0] = _m_pfmul (r2, r0);      // (x|w) = (x/sqrt(...)|w/sqrt(...))
    pr[1] = _m_pfmul (r3, r0);      // (z|y) = (z/sqrt(...)|y/sqrt(...))

    _m_femms();
}



//;******************************************************************************
//; Routine:  void _quat2D3DMat (D3DMATRIX *A1, const D3DRMQUATERNION *A0)
//; Input:    A0 - quaternion address
//;           A1 - matrix (4x4) address
//; Output:   A1 = rotation matrix from A0
//; Uses:     eax, mm0-mm7
//;******************************************************************************
void _quat2D3DMat (D3DMATRIX *A1, const D3DRMQUATERNION *A0)
{
    __m64 r0, r1, r2, r3, r4, r5, r6, r7;

    _m_femms();

    const __m64 *pA0 = (const __m64 *)A0;

    r0 = pA0[0];                    //; mm0 qx|qw
    r1 = pA0[1];                    //; mm1 qz|qy

    r2 = _m_pfmul (r0, r0);         //; mm2 qx*qx|qw*qw

    r3 = _m_pfmul (r1, r1);         //; mm3 = qz*qz|qy*qy
    r3 = _m_pfacc (r3, r2);         //; mm3 = qx*qx+qw*qw|qz*qz+qy*qy

    r3 = _m_pfacc (r3, r3);         //; mm3 = qx*qx+qw*qw|qz*qz+qy*qy|qx*qx+qw*qw|qz*qz+qy*qy

    r4 = _m_pfrcp (r3);
    r3 = _m_pfrcpit2 (_m_pfrcpit1 (r3, r4), r4);

    r2 = _m_pfmul (r3, TWOS);       //; mm2 = s|s
    r7 = _m_punpckhdq (r0, r0);     //; mm7 = qx|qx

    r6 = r0;                        //; mm6 = qx|qw
    r5 = _m_pfmul (r0, r2);         //; mm5 = qx * s|???

    r2 = _m_pfmul (r2, r1);         //; mm2 = qz*s(zs)|qy*s(ys)
    r7 = _m_punpckldq (r7, r0);     //; mm7 = qw|qx

    r5 = _m_punpckhdq (r5, r5);     //; mm5 = xs|xs

    r3 = r1;                        //; mm3 = qz|qy
    r5 = _m_pfmul (r5, r0);         //; mm5 = qx*xs(xx)|qw * xs(wx)

    r6 = _m_pfmul (r6, r2);         //; mm6 = qx*zs(xz)|qw*ys(wy)

    r7 = _m_pfmul (r7, r2);         //; mm7 = qw*zs(wz)|qx*ys(xy)

    r3 = _m_pfmul (r3, r2);         //; mm3 = qz*zs(zz)|qy*ys(yy)
    r2 = _m_punpckhdq (r2, r2);     //; mm2 = zs|zs

    r2 = _m_pfmul (r2, r1);         //; mm2 = ???|q.y*zs(yz)
    r0 = r3;                        //; mm0 = zz|yy

    r0 = _m_punpckhdq (r0, r5);     //; mm0 = xx|zz

    r0 = _m_pfacc (r0, r3);         //; mm0 = zz+yy|xx+zz

    r4 = _m_punpckldq (r4, r3);     //; mm4 = yy|???

    r1 = _m_pfsub (ONES, r0);       //; mm1 = 1-(zz+yy)|1-(xx+zz)
    r4 = _m_pfadd (r4, r5);         //; mm4 = xx+yy|???

    r3 = r6;                        //; mm3 = xz | wy

    A1->_22 = _m_to_float (r1);
    r1 = _m_punpckhdq (r1, r1);     //; mm1 = ???|1-(zz+yy)

    r0 = _m_pfsub (ONES, r4);       //; mm0 = 1-(xx+yy)|???
    A1->_11 = _m_to_float (r1);     //; mm1 = ???|1-(zz+yy)

    r3 = _m_pfacc (r3, r3);         //; mm3 = ???|xz+wy

    r0 = _m_punpckhdq (r0, r0);     //; mm0 = ???|1-(xx+yy)
    r1 = _m_punpckldq (r5, r2);     //; mm1 = yz | wx

    A1->_33 = _m_to_float (r0);     //; mm0 = ???|1-(xx+yy)
    r1 = _m_pfacc (r1, r7);         //; mm1 = wz+xy|yz+wx

    r0 = r6;                        //; mm0 = xz | wy
    A1->_13 = _m_to_float (r3);     //; mm3 = ???|xz+wy

    r0 = _m_punpckhdq (r0, r0);     //; mm0 = xz | xz
    r2 = _m_pfsub (r2, r5);         //; mm2 = ???|yz-wx

    A1->_32 = _m_to_float (r1);     //; mm1 = wz+xy|yz+wx
    r0 = _m_pfsub (r0, r6);         //; mm0 = ???|xz-wy

    r4 = _m_punpckhdq (r7, r7);     //; mm4 = wz | wz

    r1 = _m_punpckhdq (r1, r1);     //; mm1 = wz+xy|wz+xy
    r7 = _m_pfsub (r7, r4);         //; mm7 = ???| xy-wz

    A1->_21 = _m_to_float (r1);     //; mm1 = wz+xy|wz+xy
    A1->_31 = _m_to_float (r0);     //; mm0 = ???|xz-wy  
    A1->_23 = _m_to_float (r2);     //; mm2 = ???|yz-wx  
    A1->_12 = _m_to_float (r7);     //; mm7 = ???| xy-wz 

    //; Clear out remaining entries (collapse into movq's?)
    A1->_14 = 0;
    A1->_24 = 0;
    A1->_34 = 0;
    A1->_44 = 0;
    A1->_41 = 0;
    A1->_42 = 0;
    A1->_43 = 0;
    A1->_44 = 1;

    _m_femms();
}



//;******************************************************************************
//; Routine:  void _euler2quat (D3DRMQUATERNION *A1, const float *A0)
//; Input:    A1 - resultant quaternion
//;           A0 - euler angles address (array of 3 floats)
//; Output:   A1 = quaternion performing the specified rotations
//; Uses:     eax, ecx, edx, mm0-mm7
//;******************************************************************************
void _euler2quat (D3DRMQUATERNION *A1, const float *A0)
{
    __m64 r0;
    __m64 csx, csz, cssc, ccss, sscc, sccs, cysy, cynsy;

    __m64 *pA0 = (__m64 *)A0;
    __m64 *pA1 = (__m64 *)A1;

    _m_femms();

    r0 = pA0[0];
    csx = _m_sincos (_m_pfmul (pA0[0], DEG2RAD2));
    csz = _m_sincos (_m_pfmul (pA0[1], DEG2RAD2));

    //; cc = csx[0] * csz[0];
    //; ss = csx[1] * csz[1];
    ccss = _m_pfmul (csx, csz);                     //; mm2 = cc:ss

    //; cs = csx[0] * csz[1];
    //; sc = csx[1] * csz[0];
    cssc = _m_pfmul (_m_pswapd (csz), csx);         //; mm3 = cs:sc

    r0 = _m_pfmul (_m_from_float (A0[1]), DEG2RAD2);
    cysy  = _m_sincos (r0);                         //; mm0 = cy:sy
    cynsy = _m_pxor (cysy, NEG2);                   //; mm3 = cy/-sy

    sscc = _m_pswapd (ccss);  //; ss:cc
    sccs = _m_pswapd (cssc);  //; sc:cs

    //; final computation
    pA1[0] = _m_pfacc (_m_pfmul (cysy, ccss),
                       _m_pfmul (cynsy, sccs));

    pA1[1] = _m_pfacc (_m_pfmul (cysy, sscc),
                       _m_pfmul (cynsy, sccs));

    _m_femms();
}



//;******************************************************************************
//; Routine:  void _axis_angle2quat (D3DRMQUATERNION *quat, const float *axis)
//; Input:    quat - output quaternion
//;           axis - axis address (1x4 of <x,y,z> and w angle in degrees)
//; Output:   quat = rotation around given axis
//; Uses:     eax, ecx, edx, mm0-mm7
//;******************************************************************************
void _axis_angle2quat (D3DRMQUATERNION *quat, const float *axis)
{
    __m64 r0, r1, r2, r3;

    _m_femms();
    r0 = _m_sincos (_m_pfmul (_m_from_float (axis[3]), HALFS));
    r1 = _m_punpckhdq (r0, r0);
    r0 = _m_punpckldq (r0, r0);
    r2 = _m_from_floats (axis + 8);
    quat->s = _m_to_float (r1);
    r2 = _m_pfmul (_m_punpckldq (r2, r2), r0);
    r3 = _m_pfmul (_m_from_floats (axis), r0);
    r2 = _m_punpckhdq (r2, r3);
    quat->v.x = _m_to_float (r3);
    *((__m64 *)&quat->v.y) = r2;

    _m_femms ();
}


//;******************************************************************************
//; Routine:  _quat2axis_angle (float *axis, const D3DRMQUATERNION *quat)
//; Input:    axis    - resultant float[4] <x,y,z,angle>
//;           quat    - input quaternion
//; Output:   axisAngle representation of 'quat'
//; Uses:     eax, ecx, edx, mm0-mm7
//;******************************************************************************
void _quat2axis_angle (float *axis, const D3DRMQUATERNION *quat)
{
    __m64 scale, tw;

    _m_femms();

    tw = _m_acos (_m_from_float (quat->s));
    axis[3] = _m_to_float (_m_pfmul (tw, RAD2DEG2));

    scale = _m_sin (tw);
    scale = _m_pfrcp (tw);
    scale = _m_pfrcpit2 (_m_pfrcpit1 (tw, scale), scale);

    tw = _m_pfmul (_m_from_floats (&quat->v.y), scale);
    axis[0] = _m_to_float (_m_pfmul (_m_from_float (quat->v.x), scale));
    axis[1] = _m_to_float (tw);
    axis[2] = _m_to_float (_m_punpckhdq (tw, tw));

    _m_femms();
}



//;******************************************************************************
//; Routine:  void _slerp_quat (D3DRMQUATERNION *result,
//;                             const D3DRMQUATERNION *quat1, const D3DRMQUATERNION *quat2,
//;                             float slerp)
//; Input:    result - resultant quaternion
//;           q1     - input quaternion (slerp == 0.0)
//;           q2     - input quaternion (slerp == 1.0)
//;           slerp  - float interp factor
//; Output:   result = quaternion spherically interpolate between q1 and q2
//; Uses:     eax, ecx, edx, mm0-mm7
//;******************************************************************************
void _slerp_quat (D3DRMQUATERNION *result,
                  const D3DRMQUATERNION *q1, const D3DRMQUATERNION *q2,
                  float slerp)
{
    __m64 omega, scale0, scale1, cosom, isinom, qwx, qyz, r0, r1, r2;

    _m_femms();

    //; DOT the quats to get the acosine of the angle between them
    cosom = _m_pfadd (_m_pfmul (_m_from_floats (&q1->s),
                                _m_from_floats (&q2->s)),
                      _m_pfmul (_m_from_floats (&q1->v.y),
                                _m_from_floats (&q2->v.y)));
    cosom = _m_pfacc (cosom, cosom);

    //; Two special cases:
    //; if (cosom <= DELTA - 1.0f) do perpendiclar
    if (_m_to_int (_m_pfcmpgt (cosom, _m_from_float (DELTA_1))) == 0)
    {
        // Perpendicular case
        //; set scale0, scale1 for slerp to a perpendicular quat
        //; scale0 = _sin ((1.0f - slerp) * HALF_PI);
        scale0 = _m_sin (_m_pfmul (_m_pfsub (_m_from_float (ONES.m64_f32[0]),
                                             _m_from_float (slerp)),
                                   _m_from_float (HALF_PI)));


        //; scale1 = _sin (slerp * HALF_PI);
        r0 = _m_sin (_m_pfmul (_m_from_float (slerp),
                               _m_from_float (HALF_PI)));
        scale1 = _m_punpckldq (r0, r0);                  // scale1 | scale1

        //; q2w = quat2[3] *  scale1
        //; q2x = quat2[2] * -scale1
        //; q2y = quat2[1] *  scale1
        //; q2z = quat2[0] * -scale1
        r1 = _m_from_floats (&q2->v.y);
        r2 = _m_from_floats (&q1->s);

        // swap low and high dwords
        r1 = _m_punpckhdq (_m_punpckldq (r1, r1), r1);
        r2 = _m_punpckhdq (_m_punpckldq (r2, r2), r2);

        qwx = _m_pfmul (r1, r0);
        qyz = _m_pfmul (r2, r0);
    }
    else
    {
        //; if (cosom >= 1.0f - DELTA) do linear
        if (_m_to_int (_m_pfcmpge (cosom, _m_from_float (DELTA_2))) != 0)
        {
            //; Set scale0, scale1 for linear interpolation
            //; scale0 = 1.0f - slerp;
            //; scale1 = slerp;
            r1 = _m_from_float (slerp);
            scale0 = _m_pfsub (_m_from_float (ONES.m64_f32[0]), r1);    // -slerp | 1 - slerp
            scale1 = _m_punpckldq (r1, r1);                             // slerp | slerp
        }
        else
        {
            //; otherwise, SLERP away!
            //; omega = (float)acos (cosom);
            omega = _m_acos (cosom);

            //; isinom = 1.0f / (float)sin (omega);
            r0 = _m_sin (omega);
            r1 = _m_pfrcp (r0);
            isinom = _m_pfrcpit2 (_m_pfrcpit1 (r0, r1), r1);    // isinom = 1/sin(o) | 1/sin(o)

            //; scale0 = (float)sin ((1.0f - slerp) * omega) * isinom;
            //; scale1 = (float)sin (slerp * omega) * isinom;
            //; (this code is terribly sisd, but I'm can't puzzle out a better way)
            scale0 = _m_sin (_m_pfmul (_m_pfsub (ONES,
                                                 _m_from_float (slerp)),
                                       omega));

            r0 = _m_sin (_m_pfmul (_m_from_float (slerp), omega));
            scale1 = _m_pfmul (_m_punpckldq (r0, r0), isinom);

        }

        //; q2w = quat2[0] * scale1;
        //; q2x = quat2[1] * scale1;
        //; q2y = quat2[2] * scale1;
        //; q2z = quat2[3] * scale1;

        qwx = _m_pfmul (scale1, _m_from_floats (&q2->s));
        qyz = _m_pfmul (scale1, _m_from_floats (&q2->v.y));
    }

    // Slerp away
    //; result[0] = scale0 * quat1[0] + q2x;
    //; result[1] = scale0 * quat1[1] + q2y;
    //; result[2] = scale0 * quat1[2] + q2z;
    //; result[3] = scale0 * quat1[3] + q2w;
    r0 = _m_punpckldq (scale0, scale0);
    __m64 *pres = (__m64 *)result;
    pres[0] = _m_pfadd (_m_pfmul (r0,
                                  _m_from_floats (&q1->s)),
                        qwx);
    pres[1] = _m_pfadd (_m_pfmul (r0,
                                  _m_from_floats (&q1->v.y)),
                        qyz);
}


//;******************************************************************************
//; Routine:  void _trans_quat (D3DVECTOR *d, const D3DVECTOR *a, const D3DRMQUATERNION *b)
//; Input:    d - translated vector
//;           a - input vector
//;           b - rotation quaternion
//; Output:   d = 'a' rotated with 'b'
//; Comments: Note that this is equivalent to using quat2mat to make a rotation
//;           matrix, and then multiplying the vector by the matrix.  This form
//;           is more compact, and equally efficient when only transforming a
//;           single vector.  For other cases, it is advisable to construct
//;           a rotation matrix.
//; Uses:     eax,edx,mm0-mm7
//;******************************************************************************
void _trans_quat (D3DVECTOR *r, const D3DVECTOR *a, const D3DRMQUATERNION *b)
{
    __m64 r0, r1, r2, r3, r4, r5, r6, r7;

    _m_femms();

    r0 = _m_from_floats (&b->s);    //; mm0 = qx|qw
    r1 = _m_from_floats (&b->v.y);  //; mm1 = qz|qy
    r5 = _m_from_float (a->x);      //; mm5 = 0 |vx
    r2 = _m_pfmul (r0, r0);         //; mm2 = qx*qx|qw*qw
    r0 = _m_punpckhdq (r0, r0);     //; mm0 = qx|qx
    r3 = _m_pfmul (r1, r1);         //; mm3 = qz*qz|qy*qy

    r0 = _m_pfmul (r0, r5);         //; mm0 = 0|qx*vx

    r3 = _m_pfacc (r3, r3);         //; mm3 = qz*qz+qy*qy|qz*qz+qy*qy
    r4 = _m_punpckhdq (r2, r2);     //; mm4 = ???|qx*qx

    r6 = _m_from_floats (&a->y);    //; mm6 = vz|vy
    r2 = _m_pfsub (r2, r4);        //; mm2 = ???|qw*qw - qx*qx

    r2 = _m_pfsub (r2, r3);        //; mm2 = ???|dot_prod1
    r7 = r6;                        //; mm7 = vz|vy

    r5 = _m_pfmul (r5, r2);         //; mm5 = ???|vx*dot_prod1(dst.x)
    r2 = _m_punpckldq (r2, r2);     //; mm2 = dot_prod1|dot_prod1

    r2 = _m_pfmul (r2, r6);         //; mm2 = vz*dot_prod1(dst.z)|vy*dot_prod1(dst.y)

    r6 = _m_pfmul (r6, r1);         //; mm6 = qz * vz|qy * vy
    r3 = TWOS;

    r6 = _m_pfacc (r6, r6);         //; mm6 = ???|qz * vz+qy * vy

    r4 = _m_punpckhdq (r1, r1);     //; mm1 = qz|qz

    r6 = _m_pfadd (r6, r0);         //; mm6 = ???|qz * vz+qy * vy+qx*vx
    r0 = _m_from_float (b->v.x);    //; mm0 = ???|qx

    r4 = _m_punpckldq (r4, r1);     //; mm4 = qy|qz

    r6 = _m_pfmul (r6, r3);         //; mm6 = ???|dot_prod2

    r4 = _m_pfmul (r4, r7);         //; mm4 = qy*vz|qz*vy
    r7 = TWOS;

    r0 = _m_pfmul (r0, r6);         //; mm0 = ???|qx * dot_prod2
    r6 = _m_punpckldq (r6, r6);     //; mm6 = dot_prod2|dot_prod2

    r6 = _m_pfmul (r6, r1);         //; mm6 = qz*dot_prod2|qy*dot_prod2
    r3 = r4;                        //; mm3 = qy*vz|qz*vy

    r5 = _m_pfadd (r5, r0);         //; mm5 = ???dst.x
    r3 = _m_punpckhdq (r3, r3);     //; mm3 = qy*vz|qy*vz

    r2 = _m_pfadd (r2, r6);         //; mm2 = dst.z|dst.y
    r0 = _m_from_float (b->s);

    r6 = _m_from_float (b->v.x);    //; mm6 = ???|qx
    r3 = _m_pfsub (r3, r4);         //; mm3 = ???|qy * vz-qz * vy

    r7 = _m_pfmul (r7, r0);         //; mm7 = ???|qw * 2.0
    r6 = _m_punpckldq (r6, r1);     //; mm6 = qy|qx

    r1 = _m_punpckhdq (r1, r1);     //; mm1 = qz|qz

    r3 = _m_pfmul (r3, r7);         //; mm3 = ???|(qy * vz - qz * vy)* qw * 2.0
    r0 = _m_from_floats (&a->x);    //; mm0 = vy|vx

    r1 = _m_punpckldq (r1, r6);     //; mm1 = qx|qz

    r5 = _m_pfadd (r5, r3);         //; mm5 = ???|dst.x
    r3 = _m_from_float (a->z);      //; mm3 = ???|vz

    r1 = _m_pfmul (r1, r0);         //; mm1 = qx * vy|qz * vx
    r3 = _m_punpckldq (r3, r0);     //; mm3 = vx|vz

    r7 = _m_punpckldq (r7, r7);     //; mm7 = qw * 2.0|qw * 2.0
    r6 = _m_pfmul (r6, r3);         //; mm6 = qy*vx|qx*vz

    r1 = _m_pfsub (r1, r6);        //; mm1 = qx * vy-qy*vx|qz * vx-qx*vz
    r->x = _m_to_float (r5);        //; mm5 = ???|dst.x
    *((__m64 *)&r->y) = _m_pfadd (r2, _m_pfmul (r1, r7));

    _m_femms();
}

// eof
