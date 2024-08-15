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
//; MATH.ASM
//;
//; AMD3D 3D library code: Math primitives
//;
//;******************************************************************************

#include <float.h>
#include "amath.h"

// It's OK not to use FEMMS in these routines
#pragma warning (disable:4799)  // No (F)EMMS
#pragma warning (disable:4730)  // Float/MMX mixing (not real)
#pragma warning (disable:4700)  // Uninitialized var used (result discarded)


#define ML2H    0xbf318000
#define MASKSMH 0x807fffff          // mask for sign and significand
#define MASKSH  0x80000000          // mask for sign bit


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


//;******************************************************************************
//; Vector (3DNow!) data 
//;******************************************************************************
static __m64  PMOne;
static __m64  HalfVal;
static __m64  HalfMin;
static __m64  ones;
static __m64  twos;
static const __m64i izeros      =   { 0,            0               };
static const __m64i pinfs       =   { 0x7f800000,   0x7f800000      };  // dword PINH, PINH
static const __m64i smh_masks   =   { 0x807fffff,   0x807fffff      };  // dword MASKSMH, MASKSMH
static const __m64i sign_mask   =   { 0x7fffffff,   0x7fffffff      };
static const __m64i sh_masks    =   { 0x80000000,   0x80000000      };  // dword MASKSH, MASKSH
static const __m64i two_126s    =   { 126,          126             };
static const __m64i ffs         =   { 0xffffffff,   0xffffffff      };
static const __m64i iones       =   { 1,            1               };

// ; SINCOS specific values
static __m64  mo2s;
static __m64  mo12_6;
static __m64  mo30_20;
static __m64  mo56_42;
static const __m64i pio4s       =   { 0x3f490fdb,   0x3f490fdb      };  // approximately 0.785398 | 0.785398
static const __m64i pio4ht      =   { 0xbf490000,   0xb97daa22      };  // approximately -0.000241913 | -0.785156


void init_imath()
{

	PMOne.m64_f32[0] = 1.0;
	PMOne.m64_f32[1] = -1.0;
	HalfVal.m64_f32[0] = 0.5;
	HalfVal.m64_f32[1] = 0.5;
	HalfMin.m64_f32[0] = 0.5;
	HalfMin.m64_f32[1] = -0.5f;
	ones.m64_f32[0] = 1.0;
	ones.m64_f32[1] = 1.0;
	twos.m64_f32[0] = 2.0;
	twos.m64_f32[1] = 2.0;
	mo2s.m64_f32[0] = -1.0f/2.0f;
	mo2s.m64_f32[1] = -1.0f/2.0f;
	mo12_6.m64_f32[0] = -1.0f/12.0f;
	mo12_6.m64_f32[1] = -1.0f/6.0f;
	mo30_20.m64_f32[0] = -1.0f/30.0f;
	mo30_20.m64_f32[1] = -1.0f/20.0f;
	mo56_42.m64_f32[0] = -1.0f/56.0f;
	mo56_42.m64_f32[1] = -1.0f/42.0f;
}

//;******************************************************************************
//; Scalar (single float) data
//;******************************************************************************
static int fouropi    =       0x3fa2f983;             // 1.27324f
static int xmax       =       0x46c90fdb;             // 25735.9
static int sgn        =       0x80000000;
static int mabs       =       0x7FFFFFFF;
static int mant       =       0x007FFFFF;
static int expo       =       0x7F800000;
static int one        =       0x3F800000;
static int half       =       0x3F000000;
static int two        =       0x40000000;
static int oob        =       0x00000000;
static int nan        =       0x7fffffff;
static int pnan       =       0x7fc00000;
static int n0         =       0x40A008EF;
static int n1         =       0x3DAA7B3D;
static int d0         =       0x412008EF;
static int qq0        =       0x419D92C8;
static int qq1        =       0x41E6BD60;
static int qq2        =       0x41355DC0;
static int pp0        =       0xC0D21907;
static int pp1        =       0xC0B59883;
static int pp2        =       0xBF52C7EA;
static int bnd        =       0x3F133333;
static int asp0       =       0x3F6A4AA5;
static int asp1       =       0xBF004C2C;
static int asq0       =       0x40AFB829;
static int asq1       =       0xC0AF5123;
static int pio2       =       0x3FC90FDB;
static int npio2      =       0xBFC90FDB;
static int ooln2      =       0x3FB8AA3B;
static int upper      =       0x42B17218;
static int lower      =       0xC2AEAC50;
static int ln2hi      =       0x3F317200;
static int ln2lo      =       0x35BFBE8E;
static int rt2        =       0x3FB504F3;
static int edec       =       0x00800000;
static int bias       =       0x0000007F;
static int c2         =       0x3E18EFE2;
static int c1         =       0x3E4CAF6F;
static int c0         =       0x3EAAAABD;
static int tl2e       =       0x4038AA3B;
static int maxn       =       0xFF7FFFFF;
static int q1         =       0x43BC00B5;
static int p1         =       0x41E77545;
static int q0         =       0x45E451C5;
static int p0         =       0x451E424B;
static int mine       =       0xC2FC0000;
static int maxe       =       0x43000000;
static int max        =       0x7F7FFFFF;
static int rle10      =       0x3ede5bdb;


//;******************************************************************************
//; SINCOSMAC - sin/cos simultaneous computation
//; Input:    mm0 - angle in radians
//; Output:   mm0 - (sin|cos)
//; Uses:     mm0-mm7, eax, ebx, ecx, edx, esi
//; Comment:  This macro simultaneously computes sin and cos of the input
//;           parameter, and returns the result packed in mm0 as (sin|cos).
//;           Ultimately, this routine needs higher precision and a more
//;           efficient implementation (less inter-register bank traffic).
//;******************************************************************************
inline __m64 SINCOSMAC (__m64 r0)
{
    __m64 r1, r2, r4, r5, r6, r7;
    unsigned int ir0, isign_bit, ecx, edx;

    ir0 = _m_to_int (r0);
    r1  = r0;
    r0  = _m_pand (r0, _m_from_int (mabs));
    r2  = _m_from_int (fouropi);

    isign_bit = ir0 & 0x80000000;
    edx       = ir0 >> 31;

    // fabs(r0) >= xmax
    if ((ir0 ^ isign_bit) >= xmax)
    {
        r0  = ones;
    }
    else
    {
        r1  = r0;
        r0  = _m_pfmul      (r0,r2);    // fabs(x) * 4 / PI
        r0  = _m_pf2id      (_m_punpckldq  (r0,r0));
        ecx = _m_to_int     (r0);
        edx ^= ecx >> 2;

        // r2 = ((ecx & 1) == 0)
        // r4 = ((ecx & 1) != 0)
        r2  = _m_pcmpeqd    (_m_pand (r0,iones), iones);
        r4  = _m_pxor       (r2, ffs);

        r0  = _m_pfmul      (_m_pi2fd (r0), pio4ht);
        r1  = _m_pfadd      (r1,r0);
        r0  = _m_punpckhdq  (r0,r0);
        r1  = _m_pfadd      (r1,r0);
        r1  = _m_punpckldq  (r1,r1);

//        if ((ecx & 1) != 0) r1 = _m_pfsubr (r1, pio4s);
//        else                r1 = r1;
        r1  = _m_por (_m_pand (r2, _m_pfsubr (r1, pio4s)),
                      _m_pand (r4, r1));

        edx <<= 31;                     // Becomes a sign bit
        r2  = _m_punpckldq  (ones,r1);
        r1  = _m_pfmul      (r1,r1);
        isign_bit ^= (ecx & 2) << 30;
        r0  = _m_from_int   (edx);

#if 1
        // Series solutions, 4 iterations unrolled
        r4 = _m_pfmul (mo56_42, r1);

        r5 = _m_pfmul (mo30_20, r1);
        r4 = _m_pfadd (_m_pfmul (r4, r5), r5);

        r5 = _m_pfmul (mo12_6, r1);
        r4 = _m_pfadd (_m_pfmul (r4, r5), r5);

        r5 = _m_punpckldq (_m_pfmul (mo2s, r1), ones);
        r4 = _m_pfadd (_m_pfmul (r4, r5), r5);

#else

        // Series solution, 4 iterations nested.
        // Let the optimizer schedule this however it likes
        r4  = _m_pfmul (
                _m_pfadd (
                    _m_pfmul (
                        _m_pfadd (
                            _m_pfmul (
                                _m_pfadd (
                                    _m_pfmul (
                                        mo56_42,
                                        r1),
                                    ones),
                                _m_pfmul (
                                    mo30_20,
                                    r1)),
                            ones),
                        _m_pfmul (
                            mo12_6,
                            r1)),
                    ones),
                _m_punpckldq (
                    _m_pfmul (
                        mo2s,
                        r1),
                    ones));
#endif

        r0  = _m_punpckldq  (r0,r0);
        r1  = _m_from_int   (isign_bit);
        r4  = _m_pfadd      (r4,_m_from_int (one));    // r4 += 1|0
        r4  = _m_pfmul      (r4,r2);

        if ((ecx & 1) != 0)
        {
//            r4 = _m_pswapd (r4);
            // r5 is a scratch register
            r4 = _m_punpckhdq (r4,_m_punpckldq (r5,r4));
        }
        r4  = _m_pxor       (r4,r1);    // set sign bit (???)
        r0  = _m_pxor       (r0,r4);    // set sign bit (quadrant)
    }

    return r0;
}



