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

 MATH_LIB.C

 AMD3D 3D library code: Math routines (wrap fastcall ASM routines in math.asm)

******************************************************************************/

#include "amath.h"

#define HALFH   0x3f000000
#define PINH    0x7f800000
#define MASKSMH 0x807fffff

static float ones[2] = { 1.0f, 1.0f };
static long  sh_masks[2] = {  0x80000000u, 0x80000000u };

/******************************************************************************
 Routine:   _atan
 Input:     x - float
 Output:    return (float)atan (x)
 Comment:   Identical to the a_atan() call, but takes the parameters from
            the stack, rather than from the mmx registers.
            In this case, implemented as a simple wrapper.
            See a_atan code for more details.
******************************************************************************/
float _atan (float x)
{
    float res=0.0f;
    __asm
    {
        FEMMS
        movd    mm0,x       // a_atan takes 'x' in mm0
        call    a_atan
        movd    res,mm0     // a_atan returns atan(x) in mm0
        FEMMS
    }
    return res;
}


/******************************************************************************
 Routine:   _atan2
 Input:     x - float
 Output:    return (float)atan2 (x,y)
 Comment:   Identical to the a_atan2() call, but takes the parameters from
            the stack, rather than from the mmx registers.
            In this case, implemented as a simple wrapper.
            See a_atan2 code for more details.
******************************************************************************/
float _atan2 (float x, float y)
{
    float res=0.0f;
    __asm
    {
        FEMMS
        movd    mm0,x       // a_atan2 takes 'x' in mm0
        movd    mm1,y       // a_atan2 takes 'y' in mm1
        call    a_atan2
        movd    res,mm0     // result in mm0
        FEMMS
    }
    return res;
}


/******************************************************************************
 Routine:   _acos
 Input:     x - float (-1.0 -> 1.0)
 Output:    return (float)acos (x)
 Comment:   Identical to the a_acos() call, but takes the parameters from
            the stack, rather than from the mmx registers.
            In this case, implemented as a simple wrapper.
            See a_acos code for more details.
******************************************************************************/
float _acos(float x)
{
    float fval;
        
    __asm
    {
        FEMMS
        movd    mm0,x
        call    a_acos
        movd    fval,mm0     // result in mm0
        FEMMS
    }
    return fval;
}



/******************************************************************************
 Routine:   _asin
 Input:     x - float (-1.0 -> 1.0)
 Output:    return (float)asin (x)
 Comment:   Identical to the a_asin() call, but takes the parameters from
            the stack, rather than from the mmx registers.
            In this case, implemented as a simple wrapper.
            See a_asin code for more details.
******************************************************************************/
float _asin(float x)
{
    float fval;
        
    __asm
    {
        FEMMS
        movd    mm0,x
        call    a_asin
        movd    fval,mm0    // result in mm0
        FEMMS
    }
    return fval;
}

    
/******************************************************************************
 Routine:   _log
 Input:     x - float
 Output:    return (float)log (x)
 Comment:   Identical to the a_log() call, but takes the parameters from
            the stack, rather than from the mmx registers.
            In this case, implemented as a simple wrapper.
            See a_log code for more details.
******************************************************************************/
float _log(float x)
{
    float fval;
        
    __asm
    {
        FEMMS
        movd    mm0,x
        call    a_log
        movd    fval,mm0    // result in mm0
        FEMMS
    }
    return fval;
}


/******************************************************************************
 Routine:   _log10
 Input:     x - float
 Output:    return (float)log10 (x)
 Comment:   Identical to the a_log10() call, but takes the parameters from
            the stack, rather than from the mmx registers.
            In this case, implemented as a simple wrapper.
            See a_log10 code for more details.
******************************************************************************/
float _log10(float x)
{
    float fval;
        
    __asm
    {
        FEMMS
        movd    mm0,x
        call    a_log10
        movd    fval,mm0    // result in mm0
        FEMMS
    }
    return fval;
}


