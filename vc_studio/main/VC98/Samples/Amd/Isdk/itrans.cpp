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
//; TRANS.ASM
//;
//; AMD3D 3D library code: vector translation
//;
//; The 4x4 versions of these routines have been re-declared to use Direct3D
//; types for greater compatability with most game programs.  The 3x3 versions
//; have been retained, but with the float * interface.  D3D* functions use
//; Direct3D-style row major matrices, and gl* functions use OpenGL-style
//; column major matrices.
//;
//; There are no 4x3 matrix routines currently implemented.
//;
//;******************************************************************************

#include "atrans.h"
#include "amath.h"


// It's OK not to use FEMMS in these routines
#pragma warning (disable:4799)
#pragma warning (disable:4730)

#define _m_from_floats(f)   (*(__m64 *)(f))
#define _m_to_float(f)      ((f).m64_f32[0])
#define _m_from_float(f)    _m_from_int (*(int *)&(f))


//;******************************************************************************
//; Routine:  void _D3DTrans_4x4 (D3DTLVERTEX &r, const D3DTLVERTEX &a,
//;                               const D3DMATRIX &m)
//; Input:    r - result vector (1x4)
//;           a - input  vector (1x4)
//;           m - matrix (4x4)
//; Output:   r = a * m (matrix multiplication rules)
//; Uses:     eax, ecx, edx, mm0 - mm7
//;******************************************************************************
void _D3DTrans_4x4 (D3DTLVERTEX &r, const D3DTLVERTEX &a, const D3DMATRIX &m)
{
    __m64 r0, r1, r2, r3, r4, r5, r6, r7;

    _m_femms();

    r0 = _m_from_floats (&a.sx);	                //;  y   |  x
    r1 = _m_from_floats (&a.sz);                    //; rhw  |  z
    r4 = _m_from_floats (&m._11);                   //; m_12 | m_11
    r2 = _m_punpckhdq (r0, r0);                     //;  y   |  ?
    r5 = _m_from_floats (&m._21);                   //; m_22 | m_21
    r0 = _m_punpckldq (r0, r0);                     //;  x   |  x
    r6 = _m_from_floats (&m._31);                   //; m_32 | m_31
    r4 = _m_pfmul (r4, r0);   	                    //; x * m_12 | x * m_11
    r7 = _m_from_floats (&m._41);                   //; m_42 | m_41
    r2 = _m_punpckhdq (r2, r2);                     //;  y   |  y
    r3 = _m_punpckhdq (r1, r1);                     //; rhw  |  ?
    r5 = _m_pfmul (r5, r2);   	                    //; y * m_22 | y * m_21
    r1 = _m_punpckldq (r1, r1);                     //;  z   |  z
    r0 = _m_pfmul (r0, _m_from_floats (&m._13));    //; x * m_14 | x * m_13
    r3 = _m_punpckhdq (r3, r3);                     //; rhw  | rhw
    r2 = _m_pfmul (r2, _m_from_floats (&m._23));    //; y * m_24 | y * m_23
    r6 = _m_pfmul (r6, r1);   	                    //; z * m_32 | z * m_31
    r5 = _m_pfadd (r5, r4);   	                    //; x * m_12 + y * m_22 | x * m_11 + y * m_21
    r1 = _m_pfmul (r1, _m_from_floats (&m._33));    //; z * m_34 | z * m_33
    r2 = _m_pfadd (r2, r0);   	                    //; x * m_14 + y * m_24 | x * m_13 +  y * m_23
    r7 = _m_pfmul (r7, r3);   	                    //; rhw * m_42 | rhw * m_41
    r6 = _m_pfadd (r6, r5);   	                    //; x * m_12 + y * m_22 +  z * m_32 | x * m_11 + y * m_21 + z * m_31
    r3 = _m_pfmul (r3, _m_from_floats (&m._43));    //; rhw * m_44  | rhw * m_43
    r2 = _m_pfadd (r2, r1);       //; x * m_14 + y * m_24 + z * m_34 | x * m_13 +  y * m_23 + z * m_33
    r7 = _m_pfadd (r7, r6);   	//; x * m_12 + y * m_22 + z * m_32 + rhw * m_42 | x * m_11 + y * m_21 + z * m_31 + rhw * m_41
    r3 = _m_pfadd (r3, r2);   	//; x * m_14 + y * m_24 + z * m_34 + rhw * m_44 | x * m_13 + y * m_23 + z * m_33 + rhw * m_43
    *(__m64 *)&r.sx = r7;           //; r_y   | r_x
    *(__m64 *)&r.sz = r3;           //; r_rhw | r_z

    _m_femms();
}