//;******************************************************************************
//; Routine:  _m_atan
//; Input:    r0.lo
//; Result:   r0.lo
//; Uses:     r0-r7
//; Comment:
//;   Compute atan(x) using MMX and 3DNow! instructions.Scalar version.
//;
//;   If the input has an exponent of 0xFF, the result of this routine
//;   is undefined. Inputs with an exponent of 0 are treated as true
//;   zeroes and return a function value of 0. Result can not overflow.
//;
//;   atan(x) = sign(x)*atan(abs(x). If x > 1, atan(x) = pi/2-atan(1/x)
//;   atan(x) for -1 <= x <= 1 is approximated by a rational minimax
//;   approximation.
//;
//;   Testing shows that this function has an error of less than 2.27
//;   single precision ulps
//;
//;   input      r0.low argument x
//;   output     r0.low result atan(x)
//;   destroys   r0, r1, r2, r3, r4, r5, r6, r7
//;******************************************************************************
__m64 _m_atan (__m64 r0)
{
    __m64 r1, r2, r3, r4, r5, r6, r7;
    r7 = _m_from_int    (sgn);          //; mask to extract sign bit
    r5 = _m_from_int    (mabs);         //; mask to clear sign bit
    r7 = _m_pand        (r7,r0);        //; sign(x)
    r6 = _m_from_int    (one);          //; 1.0
    r0 = _m_pand        (r0,r5);        //; z=abs(x)
    r6 = _m_pfcmpgt     (r6,r0);        //; z < 1 ? 0xffffffff : 0
    r2 = _m_pfrcp       (r0);           //; 1/z approx
    r1 = r0;                            //; save z
    r0 = _m_pfrcpit1    (r0,r2);        //; 1/z step
    r3 = _m_from_int    (qq2);          //; qq2
    r0 = _m_pfrcpit2    (r0,r2);        //; 1/z final
    r4 = _m_from_int    (pp2);          //; pp2
    r0 = _m_pfmin       (r0,r1);        //; z = z < 1 ? z : 1/z
    r1 = r0;                            //; save z
    r0 = _m_pfmul       (r0,r0);        //; z^2
    r5 = _m_from_int    (pp1);          //; pp1
    r3 = _m_pfadd       (r3,r0);        //; z^2 + qq2
    r4 = _m_pfmul       (r4,r0);        //; pp2 * z^2
    r2 = _m_from_int    (qq1);          //; qq1
    r3 = _m_pfmul       (r3,r0);        //; (z^2 + qq2) * z^2
    r4 = _m_pfadd       (r4,r5);        //; p2 * z^2 + pp1
    r5 = _m_from_int    (pp0);          //; pp0
    r3 = _m_pfadd       (r3,r2);        //; (z^2 + qq2) * z^2 + qq1
    r4 = _m_pfmul       (r4,r0);        //; (p2 * z^2 + pp1) * z^2
    r2 = _m_from_int    (qq0);          //; qq0
    r3 = _m_pfmul       (r3,r0);        //; ((z^2 + qq2) * z^2 + qq1) * z^2
    r4 = _m_pfadd       (r4,r5);        //; (p2 * z^2 + pp1) * z^2 + pp0
    r3 = _m_pfadd       (r3,r2);        //; qx=((z^2 + qq2) * z^2 + qq1) * z^2 + qq0
    r0 = _m_pfmul       (r0,r4);        //; ((p2 * z^2 + pp1) * z^2 + pp0) * z^2
    r0 = _m_pfmul       (r0,r1);        //; px*z^3=((p2 * z^2 + pp1) * z^2 + pp0) * z^3
    r5 = _m_pfrcp       (r3);           //; 1/qx approx
    r4 = _m_from_int    (pio2);         //; pi/2
    r3 = _m_pfrcpit1    (r3,r5);        //; 1/qx step
    r3 = _m_pfrcpit2    (r3,r5);        //; 1/qx final
    r0 = _m_pfmul       (r0,r3);        //; z^3*px/qx
    r0 = _m_pfadd       (r0,r1);        //; res=z + z^3 * px/qx 
    r4 = _m_pfsub       (r4,r0);        //; pi/2-res
    r6 = _m_pandn       (r6,r4);        //; z < 1 ? 0 : pi/2-res
    r0 = _m_pfmax       (r0,r6);        //; atan(abs(x)) = z < 1 ? res : pi/2-res
    r0 = _m_por         (r0,r7);        //; atan(x)=sign(x)*atan(abs(x))

    return r0;
}