/******************************************************************************
 Routine:   _exp
 Input:     x - float
 Output:    return (float)exp(x)
 Comment:   Identical to the a_exp() call, but takes the parameters from
            the stack, rather than from the mmx registers.
            In this case, implemented as a simple wrapper.
            See a_exp code for more details.
******************************************************************************/
float _exp(float x)
{
    float fval;
        
    __asm
    {
        FEMMS
        movd    mm0,x
        call    a_exp
        movd    fval,mm0    // result in mm0
        FEMMS
    }
    return fval;
}   


/******************************************************************************
 Routine:   _sqrt
 Input:     x - float
 Output:    return (float)sqrt(x)
 Comment:   Uses the reciprical square root opcodes, as per the 3DNow!
            guide.  Performs the Newton-Raphson iteration steps for
            best precision.
******************************************************************************/
float _sqrt(float x)
{
    float fval;
        
    __asm
    {
        FEMMS
        movd        mm0,x
        pfrsqrt     mm1,mm0     // mm1 = 1/sqrt(x), first pass
        movq        mm2,mm1     // save imprecise 1/sqrt(x) for Newton-Raphson
        pfmul       mm1,mm1     // compute a new 'x' by squaring sqrt(x)
        pfrsqit1    mm1,mm0     // N-R 1 (uses original 'x' and approximated 'x')
        pfrcpit2    mm1,mm2     // N-R 2 (uses 1/sqrt(x) and result of N-R 1)
                                // mm1 = 1/sqrt(x), iteratively refined
        pfmul       mm0,mm1     // sqrt(x) = x/sqrt(x) (mathematically true)
        movd        fval,mm0    // result in mm0
        FEMMS
    }
    return fval;
}


/******************************************************************************
 Routine:   _fabs
 Input:     x - float
 Output:    return (float)fabs(x)
 Comment:   Included for completeness, probably no faster than the
            built-in version.
******************************************************************************/
float _fabs (float x)
{
    float fval;
        
    __asm
    {
        mov     eax,x           // starting with 
        and     eax,0x7fffffff  // And out the sign bit
        mov     fval,eax        // result in mm0
    }
    return fval;
}
        

/******************************************************************************
 Routine:   _ceil
 Input:     x - float
 Output:    return (float)ceil (x)
 Comment:   Identical to the a_ceil() call, but takes the parameters from
            the stack, rather than from the mmx registers.
******************************************************************************/
float _ceil(float x)
{
    float fval;
        
    __asm
    {
        FEMMS
        movd        mm0,[x]
        pxor        mm4,mm4         // mm4 = 0:0
        movq        mm3,[sh_masks]
        pf2id       mm2,mm0         // I = mm2
        movq        mm1,[ones]      // mm1 = 1:1
        pi2fd       mm2,mm2
        pand        mm3,mm0         // mm3 = sign bit
        pfsub       mm0,mm2         // F   = mm0
        pfcmpgt     mm0,mm4         // mm0 = F > 0.0
        pand        mm0,mm1         // mm0 = (F > 0) ? 1: 0
        por         mm2,mm3         // re-assert the sign bit
        por         mm0,mm3         // add sign bit
        pfadd       mm0,mm2
        movd        [fval],mm0
        FEMMS
    }
    return fval;
} 


/******************************************************************************
 Routine:   _floor
 Input:     x - float
 Output:    return (float)floor(x)
 Comment:   Identical to the a_floor() call, but takes the parameters from
            the stack, rather than from the mmx registers.
******************************************************************************/
float _floor(float x)
{
    float fval;
        
    __asm
    {
        FEMMS
        movd        mm0,x
        pf2id       mm2,mm0         // I = mm2
        pxor        mm1,mm1         // mm1 = 0|0
        pi2fd       mm2,mm2
        movq        mm3,mm2         // I = mm3
        pfsubr      mm2,mm0         // F = mm0
        pfcmpgt     mm1,mm2         // is F > I? (result becomes a bit mask)
        movq        mm0,[ones]      // mm0 = 1|1
        pand        mm0,mm1         // mm0 = F > I ? 1 : 0
        pfsubr      mm0,mm3         // mm0 = I - (F > I ? 1 : 0)
        movd        fval,mm0
        FEMMS
    }
    return fval;
}       
        