//;******************************************************************************
//; Routine:  void _glTrans_4x4 (float *r, const float *a, const float *m)
//; Input:    r - result vector (4x1), 4 floats
//;           a - input  vector (4x1), 4 floats
//;           m - matrix (4x4), 16 floats
//; Output:   r = a * m (matrix multiplication rules)
//; Uses:     eax, ecx, edx, mm0 - mm7
//;******************************************************************************
void _glTrans_4x4 (float *r, const float *a, const float *m)
{
    _m_femms();

    const __m64 yx   = _m_from_floats (a);   	    // y   |  x
    const __m64 rhwz = _m_from_floats (a + 2);      // rhw |  z

#if 0
    __m64 r0, r1, r2, r3, r4, r5, r6, r7;

    r0 = _m_pfmul (yx,   _m_from_floats (m));       // y * m_21 | x * m_11
    r4 = _m_pfmul (rhwz, _m_from_floats (m+2));     // rhw * m_41 | z * m_31
    r1 = _m_pfmul (yx,   _m_from_floats (m+4));     // y * m_22 | x * m_12
    r5 = _m_pfmul (rhwz, _m_from_floats (m+6));     // rhw * m_42 | z * m_32
    r2 = _m_pfmul (yx,   _m_from_floats (m+8));     // y * m_23 | x * m_13
    r6 = _m_pfmul (rhwz, _m_from_floats (m+10));    // rhw * m_43 | z * m_33
    r3 = _m_pfmul (yx,   _m_from_floats (m+12));    // y * m_24 | x * m_14
    r7 = _m_pfmul (rhwz, _m_from_floats (m+14));    // rhw * m_44 | z * m_34
    r0 = _m_pfacc (r0, r4);     // rhw * m_41 + z * m_31 | y * m_21 + x * m_11 
    r1 = _m_pfacc (r1, r5);     // rhw * m_42 + z * m_32 | y * m_22 + x * m_12
    r2 = _m_pfacc (r2, r6);     // rhw * m_43 + z * m_33 | y * m_23 + x * m_13
    r3 = _m_pfacc (r3, r7);     // rhw * m_44 + z * m_34 | y * m_24 | x * m_14 
    r0 = _m_pfacc (r0, r1);     // rhw * m_41 + z * m_31 + y * m_21 + x * m_11 | rhw * m_42 + z * m_32 + y * m_22 + x * m_12
    r2 = _m_pfacc (r2, r3);     // rhw * m_44 + z * m_34 + y * m_24 | x * m_14 | rhw * m_43 + z * m_33 + y * m_23 + x * m_13

    *(__m64 *)(r+0) = r0;       //; r_y   | r_x
    *(__m64 *)(r+2) = r2;       //; r_rhw | r_z

#else

    // Condensed version - let the compiler do all the register allocation
    // and scheduling
    ((__m64 *)r)[0] = _m_pfacc (_m_pfacc (_m_pfmul (yx,   _m_from_floats (m)),
                                          _m_pfmul (rhwz, _m_from_floats (m+2))),
                                _m_pfacc (_m_pfmul (yx,   _m_from_floats (m+4)),
                                          _m_pfmul (rhwz, _m_from_floats (m+6))));
    ((__m64 *)r)[1] = _m_pfacc (_m_pfacc (_m_pfmul (yx,   _m_from_floats (m+8)),
                                          _m_pfmul (rhwz, _m_from_floats (m+10))),
                                _m_pfacc (_m_pfmul (yx,   _m_from_floats (m+12)),
                                          _m_pfmul (rhwz, _m_from_floats (m+14))));
#endif

    _m_femms();
}