//;******************************************************************************
//; Routine:  _m_atan2
//; Input:    r0.lo = x
//;           r0.hi = y
//; Result:   r0.lo = atan2 (y, x)
//; Uses:     r0-r7
//; Comment:
//;   Compute atan2(y,x) using MMX and 3DNow! instructions.Scalar version.
//;
//;   If the input has an exponent of 0xFF, the result of this routine
//;   is undefined. Inputs with an exponent of 0 are treated as true
//;   zeroes and return a function value of 0. Result can not overflow.
//;
//;   atan(x) = sign(x)*atan(abs(x). If x > 1, atan(x) = pi/2-atan(1/x)
//;   atan(x) for -1 <= x <= 1 is approximated by a rational minimax
//;   approximation.
//;
//;   Testing shows that this function has an error of less than 2.27
//;   single precision ulps
//;
//;   input      r0.low argument x
//;   output     r0.low result atan(x)
//;   destroys   r0, r1, r2, r3, r4, r5, r6, r7
//;******************************************************************************
__m64 _m_atan2 (__m64 r0)
{
    __m64 r1, r2, r3, r4, r5, r6, r7;

    r1 = _m_punpckhdq   (r0, r0);   //; y
    r7 = _m_from_int    (sgn);      //; mask to extract sign bit
    r5 = _m_from_int    (mabs);     //; mask to clear sign bit
    r6 = r7;                        //; mask to extract sign bit
    r7 = _m_pand        (r7,r0);    //; xs = sign(x)
    r6 = _m_pand        (r6,r1);    //; ys = sign(y)
    r7 = _m_psrldi      (r7,1);     //; xs >> 1
    r7 = _m_por         (r7,r6);    //; bit<31> = ys, bit<30> = xs
    r1 = _m_pand        (r1,r5);    //; ya = abs(y)
    r0 = _m_pand        (r0,r5);    //; xa = abs(x)
    r6 = r1;                        //; ya
    r2 = r0;                        //; save xa
    r6 = _m_pcmpgtd     (r6,r0);    //; df = (xa < ya) ? 0xfffffff : 0
    r0 = _m_pfmax       (r0,r1);    //; ma = max(xa,ya)
    r6 = _m_pslldi      (r6,31);    //; df = bit<31>
    r1 = _m_pfmin       (r1,r2);    //; mi = min(xa,ya)
    r2 = _m_pfrcp       (r0);       //; 1/ma approx
    r5 = _m_from_int    (pp1);      //; pp1
    r0 = _m_pfrcpit1    (r0,r2);    //; 1/ma step
    r4 = _m_from_int    (pp2);      //; pp2
    r0 = _m_pfrcpit2    (r0,r2);    //; 1/ma final
    r3 = _m_from_int    (qq2);      //; qq2
    r0 = _m_pfmul       (r0,r1);    //; r = mi/ma = mi*(1/ma)
    r1 = r0;                        //; save r
    r0 = _m_pfmul       (r0,r0);    //; r^2
    r3 = _m_pfadd       (r3,r0);    //; r^2 + qq2
    r4 = _m_pfmul       (r4,r0);    //; pp2 * r^2
    r2 = _m_from_int    (qq1);      //; qq1
    r3 = _m_pfmul       (r3,r0);    //; (r^2 + qq2) * z^2
    r4 = _m_pfadd       (r4,r5);    //; p2 * r^2 + pp1
    r5 = _m_from_int    (pp0);      //; pp0
    r3 = _m_pfadd       (r3,r2);    //; (z^2 + qq2) * z^2 + qq1
    r4 = _m_pfmul       (r4,r0);    //; (p2 * r^2 + pp1) * r^2
    r2 = _m_from_int    (qq0);      //; qq0
    r3 = _m_pfmul       (r3,r0);    //; ((r^2 + qq2) * r^2 + qq1) * r^2
    r4 = _m_pfadd       (r4,r5);    //; (p2 * r^2 + pp1) * r^2 + pp0
    r3 = _m_pfadd       (r3,r2);    //; qx=((r^2 + qq2) * r^2 + qq1) * r^2 + qq0
    r0 = _m_pfmul       (r0,r4);    //; ((p2 * r^2 + pp1) * r^2 + pp0) * r^2
    r4 = r7;                        //; ys, xs
    r0 = _m_pfmul       (r0,r1);    //; px*r^3=((p2 * r^2 + pp1) * r^2 + pp0) * r^3
    r5 = _m_pfrcp       (r3);       //; 1/qx approx 
    r7 = _m_pslldi      (r7,1);     //; xs = bit<31>
    r3 = _m_pfrcpit1    (r3,r5);    //; 1/qx step
    r3 = _m_pfrcpit2    (r3,r5);    //; 1/qx final
    r5 = r7;                        //; xs
    r7 = _m_pxor        (r7,r6);    //; xs^df ? 0x80000000 : 0
    r0 = _m_pfmul       (r0,r3);    //; r^3*px/qx
    r3 = _m_from_int    (npio2);    //; -pi/2
    r5 = _m_pxor        (r5,r3);    //; xs ? pi/2 : -pi/2
    r6 = _m_psradi      (r6,31);    //; df ? 0xffffffff : 0
    r0 = _m_pfadd       (r0,r1);    //; atan(r) = r + r^3 * px/qx
    r6 = _m_pandn       (r6,r5);    //; xs ? (df ? 0 : pi/2) : (df ? 0 : -pi/2)
    r1 = _m_from_int    (sgn);      //; mask for sign bit (to isolate ys)
    r6 = _m_pfsub       (r6,r3);    //; pr = pi/2+(xs?(df ? 0 : pi/2):(df ? 0 : -pi/2))
    r0 = _m_por         (r0,r7);    //; ar = xs^df ? -atan(r) : atan(r)
    
    //; here  xs df  pr     ar
    //;
    //;        0 0   0      atan(r)
    //;        0 1   pi/2   -atan(r)
    //;        1 0   pi     -atan(r)
    //;        1 1   pi/2   atan(r)
    
    r1 = _m_pand        (r1,r4);    //; ys
    r0 = _m_pfadd       (r0,r6);    //; res = ar + pr
    r0 = _m_por         (r0,r1);    //; atan2(y,x) = ys * res

    return r0;
}



//;******************************************************************************
//; Routine:  _m_acos
//; Input:    r0.lo
//; Result:   r0.lo
//; Uses:     r0-r7
//; Comment:
//;   Compute acos(x) using MMX and 3DNow! instructions.Scalar version.
//
//;   If the input has an exponent of 0xFF, the result of this routine
//;   is undefined. If the absolute value of the input is greater than
//;   1, a special result is returned (currently this is 0). Results
//;   for arguments in [-1, 1] are in [-pi/2, pi/2].
//;
//;   Let z=abs(x). Then acos(x) can be computed as follows:
//;
//;    -1 <= x <= -0.575: acos(x) = pi - 2 * asin(sqrt((1-z)/2))
//;    -0.575 <= x < 0  : acos(x) = pi/2 + asin(z)
//;    0 <= x < 0.575   : acos(x) = pi/2 - asin(z)
//;    0.575 <= x <= 1  : acos(x) = 2 * asin(sqrt((1-z)/2))
//;
//;   asin(z) for 0 <= z <= 0.575 is approximated by a rational minimax
//;   approximation.
//;
//;   Testing shows that this function has an error of less than 3.07
//;   single precision ulps.
//;
//;******************************************************************************
__m64 _m_acos (__m64 r0)
{
    __m64 r1, r2, r3, r4, r5, r6, r7;

    r6 = _m_from_int    (sgn);      //; mask for sign bit
    r7 = _m_from_int    (mabs);     //; mask for absolute value
    r4 = _m_from_int    (half);     //; 0.5
    r6 = _m_pand        (r6, r0);   //; extract sign bit
    r5 = _m_from_int    (one);      //; 1.0
    r0 = _m_pand        (r0, r7);   //; z = abs(x)
    r3 = r0;                        //; z
    r3 = _m_pcmpgtd     (r3, r5);   //; z > 1.0 ? 0xFFFFFFFF : 0
    r5 = r0;                        //; save z
    r0 = _m_pfmul       (r0, r4);   //; z*0.5
    r2 = _m_from_int    (bnd);      //; 0.575
    r0 = _m_pfsubr      (r0, r4);   //; 0.5 - z * 0.5
    r7 = _m_pfrsqrt     (r0);       //; 1/sqrt((1-z)/2) approx low
    r1 = r7;                        //; complete
    r7 = _m_pfmul       (r7, r7);   //;  reciprocal
    r2 = _m_pcmpgtd     (r2, r5);   //; z < 0.575 ? 0xfffffff : 0
    r7 = _m_pfrsqit1    (r7, r0);   //;   square root
    r4 = _m_from_int    (asq1);     //; asq1
    r7 = _m_pfrcpit2    (r7, r1);   //;    computation
    r7 = _m_pfmul       (r7, r0);   //; sqrt((1-z)/2)
    r0 = r2;                        //; duplicate mask
    r5 = _m_pand        (r5, r2);   //; z < 0.575 ? z : 0
    r0 = _m_pandn       (r0, r7);   //; z < 0.575 ? 0 : sqrt((1-z)/2)
    r7 = _m_from_int    (asp1);     //; asp1
    r0 = _m_por         (r0, r5);   //; z < 0.575 ? z : sqrt((1-z)/2)
    r1 = r0;                        //; save z
    r0 = _m_pfmul       (r0, r0);   //; z^2
    r5 = _m_from_int    (asp0);     //; asp0
    r7 = _m_pfmul       (r7, r0);   //; asp1 * z^2
    r4 = _m_pfadd       (r4, r0);   //; z^2 + asq1 
    r7 = _m_pfadd       (r7, r5);   //; asp1 * z^2 + asp0
    r5 = _m_from_int    (asq0);     //; asq0
    r7 = _m_pfmul       (r7, r0);   //; (asp1 * z^2 + asp0) * z^2
    r0 = _m_pfmul       (r0, r4);   //; (z^2 + asq1) * z^2
    r0 = _m_pfadd       (r0, r5);   //; qx = (z^2 + asq1) * z^2 + asq0
    r7 = _m_pfmul       (r7, r1);   //; z^3*px = (asp1 * z^2 + asp0) * z^3
    r4 = _m_pfrcp       (r0);       //; 1/qx approx
    r0 = _m_pfrcpit1    (r0, r4);   //; 1/qx step 
    r5 = _m_from_int    (npio2);    //; -pi/2
    r0 = _m_pfrcpit2    (r0, r4);   //; 1/qx final
    r0 = _m_pfmul       (r0, r7);   //; z^3*px/qx
    r4 = r2;                        //; z < 0.575 ? 0xfffffff : 0
    r2 = _m_pandn       (r2, r5);   //; z < 0.575 ? 0 : -pi/2
    r5 = _m_from_int    (pio2);     //; pi/2
    r7 = r4;                        //; z < 0.575 ? 0xfffffff : 0
    r2 = _m_pxor        (r2, r6);   //; z < 0.575 ? 0 : (sign ? pi/2 : -pi/2)
    r1 = _m_pfadd       (r1, r0);   //; r = z + z^3*px/qx
    r0 = _m_from_int    (oob);      //; special result for out of bound arguments
    r2 = _m_pfadd       (r2, r5);   //; z < 0.575 ? pi/2 : (sign ? pi : 0)
    r7 = _m_pslldi      (r7, 31);   //; z < 0.575 ? 0x80000000 : 0
    r4 = _m_pandn       (r4, r1);   //; z < 0.575 ? 0 : r
    r7 = _m_pxor        (r7, r6);   //; ((z < 0.575) != sign) ? 0x80000000 : 0
    r1 = _m_pfadd       (r1, r4);   //; z < 0.575 ? r : 2*r
    r0 = _m_pand        (r0, r3);   //; if abs(x) > 1 select special result
    r1 = _m_por         (r1, r7);   //; ((z < 0.575) != sign) ? -r,-2*r : r,2*r
    r2 = _m_pfadd       (r2, r1);   //; acos(x)
    r3 = _m_pandn       (r3, r2);   //; if abs(x) <= 1, select regular result
    r0 = _m_por         (r0, r3);   //; mux together results

    return r0;
}



