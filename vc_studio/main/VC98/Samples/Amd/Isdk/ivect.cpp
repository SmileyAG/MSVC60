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
//; IVECT.CPP
//;
//; AMD3D 3D library code: Vector Math Functions
//;
//; Vector types are based on Direct3D vectors for greater compatability with
//; game code.  For non-Direct3D applications, a simple typedef such as the
//; following will suffice:
//;
//; typedef struct { float x, y, z; } D3DVECTOR;
//;
//; These routines will only ever access three elements (12 bytes) of any
//; structure passed, although it is recommended that the user pad data elements
//; to 16 byte boundaries for performance reasons.
//;
//;******************************************************************************

#include <math.h>
#include <mm3dnow.h>
#include "avector.h"

#pragma warning (disable:4799)
#pragma warning (disable:4730)

#define _m_from_floats(f) (*(__m64 *)(f))
#define _m_to_float(f)  ((f).m64_f32[0])
#define _m_from_float(f)  _m_from_int (*(int *)&(f))


//;******************************************************************************
//; Routine:  void _add_vect (D3DVECTOR *r, const D3DVECTOR *a, const D3DVECTOR *b)
//; Input:    r   - resultant vector
//;           a   - input vector
//;           b   - input vector
//; Output:   r.i = a.i + b.i, for i in (x, y, z)
//; Uses:     eax, edx, mm0 - mm2
//;******************************************************************************
void _add_vect (D3DVECTOR *r, const D3DVECTOR *a, const D3DVECTOR *b)
{
    _m_femms();
    ((__m64 *)r)[0] = _m_pfadd (((__m64 *)a)[0], ((__m64 *)b)[0]);
    r->z = _m_to_float (_m_pfadd (_m_from_float (a->z), _m_from_float (b->z)));
    _m_femms();
}


//;******************************************************************************
//; Routine:  void _sub_vect (D3DVECTOR *r, const D3DVECTOR *a, const D3DVECTOR *b)
//; Input:    r   - resultant vector
//;           a   - input vector
//;           b   - input vector
//; Output:   r.i = a.i - b.i, for i in (x, y, z)
//; Uses:     eax, edx, mm0 - mm2
//;******************************************************************************
void _sub_vect (D3DVECTOR *r, const D3DVECTOR *a, const D3DVECTOR *b)
{
    _m_femms();
    ((__m64 *)r)[0] = _m_pfsub (((__m64 *)a)[0], ((__m64 *)b)[0]);
    r->z = _m_to_float (_m_pfsub (_m_from_float (a->z), _m_from_float (b->z)));
    _m_femms();
}



//;******************************************************************************
//; Routine:  void _mult_vect (D3DVECTOR *r, const D3DVECTOR *a, const D3DVECTOR *b)
//; Input:    r   - resultant vector
//;           a   - input vector
//;           b   - input vector
//; Output:   r.i = a.i * b.i, for i in (x, y, z)
//; Uses:     eax, edx, mm0 - mm2
//;******************************************************************************
void _mult_vect (D3DVECTOR *r, const D3DVECTOR *a, const D3DVECTOR *b)
{
    _m_femms();
    ((__m64 *)r)[0] = _m_pfmul (((__m64 *)a)[0], ((__m64 *)b)[0]);
    r->z = _m_to_float (_m_pfmul (_m_from_float (a->z), _m_from_float (b->z)));
    _m_femms();
}


//;******************************************************************************
//; Routine:  void _scale_vect (D3DVECTOR *r, const D3DVECTOR *a, float f)
//; Input:    r   - resultant vector
//;           a   - input vector
//;           f   - scale factor
//; Output:   r = copy of 'a' with each element multiplied by 'f'
//; Uses:     eax, mm0 - mm3
//;******************************************************************************
void _scale_vect (D3DVECTOR *r, const D3DVECTOR *a, float f)
{
    _m_femms();
    __m64 v = _m_from_float (f);
    v = _m_punpckldq (v, v);

    __m64 *const pr = (__m64 *)r;
    pr[0] = _m_pfmul (_m_from_floats (&a->x), v);
    r->z  = _m_to_float (_m_pfmul (_m_from_float (a->z), v));
    _m_femms();
}


//;******************************************************************************
//; Routine:  void _norm_vect (D3DVECTOR *r, const D3DVECTOR *a)
//; Input:    r - resultant vector
//;           a - input vector
//; Output:   r = normalize('a'), equals 'a' if mag('a') < 0.00001
//; Uses:     eax, edx, mm0 - mm4
//;******************************************************************************
void _norm_vect (D3DVECTOR *r, const D3DVECTOR *a)
{
    _m_femms();
    __m64 a_lo = _m_from_floats (&a->x);
    __m64 a_hi = _m_from_float  (a->z);
    __m64 mag  = _m_pfadd (_m_pfmul (a_lo, a_lo),
                           _m_pfmul (a_hi, a_hi));
    __m64 tmp  = _m_pfacc (mag, mag);

    mag = _m_pfrsqrt (tmp);
    mag = _m_pfrcpit2 (_m_pfrsqit1 (_m_pfmul (mag, mag),
                                    tmp),
                       mag);

    __m64 *const pr = (__m64 *)r;
    pr[0] = _m_pfmul (a_lo, mag);
    r->z = _m_to_float (_m_pfmul (a_hi, mag));
    _m_femms();
}