//;******************************************************************************
//; Routine:  void _D3DTrans_3x3 (float *r, const float *a, const float *m)
//; Input:    r - result vector (1x3)
//;           a - input  vector (1x3)
//;           m - matrix (3x3)
//; Output:   r = a * m (matrix multiplication rules)
//; Uses:     eax, ecx, edx, mm0 - mm4
//; Note:     D3D matrix format - RHW not affected - basic 3x3 
//;******************************************************************************
void _D3DTrans_3x3 (float *r, const float *a, const float *m)
{
    __m64 r0, r1, r2, r3, r4, r5, r6, r7;

    _m_femms();

    r0 = _m_from_floats (a);   	                    //  y   |  x
    r1 = _m_from_float  (a[2]);                     //  0   |  z
    r4 = _m_from_float  (m[2]);                     //  0   | m_13
    r3 = r0;                    	                //  y   |  x
    r2 = _m_from_float  (m[8]); 	                //  0   | m_33
    r0 = _m_punpckldq (r0, r0);                     //  x   |  x
    r4 = _m_punpckldq (r4, _m_from_float (m[5]));   // m_31 | m_23
    r0 = _m_pfmul (r0, _m_from_floats (m));         //  x * m_12 | x * m_11
    r3 = _m_punpckhdq (r3, r3);                     //  y   |  y
    r2 = _m_pfmul (r2, r1);                         //  0   |  z * m_33
    r1 = _m_punpckldq (r1, r1);                     //  z   |  z
    r4 = _m_pfmul (r4, _m_from_floats (a));         // y * m_31 | x * m_23
    r3 = _m_pfmul (r3, _m_from_floats (&m[3]));     // y * m_22 | y * m_21
    r1 = _m_pfmul (r1, _m_from_floats (&m[6]));     // z * m_32 | z * m_32
    r4 = _m_pfacc (r4, r4);                         //  ?   | y * m_31 + x * m_23
    r3 = _m_pfadd (r3, r0);                         //  x * m_12 + y * m_22 | x * m_11 + y * m_21

    r[2] = _m_to_float (_m_pfadd (r4, r2));         //  ?   | y * m_31 + x * m_23 + z * m_33
    *(__m64 *)r = _m_pfadd (r3, r1);                //  x * m_12 + y * m_22 + z * m_32 | x * m_11 + y * m_21 + z * m_32

    _m_femms();
}