//;******************************************************************************
//; Routine:  _m_asin
//; Input:    r0.lo
//; Result:   r0.lo
//; Uses:     r0-r7
//; Comment:
//;   Compute asin(x) using MMX and 3DNow! instructions.Scalar version.
//;
//;   If the input has an exponent of 0xFF, the result of this routine
//;   is undefined. Inputs with an exponent of 0 are treated as true
//;   zeroes and return a function value of 0. If the absolute value
//;   of the input is greater than 1, a special result is returned
//;   (currently this is 0). Results for arguments in [-1, 1] are in
//;   [-pi/2, pi/2].
//;
//;   asin(x)=sign(x)*asin(abs(x)). Let z=abs(x). If z>0.575, asin(z)=
//;   pi/2 - 2*asin(sqrt(0.5-0.5*z)). asin(z) for 0 <= z <= 0.575 is
//;   is approximated by a rational minimax approximation.
//;
//;   Testing shows that this function has an error of less than 3.25
//;   single precision ulps.
//;
//;******************************************************************************
__m64 _m_asin (__m64 r0)
{
    __m64 r1, r2, r3, r4, r5, r6, r7;

    r6 = _m_from_int    (sgn);      //; mask for sign bit
    r7 = _m_from_int    (mabs);     //; mask for absolute value
    r4 = _m_from_int    (half);     //; 0.5
    r6 = _m_pand        (r6, r0);   //; extract sign bit
    r5 = _m_from_int    (one);      //; 1.0
    r0 = _m_pand        (r0, r7);   //; z = abs(x)
    r3 = _m_pcmpgtd     (r0, r5);   //; z > 1.0 ? 0xFFFFFFFF : 0
    r5 = r0;                        //; save z
    r0 = _m_pfmul       (r0, r4);   //; z*0.5
    r2 = _m_from_int    (bnd);      //; 0.575
    r0 = _m_pfsubr      (r0, r4);   //; 0.5 - z * 0.5
    r7 = _m_pfrsqrt     (r0);       //; 1/sqrt((1-z)/2) approx
    r1 = r7;                        //; complete
    r7 = _m_pfmul       (r7, r7);   //;  reciprocal
    r2 = _m_pcmpgtd     (r2, r5);   //; z < 0.575 ? 0xfffffff : 0
    r7 = _m_pfrsqit1    (r7, r0);   //;   square root
    r4 = _m_from_int    (asq1);     //; asq1
    r7 = _m_pfrcpit2    (r7, r1);   //;    computation
    r7 = _m_pfmul       (r7, r0);   //; sqrt((1-z)/2)
    r0 = r2;                        //; duplicate mask
    r5 = _m_pand        (r5, r2);   //; z < 0.575 ? z : 0
    r0 = _m_pandn       (r0, r7);   //; z < 0.575 ? 0 : sqrt((1-z)/2)
    r7 = _m_from_int    (asp1);     //; asp1
    r0 = _m_por         (r0, r5);   //; z < 0.575 ? z : sqrt((1-z)/2)
    r1 = r0;                        //; save z
    r0 = _m_pfmul       (r0, r0);   //; z^2
    r5 = _m_from_int    (asp0);     //; asp0
    r7 = _m_pfmul       (r7, r0);   //; asp1 * z^2
    r4 = _m_pfadd       (r4, r0);   //; z^2 + asq1 
    r7 = _m_pfadd       (r7, r5);   //; asp1 * z^2 + asp0
    r5 = _m_from_int    (asq0);     //; asq0
    r7 = _m_pfmul       (r7, r0);   //; (asp1 * z^2 + asp0) * z^2
    r0 = _m_pfmul       (r0, r4);   //; (z^2 + asq1) * z^2
    r0 = _m_pfadd       (r0, r5);   //; qx = (z^2 + asq1) * z^2 + asq0
    r7 = _m_pfmul       (r7, r1);   //; z^3*px = (asp1 * z^2 + asp0) * z^3
    r4 = _m_pfrcp       (r0);       //; 1/qx approx
    r0 = _m_pfrcpit1    (r0, r4);   //; 1/qx step 
    r0 = _m_pfrcpit2    (r0, r4);   //; 1/qx final
    r4 = _m_from_int    (pio2);     //; pi/2
    r7 = _m_pfmul       (r7, r0);   //; z^3*px/qx
    r0 = _m_from_int    (oob);      //; special out-of-bounds result
    r1 = _m_pfadd       (r1, r7);   //; r = z + z^3*px/qx
    r5 = r1;                        //; save r
    r1 = _m_pfadd       (r1, r1);   //; 2*r
    r1 = _m_pfsubr      (r1, r4);   //; pi/2 - 2*r
    r5 = _m_pand        (r5, r2);   //; z < 0.575 ? r : 0
    r2 = _m_pandn       (r2, r1);   //; z < 0.575 ? 0 : pi/2 - 2 * r
    r0 = _m_pand        (r0, r3);   //; select special result if abs(x) > 1
    r2 = _m_por         (r2, r5);   //; z < 0.575 ? r : pi/2 - 2 * r
    r2 = _m_por         (r2, r6);   //; asin(x)=sign(x)*(z < 0.575 ? r : pi/2 - 2 * r)
    r3 = _m_pandn       (r3, r2);   //; select regular result if abs(x) <= 1
    r0 = _m_por         (r0, r3);   //; mux results together

    return r0;
}



