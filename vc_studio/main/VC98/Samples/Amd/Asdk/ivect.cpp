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
//; JFDCT
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

#include "avector.h"

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
    __asm
    {
        femms
        mov         eax,[a]
        mov         edx,[b]
        movq        mm0,[eax]
        movd        mm1,[eax+8]
        mov         eax,[r]
        movd        mm2,[edx+8]
        pfadd       mm0,[edx]
        pfadd       mm1,mm2
        movq        [eax],mm0
        movd        [eax+8],mm1
        femms
    }
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
    __asm
    {
        femms
        mov         eax,[a]
        mov         edx,[b]
        movq        mm0,[eax]
        movd        mm1,[eax+8]
        mov         eax,[r]
        pfsub       mm0,[edx]
        movd        mm2,[edx+8]
        pfsub       mm1,mm2
        movq        [eax],mm0
        movd        [eax+8],mm1
        femms
    }
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
    __asm
    {
        femms
        mov         eax,[a]
        mov         edx,[b]
        movq        mm0,[eax]
        movd        mm1,[eax+8]
        pfmul       mm0,[edx]
        movd        mm2,[edx+8]
        mov         eax,[r]
        pfmul       mm1,mm2
        movq        [eax],mm0
        movd        [eax+8],mm1
        femms
    }
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
    __asm
    {
        femms
        mov         eax,[a]
        movd        mm0,[f]
        movq        mm3,[eax]
        punpckldq   mm0,mm0
        movd        mm2,[eax+8]
        movq        mm1,mm0
        pfmul       mm0,mm3
        mov         eax,[r] 
        pfmul       mm1,mm2 
        movq        [eax],mm0
        movd        [eax+8],mm1
        femms
    }
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
    __asm
    {
        femms
        mov         eax,[a]
        mov         edx,[r]
        movq        mm0,[eax]
        movd        mm1,[eax+8]
        movq        mm4,mm0
        movq        mm3,mm1
        pfmul       mm0,mm0 
        pfmul       mm1,mm1 
        pfacc       mm0,mm0 
        pfadd       mm0,mm1 
        movd        eax,mm0
        pfrsqrt     mm1,mm0 
        movq        mm2,mm1
        cmp         eax,0038D1B717h     //; = 1.0 / 10000.0
        jl          short zero_mag

        pfmul       mm1,mm1
        pfrsqit1    mm1,mm0
        pfrcpit2    mm1,mm2
        punpckldq   mm1,mm1
        pfmul       mm3,mm1
        pfmul       mm4,mm1
        movd        [edx+8],mm3
        movq        [edx],mm4
        femms
    }
    return;

    __asm
    {
        ALIGN       16
zero_mag:
        mov         eax,[a]
        movq        mm0,[eax]
        movd        mm1,[eax+8]
        movq        [edx],mm0
        movd        [edx+8],mm1
        femms
    }
}


//;******************************************************************************
//; Routine:  a_mag_vect
//; Input:    eax - input vector pointer
//; Output:   mm0 - vector magnitude
//; Uses:     mm0 - mm2
//;******************************************************************************
__declspec (naked) void a_mag_vect ()
{
    __asm
    {
		femms		
        movq        mm0,[eax]
        movd        mm1,[eax+8]
        pfmul       mm0,mm0
        pfmul       mm1,mm1
        pfacc       mm0,mm0
        pfadd       mm0,mm1
        pfrsqrt     mm1,mm0
        movq        mm2,mm1
        pfmul       mm1,mm1
        pfrsqit1    mm1,mm0
        pfrcpit2    mm1,mm2
        pfmul       mm0,mm1
		femms
        ret
    }
}



//;******************************************************************************
//; Routine:  a_dot_vect
//; Input:    eax - input vector pointer
//;           edx - input vector pointer
//; Output:   mm0 - dot product of the two vectors (eax DOT edx)
//; Uses:     mm0-mm3
//;******************************************************************************
__declspec (naked) void a_dot_vect ()
{
    __asm
    {
		femms
        movq        mm0,[eax]
        movq        mm3,[edx]
        movd        mm1,[eax+8]
        movd        mm2,[edx+8]
        pfmul       mm0,mm3
        pfmul       mm1,mm2
        pfacc       mm0,mm0
        pfadd       mm0,mm1
		femms
        ret
    }
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
    __asm
    {
        femms
        mov         edx,[b]
        mov         eax,[a]
        movd        mm0,[edx+8]
        movd        mm1,[eax+8]
        punpckldq   mm0,mm0
        movd        mm3,[eax+4]
        punpckldq   mm1,mm1
        pfmul       mm0,[eax]
        movq        mm5,[edx]
        movd        mm4,[edx+4]
        pfmul       mm1,mm5
        movq        mm2,mm0
        pfmul       mm3,[edx]
        pfsub       mm0,mm1
        pfmul       mm4,[eax]
        mov         eax,[r]
        pfsub       mm1,mm2
        punpckhdq   mm0,mm0
        pfsub       mm4,mm3
        punpckldq   mm0,mm1
        movd        [eax+8],mm4
        movd        [eax+4],mm1
        movd        [eax],mm0
        femms
    }
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
    __asm
    {
        femms
        mov         eax,[n]         //; eax = normal vector
        mov         edx,[v]         //; edx = vector to reflect

//;     const float dot2 = 2.0f * (n->x * v->x + n->y * v->y + n->z * v->z);

        //;*** Standard DOT product code from above
        //; mm0 = n DOT v
        movq        mm0,[eax]       //;  y | x
        movq        mm3,[edx]       //;  y | x
        movd        mm1,[eax+8]     //;  0 | z
        movd        mm2,[edx+8]     //;  0 | z
        pfmul       mm0,mm3         //; yy | xx
        pfmul       mm1,mm2         //;  0 | zz

        //; 4 ticks latency on pfmul - hoist some code into here to prevent stalling
        mov         ecx,[r]         //; ecx = resultant vector
        movq        mm5,[eax]       //; mm5 = n->y | n->x
        movq        mm2,[edx]       //; mm2 = v->y | v->x
        //; end hoist

        pfacc       mm0,mm0         //; xx + yy
        movd        mm3,[eax+8]     //; mm3 = 0 | n->x (hoist for pfacc latency)
        pfadd       mm0,mm1         //; mm0 = n DOT v (xx + yy + zz)
        //;*** end n DOT v

        movd        mm4,[edx+8]     //; mm4 = 0 | v->z (hoist for pfadd latency)
        punpckldq   mm0,mm0         //; mm0 = dot | dot
        pfadd       mm0,mm0         //; mm0 = dot * 2 | dot * 2

//;       r->x   = dot2 * n->x - v->x;
//;       r->y   = dot2 * n->y - v->y;
//;       r->z   = dot2 * n->z - v->z;

        pfmul       mm5,mm0         //; mm5 = dot2 * n->y | dot2 * n->x
        pfmul       mm3,mm0         //; mm3 = 0 | dot2 * n->z
        pfsub       mm5,mm2         //; mm5 = dot2 * n->y - v->y | dot2 * n->x - v->x
        pfsub       mm3,mm4         //; mm3 = 0 | dot2 * n->z - v->z
        movq        [ecx],mm5       //; r->x = dot2 * n->x - v->x
                                    //; r->y = dot2 * n->y - v->y
        movd        [ecx+8],mm3     //; r->z = dot2 * n->z - v->z
        femms
    }
}

// eof