//;******************************************************************************
//; Routine:  void _glTrans_3x3 (float *r, const float *a, const float *m)
//; Input:    r - result vector (3x1), 3 floats
//;           a - input  vector (3x1), 3 floats
//;           m - matrix (3x3), 9 floats
//; Output:   r = a * m (matrix multiplication rules)
//; Uses:     eax, ecx, edx, mm0 - mm5
//;******************************************************************************
void _glTrans_3x3 (float *r, const float *a, const float *m)
{
    __m64 r0, r1, r2, r3, r4, r5, r6, r7;

    _m_femms();

    r0 = _m_from_floats (a);                        //;  y   |  x
    r4 = _m_from_float  (m[2]);                     //;  0   | m_31
    r3 = _m_from_float  (a[2]);	                    //;  0   |  z
    r1 = r0;                		                //;  y   |  x
    r2 = r0;                		                //;  y   |  x
    r0 = _m_pfmul (r0, _m_from_floats (m));         //; y * m_21 | x * m_11
    r1 = _m_pfmul (r1, _m_from_floats (m+3));       //; y * m_22 | x * m_12
    r3 = _m_punpckldq (r3, r3);                     //;  z   |  z
    r2 = _m_pfmul (r2, _m_from_floats (m+6));       //; y * m_23 | x * m_13
    r4 = _m_punpckldq (r4, _m_from_float (m[5]));   //; m_32 | m_31
    r0 = _m_pfacc (r0, r1);                         //; y * m_22 + x * m_12 | y * m_21 + x * m_11
    r5 = _m_from_float (m[8]);                      //;  0   | m_33
    r2 = _m_pfacc (r2, r2);                         //;      | y * m_23 + x * m_13
    r4 = _m_pfmul (r4, r3);                         //; z * m_32 | z * m_31
    r5 = _m_pfmul (r5, r3);                         //;		| z * m_33

    *(__m64 *)r = _m_pfadd (r4, r0);                //; y * m_22 + x * m_12 + z * m_32 | y * m_21 + x * m_11 + z * m_31
    r[3] = _m_to_float (_m_pfadd (r5, r2));         //; 		|  y * m_23 + x * m_13 + z * m_33

    _m_femms();
}



//;******************************************************************************
//; Routine:  void _D3DTrans_a3x3 (float *r, const float *a, const float *m, int n)
//; Input:    r - result vector, 3 floats
//;           a - input  vector, 3 floats
//;           m - matrix (3x3), 9 floats
//;           n - number of vectors in 'a' and 'r'
//; Output:   r[n] = a[n] * m (matrix multiplication rules)
//; Uses:     eax, ecx, edx, mm0 - mm7
//;******************************************************************************
void _D3DTrans_a3x3 (float *r, const float *a, const float *m, int n)
{
    __m64 r0, r1, r2, r3, r4, r5, r6, r7, m_33, xy, zz;

    if (n > 0)
    {
        _m_femms();
        _m_prefetch ((void *)a);
        r0 = _m_from_float (m[8]);                      //;  0   | m_33
        m_33 = r0;                                      //;  0   | m_33
        r7 = _m_from_floats (m);                        //; m_12 | m_11
        r5 = _m_from_float (m[2]);                      //;  0   | m_13
        r6 = _m_punpckhdq (r7, r7);                     //; m_12 | ?
        r7 = _m_punpckldq (r7, _m_from_floats (m+3));   //; m_21 | m_11
        r6 = _m_punpckhdq (r6, _m_from_floats (m+3));   //; m_22 | m_11
        r5 = _m_punpckldq (r5, _m_from_floats (m+5));   //; m_23 | m_13

        for (int i = 0; i < n; i++)
        {
            _m_prefetch ((void *)(a+3));
            xy = _m_from_floats (a);                    //;  y   |  x
            zz = _m_from_float (a[2]);                  //;  0   |  z
            r0 = _m_pfmul (xy, r7);                     //; y * m_21 | x * m_11 
            r1 = _m_pfmul (xy, r6);                     //; y * m_22 | x * m_11
            zz = _m_punpckldq (zz, zz);                 //;  z   |  z
            r2 = _m_pfmul (xy, r5);                     //; y * m_23 | x * m_13
            r3 = _m_pfmul (zz, _m_from_floats (m+6));   //; z * m_32 | z * m_31
            r4 = _m_pfmul (zz, m_33);                   //;  0   | z * m_33
            r0 = _m_pfacc (r0, r1);                     //; y * m_22 + x * m_11 |  y * m_21 + x * m_11 
            r2 = _m_pfacc (r2, r2);                     //;      |  y * m_23 + x * m_13
            r3 = _m_pfadd (r3, r0);                     //; y * m_22 + x * m_11 +  z * m_32 |  y * m_21 + x * m_11 +  z * m_31
            r4 = _m_pfadd (r4, r2);                     //;      |  y * m_23 + x * m_13 + z * m_33
            a += 3;                                     //; next vert
            *(__m64 *)r = r3;                           //; r_y  | r_x
            r[2] = _m_to_float (r4);                    //;      | r_z
            r += 3;                                 	//; next result
        }
        _m_femms();
    }
}