//;******************************************************************************
//; Routine:  _m_log
//; Input:    r0.lo
//; Result:   r0.lo
//; Uses:     r0-r7
//; Comment:
//;   Compute log(abs(x)) using MMX and 3DNow! instructions. Scalar version.
//;
//;   If the input has an exponent of 0xFF, the result of this routine
//;   is undefined. Inputs with an exponent of 0 are treated as true
//;   zeros and return a result of (- max_normal). Underflow or over-
//;   flow can not occur otherwise.
//;
//;   The input x = 2^k * m, thus the natural logarithm is log(2^k) +
//;   log(m) = k*log(2) + log(m). Here, m is chosen such than m is <
//;   sqrt(2). Then, log(m) = 2*artanh(m+1)/(m-1). A polynomial minimax
//;   approximation is used to compute artanh(z). k*log(2) is computed
//;   with increased precision by splitting the constant log(2) into a
//;   16-bit high-order part and a 24-bit low-order part. The product
//;   of the high-order part and k is exactly representable.
//;
//;   Testing shows that this function has an error of less than 2.42
//;   single precision ulps.
//;
//;******************************************************************************
__m64 _m_log (__m64 r0)
{
    __m64 r1, r2, r3, r4, r5, r6, r7;

    r6 = _m_from_int    (mant);     //; mask for mantissa                  
    r4 = r0;                        //; save x                            
    r2 = _m_from_int    (expo);     //; mask for exponent      
    r0 = _m_pand        (r0, r6);   //; extract mantissa of x => m 
    r3 = _m_from_int    (one);      //; 1.0                            
    r4 = _m_pand        (r4, r2);   //; extract biased exponent of x => e 
    r0 = _m_por         (r0, r3);   //; float(m)                            
    r3 = _m_from_int    (rt2);      //; sqrt(2)                             
    r4 = _m_psrldi      (r4, 23);   //; biased exponent e               
    r2 = r0;                        //; save m                    
    r5 = izeros;                    //; create 0                     
    r6 = _m_from_int    (edec);     //; 0x0080000                         
    r0 = _m_pcmpgtd     (r0, r3);   //; m > sqrt(2) ? 0xFFFFFFFF : 0        
    r5 = _m_pcmpeqd     (r5, r4);   //; sel = (e == 0) ? 0xFFFFFFFFL : 0
    r4 = _m_psubd       (r4, r0);   //; increment e if m > sqrt(2)          
    r3 = _m_from_int    (bias);     //; 127                             
    r0 = _m_pand        (r0, r6);   //; m > sqrt(2) ? 0x00800000 : 0
    r6 = _m_from_int    (one);      //; 1.0                                
    r2 = _m_psubd       (r2, r0);   //; if m > sqrt(2),  m = m/2      
    r4 = _m_psubd       (r4, r3);   //; true exponent = i                  
    r0 = r2;                        //; save m                             
    r2 = _m_pfadd       (r2, r6);   //; m + 1
    r0 = _m_pfsub       (r0, r6);   //; m - 1                          
    r4 = _m_pi2fd       (r4);       //; float(i)
    r7 = _m_from_int    (ln2lo);    //; lower 24 bits of ln(2)
    r6 = _m_pfrcp       (r2);       //; approx 1/mm+1) lo
    r3 = _m_from_int    (ln2hi);    //; upper 16 bits of ln(2)
    r2 = _m_pfrcpit1    (r2, r6);   //; refine 1/mm+1) 
    r7 = _m_pfmul       (r7, r4);   //; i*ln2lo
    r3 = _m_pfmul       (r3, r4);   //; i*ln2hi
    r4 = _m_from_int    (c2);       //; c2             
    r2 = _m_pfrcpit2    (r2, r6);   //; 1/mm+1)        
    r1 = _m_from_int    (c1);       //; c1        
    r2 = _m_pfmul       (r2, r0);   //; z=mm+1)/mm-1)                         
    r0 = r2;                        //; save z                            
    r0 = _m_pfadd       (r0, r0);   //; 2*z
    r2 = _m_pfmul       (r2, r2);   //; z^2                             
    r6 = r2;                        //; save z^2                             
    r2 = _m_pfmul       (r2, r4);   //; c2 * z^2                          
    r4 = _m_from_int    (c0);       //; c0               
    r2 = _m_pfadd       (r2, r1);   //; c2 * z^2 + c1        
    r2 = _m_pfmul       (r2, r6);   //; (c2 * z^2 + c1) * z^2
    r1 = _m_from_int    (maxn);     //; maxn (negative largest normal)             
    r6 = _m_pfmul       (r6, r0);   //; 2*z^3
    r2 = _m_pfadd       (r2, r4);   //; px = (c2 * z^2 + c1) * z^2 + c0
    r2 = _m_pfmul       (r2, r6);   //; px*2*z^3
    r2 = _m_pfadd       (r2, r0);   //; px*2*z^3+2*z
    r0 = r5;                        //; sel                               
    r5 = _m_pand        (r5, r1);   //; select largest negative normal if e = 0
    r2 = _m_pfadd       (r2, r7);   //; px*2*z^3+2*z+i*ln2lo
    r2 = _m_pfadd       (r2, r3);   //; ln(z)=px*2*z^3+2*z+i*ln2lo+i*ln2hi
    r0 = _m_pandn       (r0, r2);   //; select regular result if e != 0
    r0 = _m_por         (r0, r5);   //; mux in either normal or special result

    return r0;
}



//;******************************************************************************
//; Routine:  _m_log10
//; Input:    r0
//; Result:   r0
//; Uses:     r0-r7, eax, ecx, edx
//; Comment:
//;   See a_log for the details of operation, this routine merely
//;   converts the result into a log base 10 by way of the mathematical
//;   identity log10(x) = ln(x)/ln(10).
//;
//;   Note that due to register contention/latancy issues, calling
//;   _a_log() is not noticeably worse than inlining the function.
//;
//;******************************************************************************
__m64 _m_log10 (__m64 r0)
{
    return _m_pfmul (_m_log (r0), _m_from_int   (rle10));
}




//;******************************************************************************
//; Routine:  _m_exp
//; Input:    r0.lo
//; Result:   r0.lo
//; Uses:     r0-r7
//; Comment:
//;  Compute exp(x) using MMX and 3DNow! instructions. Scalar version.
//;
//;  If the input has an exponent of 0xFF, the result of this routine
//;  is undefined. Inputs with an exponent of 0 are treated as true
//;  zeroes and return a function value of 1. If the input is less
//;  than -126*log(2) the result is flushed to zero; exp(-126*log(2))=
//;  2^-126, the smallest normalized single precision number. If the
//;  input is >= exp(128*log(2)), the result is clamped to max_norm.
//;
//;  exp(x) = exp(i*log(2)+z) = exp(z)*2^i, where i = trunc(x/log(2))
//;  and z = x-i*log(2), with -log(2) <= z <= log(2). The accurate
//;  computation of z is very important in the determination of exp(x)
//;  Therefore, log(2) is represented as a set of two floating-point
//;  numbers ln2hi and ln2lo, where ln2hi contains the most signifcant
//;  16 bits of log(2), and ln2lo the next 24 bits. z is therefore
//;  computed as z = ((x-i*ln2hi)-i*ln2lo), where the product i*ln2hi
//;  is exactly representable. exp(z) is computed by a Pade type
//;  rational minimax approximation, a(z)=2*z*P(z^2)/(Q(z^2)-z*P(z^2))
//;
//;  Testing shows that this function has an error of less than 1.70
//;  single precision ulps.
//;
//;******************************************************************************
__m64 _m_exp (__m64 r0)
{
    __m64 r1, r2, r3, r4, r5, r6, r7;

    r4 = _m_from_int    (ooln2);    //; 1/log(2)
    r6 = r0;                        //; x
    r1 = _m_from_int    (upper);    //; 88.72
    r5 = r0;                        //; x
    r3 = _m_from_int    (sgn);      //; mask for sign bit
    r4 = _m_pfmul       (r4, r0);   //; x/log(2)
    r6 = _m_pfcmpge     (r6, r1);   //; sel = (x >= 88.72) ? 0xFFFFFFFFL : 0
    r1 = _m_from_int    (half);     //; 0.5
    r3 = _m_pand        (r3, r0);   //; sign(x)
    r2 = _m_from_int    (lower);    //; -87.34
    r3 = _m_por         (r3, r1);   //; sign(x)*0.5
    r7 = _m_from_int    (ln2hi);    //; ln2hi
    r4 = _m_pfadd       (r4, r3);   //; x/log(2)+sign(x)*0.5
    r1 = _m_from_int    (ln2lo);    //; ln2lo
    r4 = _m_pf2id       (r4);       //; i=trunc(x/log(2)+sign(x)*0.5)=round(x/log(2))
    r5 = _m_pfcmpge     (r5, r2);   //; mask = (x >= -87.34) ? 0xFFFFFFFFL : 0
    r3 = _m_pi2fd       (r4);       //; xi = float(i)
    r7 = _m_pfmul       (r7, r3);   //; xi*ln2hi
    r1 = _m_pfmul       (r1, r3);   //; xi*ln2lo
    r3 = _m_from_int    (d0);       //; d0
    r0 = _m_pfsub       (r0, r7);   //; x-xi*ln2hi
    r7 = _m_from_int    (n1);       //; n1
    r0 = _m_pfsub       (r0, r1);   //; z=(x-xi*ln2hi)-xi*ln2lo
    r1 = r0;                        //; save z
    r0 = _m_pfmul       (r0, r0);   //; compute z^2
    r2 = r0;                        //; save z^2
    r0 = _m_pfadd       (r0, r3);   //; qx = z^2 + d0
    r3 = r2;                        //; save z^2
    r2 = _m_pfmul       (r2, r7);   //; n1 * x^2
    r7 = _m_from_int    (n0);       //; n0
    r2 = _m_pfadd       (r2, r7);   //; n1 * z^2 + n0
    r2 = _m_pfmul       (r2, r1);   //; px = (n1 * z^2 + n0) * z
    r0 = _m_pfsub       (r0, r2);   //; qx - px
    r1 = _m_pfrcp       (r0);       //; approx 1/(qx-px)
    r0 = _m_pfrcpit1    (r0, r1);   //; refine 1/(qx-px)
    r2 = _m_pfadd       (r2, r2);   //; 2*px
    r3 = _m_from_int    (one);      //; 1.0
    r0 = _m_pfrcpit2    (r0, r1);   //; 1/(qx-px)
    r4 = _m_pslldi      (r4, 23);   //; i = i << 23
    r2 = _m_pfmul       (r2, r0);   //; exp(z)-1 = 2*px / (qx - px)
    r0 = _m_from_int    (max);      //; max
    r2 = _m_pfadd       (r2, r3);   //; exp(z)
    r0 = _m_pand        (r0, r6);   //; if (x >= 88.72) select max norm
    r2 = _m_paddd       (r2, r4);   //; exp(z) * 2^i = exp(z)+exp(i*log(2))=exp(x)
    r2 = _m_pand        (r2, r5);   //; flush normal result to 0 if x < -87.34
    r6 = _m_pandn       (r6, r2);   //; if (x < 88.72) select normal result
    r0 = _m_por         (r0, r2);   //; mux together results ==> exp(x)

    return r0;
}