//;******************************************************************************
//; Routine:  _m_mag_vect
//; Input:    eax - input vector pointer
//; Output:   mm0 - vector magnitude
//; Uses:     mm0 - mm2
//;******************************************************************************
__m64 _m_mag_vect (__m64 *p)
{
    __m64 r0, r1;

    r0 = p[0];
    r1 = _m_from_float (*(float *)(p+1));
    r0 = _m_pfmul (r0, r0);
    r1 = _m_pfmul (r1, r1);
    r0 = _m_pfacc (r0, r0);
    r0 = _m_pfadd (r0, r1);

    r1 = _m_pfrsqrt (r0);
    r1 = _m_pfrcpit2 (_m_pfrsqit1 (_m_pfmul (r1, r1),
                                   r0),
                      r1);
    r0 = _m_pfmul (r0, r1);

    return r0;
}



//;******************************************************************************
//; Routine:  _m_dot_vect
//; Input:    eax - input vector pointer
//;           edx - input vector pointer
//; Output:   mm0 - dot product of the two vectors (eax DOT edx)
//; Uses:     mm0-mm3
//;******************************************************************************
__m64 _m_dot_vect (__m64 *a, __m64 *b)
{
    __m64 r0, r1;
    r0 = _m_pfmul (a[0], b[0]);
    r1 = _m_pfmul (_m_from_float (*(float *)&a[1]),
                   _m_from_float (*(float *)&b[1]));
    r0 = _m_pfacc (r0, r0);
    r0 = _m_pfadd (r0, r1);

    return r0;
}



//;******************************************************************************
//; Routine:  void _cross_vect (D3DVECTOR *r, const D3DVECTOR *a, const D3DVECTOR *b)
//; Input:    r - resultant vector
//;           a - input vector
//;           b - input vector
//; Output:   r = 'a' cross 'b'
//; Uses:     eax, edx, mm0 - mm5
//;******************************************************************************
void _cross_vect (D3DVECTOR *r, const D3DVECTOR *a, const D3DVECTOR *b)
{
    __m64 r0, r1, r3, r4, tmp;

    _m_femms();

    r0 = _m_from_float (b->z);
    r1 = _m_from_float (a->z);
    r0 = _m_punpckldq  (r0, r0);
    r3 = _m_from_float (a->y);
    r1 = _m_punpckldq  (r1, r1);

    r0 = _m_pfmul (r0, _m_from_floats (&a->x));
    r1 = _m_pfmul (r1, _m_from_floats (&b->x));
    tmp = r0;
    r3 = _m_pfmul (r3, _m_from_floats (&b->x));
    r0 = _m_pfsub (r0, r1);
    r4 = _m_pfmul (_m_from_float  (b->y),
                   _m_from_floats (&a->x));
    r0 = _m_punpckhdq (r0, r0);

    r->z = _m_to_float (_m_pfsub (r4, tmp));
    r->y = _m_to_float (_m_pfsub (r1, tmp));
    r->x = _m_to_float (_m_punpckldq (r0, r1));

    _m_femms();
}



//;******************************************************************************
//; Routine:  void _reflect_vect (D3DVECTOR *r, const D3DVECTOR *v, const D3DVECTOR *n)
//; Input:    r - resultant vector
//;           v - input vector to be reflected
//;           n - Normal to the surface reflected off of
//; Output:   r = v reflected off of n
//;     or,   r = 2 * (n DOT v) * n - v
//; Uses:     mm0-mm5
//; Comment:  This routine has a lot of hoisted sections to try and
//;           account for instruction latencies.  Unfortunately, there aren't
//;           enough "spare" instructions to fully optimize this routine, due
//;           to its simple nature.
//;******************************************************************************
void _reflect_vect (D3DVECTOR *r, const D3DVECTOR *v, const D3DVECTOR *n)
{
    __m64 r0, r1, r2, r3, r4, r5;
    __m64 *pr = (__m64 *)r;

    _m_femms();

//;     const float dot2 = 2.0f * (n->x * v->x + n->y * v->y + n->z * v->z);
    //*** Standard DOT product code from above
    r0 = _m_pfmul (_m_from_floats (&n->x),
                   _m_from_floats (&v->x));
    r0 = _m_pfacc (r0, r0);
    r0 = _m_pfadd (r0,
                   _m_pfmul (_m_from_floats (&n->z),
                             _m_from_floats (&v->z)));
    //*** End DOT

    r5 = _m_from_floats (&n->x);    //; mm5 = n->y | n->x
    r2 = _m_from_floats (&v->x);    //; mm2 = v->y | v->x
    r3 = _m_from_float  (n->z);     //; mm3 = 0 | n->z (hoist for pfacc latency)
    r4 = _m_from_float  (v->z);     //; mm4 = 0 | v->z (hoist for pfadd latency)
    r0 = _m_punpckldq   (r0, r0);   //; mm0 = dot | dot
    r0 = _m_pfadd       (r0, r0);   //; mm0 = dot * 2 | dot * 2

//;       r->x   = dot2 * n->x - v->x;
//;       r->y   = dot2 * n->y - v->y;
//;       r->z   = dot2 * n->z - v->z;

    r5 = _m_pfmul (r5, r0);         //; mm5 = dot2 * n->y | dot2 * n->x
    r3 = _m_pfmul (r3, r0);         //; mm3 = 0 | dot2 * n->z
    r5 = _m_pfsub (r5, r2);         //; mm5 = dot2 * n->y - v->y | dot2 * n->x - v->x
    r3 = _m_pfsub (r3, r4);         //; mm3 = 0 | dot2 * n->z - v->z
    pr[0] = r5;                     //; r->x = dot2 * n->x - v->x
                                    //; r->y = dot2 * n->y - v->y

    r->z = _m_to_float (r3);        //; r->z = dot2 * n->z - v->z

    _m_femms();
}

// eof