//;******************************************************************************
//; Routine:  void _glTrans_a3x3 (float *r, const float *a, const float *m, int n)
//; Input:    r - array of 'n' vectors (3x1), n*3 floats
//;           a - array of 'n' vectors (3x1), n*3 floats
//;           m - matrix (3x3), 9 floats
//;           n - number of vectors in 'a' and 'r'
//; Output:   r[n] = a[n] * m (matrix multiplication rules)
//; Uses:     eax, ecx, edx, mm0 - mm5
//;******************************************************************************
void _glTrans_a3x3 (float *r, const float *a, const float *m, int n)
{
    __m64 xy, zz, tmp;

    if (n > 0)
    {
        _m_femms();

        for (int i = 0; i < n; ++i)
        {
			const float* a3 = a + 3;
            _m_prefetch ((void *)(a3));

            // Highly condensed version to test register scheduling.
            xy = _m_from_floats (a);
            zz = _m_from_float (a[2]);
            zz = _m_punpckldq (zz, zz);
            tmp = _m_pfmul (xy, _m_from_floats (m+6));
            xy = _m_pfadd (_m_pfmul (_m_punpckldq (_m_from_float  (m[2]),
                                                   _m_from_floats (m+5)),
                                     zz),
                           _m_pfacc (_m_pfmul (xy, _m_from_floats (m+0)),
                                     _m_pfmul (xy, _m_from_floats (m+3))));

            zz = _m_pfadd (_m_pfmul (_m_from_float (m[8]),
                                     zz),
                           _m_pfacc (tmp,
                                     tmp));
            a += 3;
            *(__m64 *)r = xy;
            r[2]        = _m_to_float (zz);
            r += 3;
        }

        _m_femms();
    }
}



//;******************************************************************************
//; Routine:  void _D3DTrans_a4x4 (D3DTLVERTEX *r, const D3DTLVERTEX *a,
//;                                const D3DMATRIX &m, int n)
//; Input:    r - array of 'n' vectors (1x4)
//;           a - array of 'n' vectors (1x4)
//;           m - matrix (4x4) reference
//;           n - number of vectors in 'a' and 'r'
//; Output:   r[n] = a[n] * m (matrix multiplication rules)
//; Uses:     eax, ecx, edx, mm0 - mm7
//;******************************************************************************
void _D3DTrans_a4x4 (D3DTLVERTEX *r, const D3DTLVERTEX *a, const D3DMATRIX &m, int n)
{
    __m64 r0, r1, r2, r3, r4, r5, r6, r7;

    if (n > 0)
    {
        _m_femms();

        for (int i = 0; i < n; ++i)
        {
			D3DTLVERTEX *rplus1 = r + 1; 
			const D3DTLVERTEX *aplus1 = a + 1;
            _m_prefetch ((void *)(aplus1));
            _m_prefetchw ((void *)(rplus1));

            r0 = _m_from_floats (&a->sx);
            r1 = _m_from_floats (&a->sz);
            r4 = _m_from_floats (&m._11);
            r2 = _m_punpckhdq (r0, r0);
            r5 = _m_from_floats (&m._21);
            r0 = _m_punpckldq (r0, r0);
            r6 = _m_from_floats (&m._31);
            r4 = _m_pfmul (r4, r0);
            r7 = _m_from_floats (&m._41);
            r2 = _m_punpckhdq (r2, r2);
            r3 = _m_punpckhdq (r1, r1);
            r5 = _m_pfmul (r5, r2);
            r1 = _m_punpckldq (r1, r1);
            r0 = _m_pfmul (r0, _m_from_floats (&m._13));
            r3 = _m_punpckhdq (r3, r3);
            r2 = _m_pfmul (r2, _m_from_floats (&m._23));
            r6 = _m_pfmul (r6, r1);
            r5 = _m_pfadd (r5, r4);
            r1 = _m_pfmul (r1, _m_from_floats (&m._33));
            r2 = _m_pfadd (r2, r0);
            r7 = _m_pfmul (r7, r3);
            r6 = _m_pfadd (r6, r5);
            r3 = _m_pfmul (r3, _m_from_floats (&m._43));
            r2 = _m_pfadd (r2, r1);
            r7 = _m_pfadd (r7, r6);
            r3 = _m_pfadd (r3, r2);

            a += 1;
            ((__m64 *)r)[0] = r7;
            ((__m64 *)r)[1] = r3;
            r += 1;
        }    
        _m_femms();
    }
}