/******************************************************************************
 Routine:   _frexp
 Input:     x - float
            y - int address
 Output:    return mantissa, store exponent in *y
 Comment:   Identical to the a_frexp() call, but takes the parameters from
            the stack, rather than from the mmx registers.
******************************************************************************/
float _frexp(float x, int *y)
{
    float res;
        
    __asm
    {
        mov      eax,[x]            // get float argument x
        mov      edx,eax            // x 
        and      eax,MASKSMH        // sign and mantissa of x
        and      edx,PINH           // masked biased expo of x
        mov      ecx,edx            // masked biased expo of x
        or       eax,HALFH          // map mantissa into [0.5,1)
        neg      ecx                // bias_expo == 0 ?
        sbb      ecx,ecx            // mask = (bias_expo==0) ? 0 : 0xfffffff
        shr      edx,23             // extract biased expo
        sub      edx,126            // expo = biased expo - BIAS + 1
        and      eax,ecx            // flush mant to 0 if biased expo was 0
        and      edx,ecx            // flush expo to 0 if biased expo was 0
        mov      ecx,[y]            // get pointer to y
        mov      res,eax            // store mantissa
        mov      [ecx],edx
    }
    return res;
}


/******************************************************************************
 Routine:   _ldexp
 Input:     x   - float
            exp - int
 Output:    return float with mantissa 'x' and exponent 'exp'
 Comment:   Inline, integer-only version of ldexp.  This version is potentially
            faster in mixed-mode code, because there is no need to FEMMS.  If
            you already have the parameters in mmx registers, then use a_ldexp()
            instead.
******************************************************************************/
float _ldexp(float x, int exp)
{
    float   res;

    __asm
    {
        mov         edx,[x]
        and         edx,0x7fffffff  // mask out the sign bit
        shr         edx,23          // shift out the mantissa (leave exponent)
        add         edx,[exp]       // add 'exp' to the exponent
        mov         eax,[x]         // Get a fresh copy of 'x'
        test        edx,0xffffff00  // test for overflow mask
        jnz         short exception // if overflow, handle the exception
        and         eax,0x807fffff  // mask out the exponent (leaving sign & mantissa)
        shl         edx,23          // shift new exponent back into place
        or          eax,edx         // combine mantissa and exponent
        mov         [res],eax
    }
    return res;

    // Error handling code - executed out of band, since most cases
    // will not be exceptional.
    __asm
    {
        ALIGN       4
exception:
        and         edx,0x80000000  // underflow? (check sign bit)
        xor         eax,eax         // result = 0
        jnz         short ending    // if underflow, we're done

        mov         eax,[x]         // restore original value
        and         eax,0x80000000  // keep its sign bit         
        or          eax,0x7f800000  // mm0 = (x < 0) ? -Inf: +Inf
ending:
        mov         [res],eax       // store the result
    }
    return res;
}


/******************************************************************************
 Routine:   _modf
 Input:     x    - float
            iptr - float address
 Output:    return integer portion of 'x', *iptr contains fraction
 Comment:   Identical to the a_modf() call, but takes the parameters from
            the stack, rather than from the mmx registers
******************************************************************************/
float _modf(float x, float *iptr)
{
    float res[2];

    // Setup registers for a_modf call, and return results
    __asm
    {
        FEMMS
        movd        mm0,x       // mm0(F) = x
        pf2id       mm1,mm0     // mm1(I) = int(x)
        pi2fd       mm1,mm1     // mm1(I) = float(int(x))
        pfsub       mm0,mm1     // mm0 = (I+F) - I
        punpckldq   mm0,mm1     // mm0 = res:iptr
        movq        res,mm0     // store mm0 for easy unpacking
        FEMMS
    }
    *iptr = res[1];
    return res[0];
}