//;******************************************************************************
//; Routine:  _m_sqrt
//; Input:    r0
//; Result:   r0
//; Uses:     r0-r2
//; Comment:
//;   Uses the reciprical square root opcodes to compute a 24-bit square
//;   root.
//;
//;   Actual precision of this routine has not yet been tested.
//;
//;******************************************************************************
__m64 _m_sqrt (__m64 r0)
{
    __m64 r1;

    r1 = _m_pfrsqrt (r0);
    r1 = _m_pfrcpit2 (_m_pfrsqit1 (_m_pfmul (r1, r1),
                                   r0),
                      r1);
    return _m_pfmul (r0, r1);
}


//;******************************************************************************
//; Routine:  _m_fabs
//; Input:    r0
//; Result:   r0
//; Uses:     r0
//; Comment:
//;   Uses the fact that the high bit of an IEEE floating point number is
//;   a sign bit.  This routine clears the sign bit, making any input number
//;   non-negative.  The precision of the number is not affected.
//;
//;   Note: it is usually more efficient to simply code the 'pand' instruction
//;   inline, but this routine is provided here for completeness.
//;
//;******************************************************************************
__m64 _m_fabs (__m64 x)
{
    return _m_pand (x, sign_mask);
}



//;******************************************************************************
//; Routine:  a_ceil
//; Input:    r0
//; Result:   r0
//; Uses:     r0-r4
//; Comment:
//;   Returns the smallest whole number that is >= the input value.
//;******************************************************************************
__m64 a_ceil (__m64 x)
{
    __m64 r0, r1, r2;

    r2 = _m_pf2id   (x);            //; I = r2
    r2 = _m_pi2fd   (r2);
    r1 = _m_pand    (sh_masks, x);  //; r3 = sign bit
    r0 = _m_pfsub   (x, r2);        //; F   = r0
    r0 = _m_pfcmpgt (r0, izeros);   //; r0 = F > 0.0
    r0 = _m_pand    (r0, ones);     //; r0 = (F > 0) ? 1: 0
    r2 = _m_por     (r2, r1);       //; re-assert the sign bit
    r0 = _m_por     (r0, r1);       //; add sign bit

    return _m_pfadd (r0, r2);
}


//;******************************************************************************
//; Routine:  a_floor
//; Input:    r0
//; Result:   r0
//; Uses:     r0-r3
//; Comment:
//;   Returns the largest integer that is <= the input value.
//;******************************************************************************
__m64 a_floor (__m64 x)
{
    __m64 r0, r1;
    __m64 I, F;

    I  = _m_pf2id (x);              // I = x
    F  = _m_pi2fd (I);              // F = I
    r1 = _m_pfcmpgt (F, I);         // is F > I? (result becomes a bit mask)
    r0 = _m_pand (ones, r1);        // r0 = F > I ? 1 : 0

    return _m_pfsub (F, r0);        // r0 = I - (F > I ? 1 : 0)
}

       

//;******************************************************************************
//; Routine:  _m_frexp
//; Input:    r0 (num:???)
//; Result:   r0 (r:exp)
//; Uses:     r0-r3
//; Comment:
//;   Split the number into exponent(exp) and mantissa(r)
//;******************************************************************************
__m64 _m_frexp (__m64 r0)
{
    __m64 r1, r2, r3;

    r1 = r0;

    //; Isolate the mantissa (r0) and exponent (r1)
    r0 = _m_pand        (r0, smh_masks);
    r2 = PMOne;
    r1 = _m_pand        (r1, pinfs);
    r3 = HalfVal;
    r0 = _m_por         (r0, r2);
    r2 = two_126s;
    r1 = _m_psrldi      (r1, 23);
    r0 = _m_pfmul       (r0, r3);
    r1 = _m_psubd       (r1, r2);

    //; Pack the two back into the output register
    r0 = _m_punpckldq   (r0, r1);
    return r0;
}


//;******************************************************************************
//; Routine:  a_ldexp
//; Input:    r0 = x, r1 = exp
//; Result:   r0 = x+E(exp)
//; Uses:     r0, r2, eax, edx
//; Comment:
//;   Computes x * 2^exp, adding the exponent to the exponent of
//;   the input value.
//;   This version is scalar (rather than vector)
//;******************************************************************************
__m64 _m_ldexp (__m64 r0, __m64 r1)
{
    int tmp;
    __m64 r2;

    r2 = r0;
    r0 = _m_pand    (r0, sign_mask);
    r0 = _m_paddd   (_m_psrldi (r0, 23), r1);
    tmp = _m_to_int (r0);
    r2  = _m_pand   (r2, smh_masks);

    // If any bits too large were set...
    if (tmp & 0xffffff00)
    {
        if (tmp & MASKSH)
        {
            r0 = izeros;
        }
        else
        {
            //; restore original value, keep its sign bit
            //; r0 = (x < 0) ? -Inf: +Inf
            r0 = _m_por (_m_pand (r2, sh_masks), pinfs);
        }
    }
    else
    {
        r0 = _m_pslldi (r0, 23);       //; shift exponent back into place
        r0 = _m_paddd  (r0, r2);       //; assemble the new exponent and mantissa
    }

    return r0;
}




//;******************************************************************************
//; Routine:  _m_modf
//; Input:    r0.lo
//; Result:   r0 (res:res)
//; Uses:     r0,r1
//; Comment:
//;   Compute both the signed fractional part and the integral part of the input.
//;   Return both packed into r0.  Note that the "integral" part of the input
//;   is not an integer, but rather a float with no fractional component.
//;   Both return values will have the same sign.
//;
//;   Note: Performing a "pfacc r0,r0" would result in the same value as was
//;   passed to this routine.
//;
//;******************************************************************************
__m64 _m_modf (__m64 x)
{
    __m64 I = _m_pi2fd (_m_pf2id (x));  //; I = float (int (x))
    __m64 F = _m_pfsub (x, I);          //; F = F - I          
    return _m_punpckldq (F, I);         //; result F:I          
}