//;******************************************************************************
//; Routine:  void _glTrans_a4x4 (float *r, const float *a, const float *m, int n)
//; Input:    r - array of 'n' vectors (4x1), n*4 floats
//;           a - array of 'n' vectors (4x1), n*4 floats
//;           m - matrix (4x4) reference, 16 floats
//;           n - number of vectors in 'a' and 'r'
//; Output:   r[n] = a[n] * m (matrix multiplication rules)
//; Uses:     eax, ecx, edx, mm0 - mm7
//;******************************************************************************
void _glTrans_a4x4 (float *r, const float *a, const float *m, int n)
{
    __m64 r0, r1, r2, r3, r4, r5, r6, r7, yx, wz;

    if (n > 0)
    {
        _m_femms();

        for (int i = 0; i < n; ++i)
        {
			float       *next_r = r + 4;
			const float *next_a = a + 4;
            _m_prefetch ((void *)next_a);
            _m_prefetchw ((void *)next_r);

            const __m64 yx = _m_from_floats (a);        // y|x
            const __m64 wz = _m_from_floats (a+2);      // w|z

#if 0
            __m64 r0, r1, r2, r3, r4, r5, r6, r7;
            r0 = _m_pfmul (yx, _m_from_floats (m+0));
            r1 = _m_pfmul (wz, _m_from_floats (m+2));
            r2 = _m_pfmul (yx, _m_from_floats (m+4));
            r3 = _m_pfmul (wz, _m_from_floats (m+6));
            r4 = _m_pfmul (yx, _m_from_floats (m+8));
            r5 = _m_pfmul (wz, _m_from_floats (m+10));
            r6 = _m_pfmul (yx, _m_from_floats (m+12));
            r7 = _m_pfmul (wz, _m_from_floats (m+14));
            r0 = _m_pfacc (r0, r1);
            r1 = _m_pfacc (r2, r3);
            r2 = _m_pfacc (r4, r5);
            r3 = _m_pfacc (r6, r7);
            r0 = _m_pfacc (r0, r1);
            r2 = _m_pfacc (r2, r3);

            a = next_a;
            ((__m64 *)r)[0] = r0;
            ((__m64 *)r)[1] = r2;
            r = next_r;
#else
            // Condensed version for testing register allocation and instruction scheduling
            ((__m64 *)r)[0] = _m_pfacc (_m_pfacc (_m_pfmul (yx, _m_from_floats (m+0)),
                                                  _m_pfmul (wz, _m_from_floats (m+2))),
                                        _m_pfacc (_m_pfmul (yx, _m_from_floats (m+4)),
                                                  _m_pfmul (wz, _m_from_floats (m+6))));
            ((__m64 *)r)[1] = _m_pfacc (_m_pfacc (_m_pfmul (yx, _m_from_floats (m+8)),
                                                  _m_pfmul (wz, _m_from_floats (m+10))),
                                        _m_pfacc (_m_pfmul (yx, _m_from_floats (m+12)),
                                                  _m_pfmul (wz, _m_from_floats (m+14))));
            a = next_a;
            r = next_r;
#endif
        }
        _m_femms();
    }
}

// eof