/******************************************************************************
 Routine:   _fmod
 Input:     x - float
            y - float modulus
 Output:    return (float)fmod (x, y)
 Comment:   Identical to the a_fmod() call, but takes the parameters from
            the stack, rather than from the mmx registers.
            In this case, implemented as a simple wrapper.
            See a_fmod code for more details.
******************************************************************************/
float _fmod(float x, float y)
{
    float res;
    
    // Setup registers for a_fmod call, and return results
    __asm {
        FEMMS
        movd        mm0,x
        movd        mm1,y
        call        a_fmod
        movd        res,mm0
        FEMMS
    }

    return res;
}   


/******************************************************************************
 Routine:   _sincos
 Input:     x - angle in radians
            v - float[2] array
 Output:    v[0] = cos (angle), v[1] = sin (angle)
 Comment:   Identical to the a_sincos() call, but takes the parameters from
            the stack, and returns the result in a 2 element float array,
            rather than using the mmx registers.
            In this case, implemented as a simple wrapper.
            See a_sincos code for more details.
******************************************************************************/
void _sincos(float x, float *v)
{
    // Setup registers for a_sincos call, and return results
    // Note that a_sincos trashes ebx,esi, so these must be preserved
    __asm
    {
        FEMMS
        movd    mm0,x
        push    ebx
        push    esi
        call    a_sincos
        mov     eax,v
        pop     esi
        pop     ebx
        movq    qword ptr [eax],mm0
        FEMMS
    }
}


/******************************************************************************
 Routine:   _sin
 Input:     x - angle in radians
 Output:    return (float)sin(x)
 Comment:   Identical to the a_sin() call, but takes the parameters from
            the stack, rather than from the mmx registers.
            In this case, implemented as a simple wrapper.
            See a_sin code for more details.
******************************************************************************/
float _sin(float x)
{
    float fval;
        
    // Setup registers for a_sin call, and return results
    // Note that a_sin trashes ebx,esi, so these must be preserved
    __asm
    {
        FEMMS
        movd    mm0,x
        push    ebx
        push    esi
        call    a_sin
        pop     esi
        pop     ebx
        movd    fval,mm0
        FEMMS
    }
    return fval;
}


/******************************************************************************
 Routine:   _cos
 Input:     x - angle in radians
 Output:    return (float)cos (x)
 Comment:   Identical to the a_cos() call, but takes the parameters from
            the stack, rather than from the mmx registers.
            In this case, implemented as a simple wrapper.
            See a_cos code for more details.
******************************************************************************/
float _cos(float x)
{
    float fval;
        
    // Setup registers for a_cos call, and return results
    // Note that a_cos trashes ebx,esi, so these must be preserved
    __asm
    {
        FEMMS
        movd    mm0,x
        push    ebx
        push    esi
        call    a_cos
        pop     esi
        pop     ebx
        movd    fval,mm0
        FEMMS
    }
    return fval;
}


/******************************************************************************
 Routine:   _tan
 Input:     x - angle in radians
 Output:    return (float)tan (x)
 Comment:   Identical to the a_tan() call, but takes the parameters from
            the stack, rather than from the mmx registers.
            In this case, implemented as a simple wrapper.
            See a_tan code for more details.
******************************************************************************/
float _tan(float x)
{
    float fval;
        
    // Setup registers for a_tan call, and return results
    // Note that a_tan trashes ebx,esi, so these must be preserved
    __asm
    {
        FEMMS
        movd    mm0,x
        push    ebx
        push    esi
        call    a_tan
        pop     esi
        pop     ebx
        movd    fval,mm0
        FEMMS
    }
    return fval;
}


/******************************************************************************
 Routine:   _pow
 Input:     x - base
            y - exponent
 Output:    return x ^ y
 Comment:   Identical to the a_pow() call, but takes the parameters from
            the stack, rather than from the mmx registers.
            In this case, implemented as a simple wrapper.
            See a_pow code for more details.
******************************************************************************/
float _pow (float x, float y)
{
    float fval;
        
    // Setup registers for a_pow call, and return results
    __asm
    {
        FEMMS
        movd    mm0,x
        movd    mm1,y
        call    a_pow
        movd    fval,mm0
        FEMMS
    }
    return fval;
}


// eof