//;******************************************************************************
//; Routine:  a_fmod
//; Input:    r0 (x), r1 (y)
//; Result:   r0
//; Uses:     r0-r5
//; Comment:
//;   The fmod function calculates the float remainder of x / y such that
//;   x = ? * y + rem, where sign(f) = sign(x), and fabs(f) < fabs(y).
//;   The ? represents any integer, indicating that r0 can be greater than
//;   r1.
//;   Note that since 3DNow! instructions do not throw exceptions, a divide
//;   by 0 is perfectly acceptable.  However, it will generate undefined results,
//;   so we need to mask the result to 0 if one is detected.  This is
//;   accomplished by creating an AND mask using the compare (r1 != 0).  If
//;   r1 is not zero, then the computed result will pass.  Otherwise, only 
//;   zero will pass.
//;******************************************************************************
__m64 _m_fmod (__m64 r0, __m64 r1)
{
    __m64 r2, r3, r4, r5;

    r4 = izeros;                    //; load for compare against 0
    r2 = r0;                        //; r2 = x
    r5 = _m_pfcmpeq     (r5,r5);    //; r5 = all bits on (since r5 = r5)
    r3 = r1;                        //; r3 = y
    r0 = _m_pfrcp       (r1);       //; 1/y, stage 1
    r4 = _m_pfcmpeq     (r4,r1);    //; r4 = (y == 0)
    r1 = _m_pfrcpit1    (r1,r0);    //; 1/y, stage 2
    r1 = _m_pfrcpit2    (r1,r0);    //; r1 = 1/y
    r5 = _m_pxor        (r5,r4);    //; r5 = (y != 0)
    r2 = _m_pfmul       (r2,r1);    //; r2 = x/y

    //; This stage is identical to modf() above
    r0 = _m_from_int    (pnan);     //; r0 = nan
    r1 = _m_pf2id       (r2);       //; int(x/y)
    r0 = _m_pand        (r0,r4);    //; r0 = (y == 0) ? nan : 0
    r1 = _m_pi2fd       (r1);       //; float(int(x/y))
    r2 = _m_pfsub       (r2,r1);    //; modf(x/y) (the fractional part)

    //; Multiply the fraction by 'y'
    r3 = _m_pfmul       (r3,r2);    //; y * modf(x/y)

    //; Select correct result (r1 == 0) ? inf : result
    r3 = _m_pand        (r3,r5);    //; r3 = (y == 0) ? 0: result
    r0 = _m_por         (r0,r3);    //; r0 = (y == 0) ? nan : result

    return r0;
}



//;******************************************************************************
//; Routine:  a_sincos/a_cos
//; Input:    r0.lo
//; Result:   r0 (cos|sin)
//; Uses:     r0-r7, eax, ebx, ecx, edx, esi
//; Comment:
//;   sincos computes both sin and cos simultaneously.
//;   Since the cos value is returned in r0.lo, the "a_cos()" routine
//;   is really an alias for a_sincos().  The only difficulty is that
//;   a_cos does not unpack its result into both halves of r0.  Since
//;   most usages don't need the vectorization, the instruction has been
//;   left out.
//;******************************************************************************
__m64 _m_sincos (__m64 r0)
{
    return SINCOSMAC (r0);
}



//;******************************************************************************
//; Routine:  _m_sin
//; Input:    r0.lo
//; Result:   r0 (sin|sin)
//; Uses:     r0-r7, eax, ebx, ecx, edx, esi
//;******************************************************************************
__m64 _m_sin (__m64 r0)
{
    r0 = SINCOSMAC (r0);
    return _m_punpckhdq (r0,r0);   //; select sin value
}


//;******************************************************************************
//; Routine:  _m_cos
//; Input:    r0.lo
//; Result:   r0 (cos|cos)
//; Uses:     r0-r7, eax, ebx, ecx, edx, esi
//;******************************************************************************
__m64 _m_cos (__m64 r0)
{
    r0 = SINCOSMAC (r0);
    return _m_punpckldq (r0,r0);   //; select cos value
}


//;******************************************************************************
//; Routine:  _m_tan
//; Input:    r0.lo
//; Result:   r0 = tan|???
//; Uses:     r0-r7, eax, ebx, ecx, edx, esi
//; Comment:
//;   Yet another spawn of the SINCOSMAC macro, a_atan computes the arctangent
//;   of its input parameter using the definition  tan(x) = sin(x) / cos(x).
//;******************************************************************************
__m64 _m_tan (__m64 r0)
{
    __m64 r1, r2;
    r1 = r0 = SINCOSMAC (r0);            //; r1 = sincos(x)
    r2 = _m_pfrcp       (r1);       //; r2 = 1/cos(x), stage 1
    r1 = _m_punpckhdq   (r1,r1);    //; r1 = sin(x)
    r0 = _m_pfrcpit1    (r0,r2);    //; r0 = 1/cos(x), stage 2
    r0 = _m_pfrcpit2    (r0,r2);    //; r0 = 1/cos(x), stage 3
    r0 = _m_pfmul       (r0,r1);    //; tan(x) = sin(x) / cos(x)

    return r0;
}



