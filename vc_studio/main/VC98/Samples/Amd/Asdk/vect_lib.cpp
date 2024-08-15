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

*******************************************************************************

 VECT_LIB.C

 AMD3D 3D library code: Vector math
    The majority of these routines are in vect.asm - this file only
    provides a C wrapper for functions needing to return a float value.

******************************************************************************/

#include <math.h>
#include "avector.h"


/******************************************************************************
 Routine:   _mag_vect
 Input:     a - vector (1x3) address
 Output:    return vector magnitude
******************************************************************************/
float _mag_vect (const D3DVECTOR *a)
{
    float r;
    __asm {
        femms
        mov         eax,a
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
        movd        r,mm0
        femms
    }
    return r;
}


/******************************************************************************
 Routine:   _dot_vect
 Input:     a - vector (1x3) address
            b - vector (1x3) address
 Output:    return (a DOT b)
******************************************************************************/
float _dot_vect (const D3DVECTOR *a, const D3DVECTOR *b)
{
    float r;
    __asm {
        femms
        mov         eax,a
        mov         edx,b
        movq        mm0,[eax]
        movq		mm3,[edx]
        pfmul       mm0,mm3
        movd		mm2,[eax+8]
        movd        mm1,[edx+8]
        pfacc       mm0,mm0
        pfmul       mm1,mm2
        pfadd       mm0,mm1
        movd        r,mm0
        femms
    }
    return r;
}


/******************************************************************************
 Routine:   add_vect
 Input:     r - resultant vector
            a - input vector
            b - input vector
 Output:    r[i] = a[i] + b[i]
******************************************************************************/
void add_vect (D3DVECTOR *r, const D3DVECTOR *a, const D3DVECTOR *b)
{
    r->x = a->x + b->x;
    r->y = a->y + b->y;
    r->z = a->z + b->z;
}


/******************************************************************************
 Routine:   sub_vect
 Input:     r - resultant vector
            a - input vector
            b - input vector
 Output:    r[i] = a[i] - b[i]
******************************************************************************/
void sub_vect (D3DVECTOR *r, const D3DVECTOR *a, const D3DVECTOR *b)
{
    r->x = a->x - b->x;
    r->y = a->y - b->y;
    r->z = a->z - b->z;
}


/******************************************************************************
 Routine:   mult_vect
 Input:     r - resultant vector
            a - input vector
            b - input vector
 Output:    r[i] = a[i] * b[i]
******************************************************************************/
void mult_vect (D3DVECTOR *r, const D3DVECTOR *a, const D3DVECTOR *b)
{
    r->x = a->x * b->x;
    r->y = a->y * b->y;
    r->z = a->z * b->z;
}


/******************************************************************************
 Routine:   scale_vect
 Input:     r - resultant vector
            a - input vector
            f - scaling factor
 Output:    r[i] = a[i] * f
******************************************************************************/
void scale_vect (D3DVECTOR *r, const D3DVECTOR *a, float b)
{
    r->x = a->x * b;
    r->y = a->y * b;
    r->z = a->z * b;
}


/******************************************************************************
 Routine:   norm_vect
 Input:     r - resultant vector
            a - input vector
 Output:    r[i] = a[i] / mag_vect (a)
******************************************************************************/
void norm_vect(D3DVECTOR *r, const D3DVECTOR *a)
{
    float len = (float)sqrt ((a->x * a->x + a->y * a->y + a->z * a->z));
    if (len < 0.0001)
    {
        r->x = a->x;
        r->y = a->y;
        r->z = a->z;
    }
    else
    {
        len = 1.0f/len;
        r->x = a->x*len;
        r->y = a->y*len;
        r->z = a->z*len;
    }
}


/******************************************************************************
 Routine:   mag_vect
 Input:     a - input vector
 Output:    return magnitude (length) of 'a'
******************************************************************************/
float mag_vect (const D3DVECTOR *a)
{
    return (float)sqrt ((a->x * a->x + a->y * a->y + a->z * a->z));
}


/******************************************************************************
 Routine:   dot_vect
 Input:     a - input vector
            b - input vector
 Output:    return (a DOT b)
******************************************************************************/
float dot_vect (const D3DVECTOR *a, const D3DVECTOR *b)
{
    return a->x * b->x + a->y * b->y + a->z * b->z;
}


/******************************************************************************
 Routine:   cross_vect
 Input:     r - resultant vector
            a - input vector
            b - input vector
 Output:    r = cross product of 'a' and 'b'
******************************************************************************/
void cross_vect (D3DVECTOR *r, const D3DVECTOR *a, const D3DVECTOR *b)
{
    r->x = a->y * b->z - a->z * b->y;
    r->y = a->z * b->x - a->x * b->z;
    r->z = a->x * b->y - a->y * b->x;
}


/******************************************************************************
 Routine:   reflect_vect
 Input:     r - resultant vector
            v - input vector to be reflected
            n - Normal to the surface reflected off of
 Output:    r = v reflected off of n
        or, r = 2 * (n DOT v) * n - v
******************************************************************************/
void reflect_vect (D3DVECTOR *r, const D3DVECTOR *v, const D3DVECTOR *n)
{
    const float dot2 = 2.0f * (n->x * v->x + n->y * v->y + n->z * v->z);

    r->x   = dot2 * n->x - v->x;
    r->y   = dot2 * n->y - v->y;
    r->z   = dot2 * n->z - v->z;
}

// eof