//;******************************************************************************
//; Routine:  _m_pow - compute pow(r0.lo,r1.lo)
//; Input:    r0.lo - base
//;           r1.lo - exponent
//; Result:   stored in r0.lo
//; Uses:     r0-r7
//; Comment:
//;   Compute pow(x,y) using MMX and 3DNow! instructions. Scalar version.
//;
//;   This routine is an almost perfect replacement for the C library
//;   function pow(). The main differences are:
//;
//;   o If one of the inputs has an exponent of 0xFF, the result of
//;     this routine is undefined. Inputs with an exponent of 0 are
//;     treated as true zeros.
//;   o Flushes underflowed results to 0 and clamps overflowed results
//;     to the maximum single precision normal.
//;   o No error and different result for certain undefined cases,
//;     (e.g. negative x raised to non-integer power y; In particular
//;     0 ^ y = 0, for all y).
//;
//;   This routine uses inline versions of y*log(abs(x)) and 2^x
//;   routines which may also be used seperately. It computes pow(x,y)
//;   as x^y = 2^y*log2(x)). This is fast, but numerically this is not
//;   the optimal method.
//;
//;   This routine has a worst case accuracy of about 16 bits, but
//;   for many arguments the accuracy is significantly better, often
//;   19 bits or more.
//;
//;******************************************************************************
__m64 _m_pow (__m64 r0, __m64 r1)
{
    __m64 r, r2, r3, r4, r5, r6, r7;

    r = r1;                         //; y
    r = _m_pf2id    (r);            //; int(y)
    r = _m_pslldi   (r, 31);        //; (int(y)&1)<<31
    r = _m_pand     (r, r0);        //; bit<31> = (x<0) && (y&1)

//;       Compute y*log2(abs(x)) using MMX and 3DNow! instructions. Scalar version.
//;
//;       If one of the inputs has an exponent of 0xFF, the result of this
//;       routine is undefined. Inputs with an exponent of 0 are treated
//;       as true zeros. The following cases occur:
//;
//;       expo(x) == 0 && expo(y) == 0  --> y*log2(abs(x)) = - max_normal
//;       expo(x) == 0 && expo(y) != 0  --> y*log2(abs(x)) = - max_normal
//;       expo(x) != 0 && expo(y) == 0  --> y*log2(abs(x)) = 0
//;
//;       Results whose absolute value is less than min_normal are flushed
//;       to zero. Results whose absolute value exceeds max_normal are
//;       clamped to +/- max_normal.
//;
//;       The input x = 2^k * m, thus log2(x) = k + log2mm). log2mm) =
//;       log2(e)*logmm). Here, m is chosen such than m is < sqrt(2) and
//;       k is ajusted accordingly. Then, logmm) = 2*artanhmm-1)/mm+1). A
//;       polynomial minimax approximation is used to compute artanh(z),
//;       where z = mm-1)/mm+1)
//;
//;       Testing shows that this function has an error of less than 3.75
//;       single precision ulps.
//;
//;       input      r0.lo   argument x
//;                  r1.lo   argument y
//;       output     r0.lo   result y*log2(x)
//;       destroys   r0, r2, r3, r4, r5, r7

    r5 = _m_from_int    (mant);     //; mask for mantissa
    r4 = r0;                        //; save x
    r2 = _m_from_int    (expo);     //; mask for exponent
    r0 = _m_pand        (r0, r5);   //; extract mantissa of x => m
    r3 = _m_from_int    (one);      //; 1.0
    r4 = _m_pand        (r4, r2);   //; extract biased exponent of x => e
    r0 = _m_por         (r0, r3);   //; floatmm)
    r3 = _m_from_int    (rt2);      //; sqrt(2)
    r4 = _m_psrldi      (r4, 23);   //; biased exponent e
    r2 = r0;                        //; save m
    r5 = izeros;                    //; create 0
    r0 = _m_pcmpgtd     (r0, r3);   //; m > sqrt(2) ? 0xFFFFFFFF : 0
    r5 = _m_pcmpeqd     (r5, r4);   //; sel = (e == 0) ? 0xFFFFFFFFL : 0
    r3 = _m_from_int    (edec);     //; 0x0080000
    r4 = _m_psubd       (r4, r0);   //; increment e if m > sqrt(2)
    r0 = _m_pand        (r0, r3);   //; m > sqrt(2) ? 0x00800000 : 0
    r3 = _m_from_int    (bias);     //; 127
    r2 = _m_psubd       (r2, r0);   //; if m > sqrt(2),  m = m/2
    r4 = _m_psubd       (r4, r3);   //; true exponent = i
    r3 = _m_from_int    (one);      //; 1.0   
    r0 = r2;                        //; save m
    r2 = _m_pfadd       (r2, r3);   //; m + 1  
    r0 = _m_pfsub       (r0, r3);   //; m - 1 
    r7 = _m_from_int    (c2);       //; c2    
    r3 = _m_pfrcp       (r2);       //; approx 1/mm+1) 
    r2 = _m_pfrcpit1    (r2, r3);   //; refine 1/mm+1) 
    r4 = _m_pi2fd       (r4);       //; float(i)
    r2 = _m_pfrcpit2    (r2, r3);   //; 1/mm+1)
    r0 = _m_pfmul       (r0, r2);   //; z=mm-1)/mm+1)
    r2 = r0;                        //; save z
    r0 = _m_pfmul       (r0, r0);   //; z^2
    r3 = r0;                        //; save z^2
    r0 = _m_pfmul       (r0, r7);   //; c2 * z^2
    r7 = _m_from_int    (tl2e);     //; 2*log2(e)
    r2 = _m_pfmul       (r2, r7);   //; z * 2 * log2(e)
    r7 = _m_from_int    (c1);       //; c1
    r0 = _m_pfadd       (r0, r7);   //; c2 * z^2 + c1        
    r7 = _m_from_int    (c0);       //; c0               
    r0 = _m_pfmul       (r0, r3);   //; (c2 * z^2 + c1) * z^2
    r3 = _m_pfmul       (r3, r2);   //; z^3 * 2 * log2(e)       
    r0 = _m_pfadd       (r0, r7);   //; px = (c2 * z^2 + c1) * z^2 + c0
    r7 = _m_from_int    (maxn);     //; maxn (largest normal)             
    r3 = _m_pfmul       (r3, r0);   //; px*z^3*2*log2(e)                 
    r0 = r5;                        //; sel                               
    r2 = _m_pfadd       (r2, r3);   //; px*z^3*2*log2(e)+z*2*log2(e)         
    r5 = _m_pand        (r5, r7);   //; select largest negative normal if e = 0
    r2 = _m_pfadd       (r2, r4);   //; log2(x)=px*z^3*2*log2(e)+z*2*log2(e)+i 
    r0 = _m_pandn       (r0, r2);   //; select regular result if e != 0
    r0 = _m_pfmul       (r0, r1);   //; r = y * log2(x)                     
    r0 = _m_por         (r0, r5);   //; mux in either normal or special result 

//;       Compute 2^r using MMX and 3DNow! instructions. Scalar version.
//;
//;       If the input has an exponent of 0xFF, the result of this routine
//;       is undefined. Inputs with an exponent of 0 are treated as true
//;       zeroes and return a function value of 1. If the input is < -126
//;       the result is flushed to zero, as 2^-126 is the smallest normal
//;       SP number. If the input is >= 128, the result is clamped to
//;       max_norm.
//;
//;       Testing shows that this function has an error of less than 4 SP
//;       ulps, meaning that the accuracy is 22 bits or better.
//;
//;       2^r = 2^(i+x), where i = trunc(x), and -1 <= x <= 1. Then, 2^r =
//;       2^i * 2^x = 2^i * ((2^x-1)+1). 2^x-1 is approximated by a Pade
//;       type rational minimax approximation.
//;
//;       input      r0.lo   argument r
//;       output     r0.lo   result 2^r
//;       destroys   r0, r1, r2, r3, r4, r5, r7

    r4 = _m_pf2id       (r0);       //; i = trunc(r)   
    r5 = r0;                        //; r                
    r1 = _m_pi2fd       (r4);       //; float(trunc(r))   
    r3 = _m_from_int    (q1);       //; q1             
    r0 = _m_pfsub       (r0, r1);   //; x = frac(r)    
    r1 = r0;                        //; save x           
    r0 = _m_pfmul       (r0, r0);   //; compute x^2       
    r7 = _m_from_int    (p1);       //; p1             
    r2 = r0;                        //; save x^2       
    r0 = _m_pfadd       (r0, r3);   //; x^2 + q1       
    r3 = r2;                        //; save x^2        
    r2 = _m_pfmul       (r2, r7);   //; p1 * x^2        
    r7 = _m_from_int    (p0);       //; p0             
    r0 = _m_pfmul       (r0, r3);   //; (x^2 + q1) * x^2
    r3 = _m_from_int    (q0);       //; q0              
    r2 = _m_pfadd       (r2, r7);   //; p1 * x^2 + p0   
    r7 = _m_from_int    (two);      //; 2.0             
    r0 = _m_pfadd       (r0, r3);   //; qx = (x^2 + q1) * x^2 + q0                    
    r2 = _m_pfmul       (r2, r1);   //; px = (p1 * x^2 + p0) * x           
    r0 = _m_pfsub       (r0, r2);   //; qx - px         
    r2 = _m_pfmul       (r2, r7);   //; 2*px              
    r1 = _m_pfrcp       (r0);       //; approx 1/(qx-px)
    r7 = _m_from_int    (one);      //; 1.0                
    r0 = _m_pfrcpit1    (r0, r1);   //; refine 1/(qx-px)
    r4 = _m_pslldi      (r4, 23);   //; i = i << 23 (shift into exponent)
    r0 = _m_pfrcpit2    (r0, r1);   //; 1/(qx-px)                            
    r3 = _m_from_int    (mine);     //; mine                                 
    r2 = _m_pfmul       (r2, r0);   //; 2xm1 = 2*px / (qx - px)
    r1 = _m_from_int    (maxe);     //; maxe                              
    r2 = _m_pfadd       (r2, r7);   //; 2^x                                
    r0 = r5;                        //; r
    r5 = _m_pfcmpge     (r5, r3);   //; mask = (r >= -126) ? 0xFFFFFFFFL : 0
    r0 = _m_pfcmpge     (r0, r1);   //; sel = (r >= 128) ? 0xFFFFFFFFL : 0
    r1 = _m_from_int    (max);      //; max
    r4 = _m_paddd       (r4, r2);   //; 2^x * 2^i = 2^(x+i)
    r1 = _m_pand        (r1, r0);   //; if (r >= 128) select max norm
    r5 = _m_pand        (r5, r4);   //; flush normal result to 0 if r < -126
    r0 = _m_pandn       (r0, r5);   //; if (r < 128) select normal result
    r0 = _m_por         (r0, r1);   //; mux together result                 
     
//;       Here we basically have pow(x,y). But in case y is an odd integer
//;       and x is negative, the sign bit is wrong. Correct it here using
//;       a mask we constructed when we entered pow() based on the values
//;       of x and y.
                                
    return _m_pxor      (r0, r);    //; invert sign if ((x<0) && (y&1)) ==> pow(x,y)
}

// eof
