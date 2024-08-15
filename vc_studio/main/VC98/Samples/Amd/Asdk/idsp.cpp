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
//; DSP.ASM
//;
//; AMD3D 3D library code: DSP primitives
//;
//; Note that all samples are 32 bit float samples, and all counts are in
//; number of SAMPLES (not bytes).
//;
//;******************************************************************************

#include <float.h>
#include <mm3dnow.h>
#include "adsp.h"

#pragma warning (disable:4799)

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
//;******************************************************************************
static const __m64i     i128    = { 0x3c000000, 0x3c000000 };
static const __m64i     i32k    = { 0x38000000, 0x38000000 };


//;******************************************************************************
//;******************************************************************************


//;******************************************************************************
//; Routine:  void _bias (int n, float *x, float av)
//; Input:    n    - samples count
//;           eax - input samples
//;           av   - real4 value
//; Output:   x[n] = x[n] - av
//; Uses:     eax, ecx, mm0 - mm2
//; Comment:  This routine handles odd sample lengths, but not very gracefully,
//;           as all accesses will likely be qword mis-aligned.  If you must have
//;           an odd number of samples, consider purposefully mis-aligning the
//;           buffer by 4 bytes to increase efficiency.  Note that the K7 cares
//;           about this issue much more than the K6 did.
//;******************************************************************************
void _bias (int n, float *x, float av)
{
    __asm
    {
        mov         eax,[x]             //; eax = destination pointer
        mov         ecx,[n]             //; ecx = number of float samples
        movd        mm1,[av]            //; mm1 = bias value
        shr         ecx,1               //; 2 at a time (only iterate 1/2 as often)
        punpckldq   mm1,mm1             //; unpack the bias value int (av | av)

        //;;; Odd number of samples
        jnc         short loop1         //; If carry is not set (from the shr above), go straight into the loop
        prefetch    [eax+4]             //; otherwise, prefetch the start of the loop
        movd        mm0,[eax]           //; process one sample
        pfsub       mm0,mm1             //; subtract the bias value
        movd        [eax],mm0           //; store the sample
        add         eax,4               //; each sample is 4 bytes - increment
        jmp         short loop1

        ALIGN       16                  //; Align the loop to a k7 cache line boundary
loop1:
        movq        mm0,[eax]           //; fetch 2 samples
        prefetch    [eax+8]             //; prefetch the next set
        pfsub       mm0,mm1             //; bias the sample
        movq        [eax],mm0           //; overwrite old sample
        add         eax,8               //; advance to next sample pair
        dec         ecx                 //; count off this pair
        jnz         short loop1
    }
}


//;******************************************************************************
//; Routine:  a_average
//; Input:    ecx - samples count
//;           eax - input samples
//; Output:   mm0 - average sample value
//; Uses:     eax, ecx, mm0 - mm2
//; Comment:  RMS is a more accurate measure of overall signal strength than
//;           _average() would give you.  _average() is a better routine to
//;           discover an overall DC bias in a signal.
//;
//;           This routine iterates through the samples, accumulating a
//;           pair of sums.  This pair is then added together at the end,
//;           and divided by the original sample count.
//;
//;           See the a_bias() comments regarding odd sample lengths and
//;           buffer alignment
//;******************************************************************************
__declspec (naked) void a_average()
{
    __asm
    {
        prefetch    [eax]           //; give the mmu a heads-up
        pxor        mm0,mm0         //; mm0 is the accumulator, start at 0
        movd        mm1,ecx         //; mm1 is 1/count,for the final division
        pi2fd       mm1,mm1         //; the input value was an integer, this needs a float
        pfrcp       mm2,mm1         //; compute the reciprocal of mm1
        shr         ecx,1           //; process 2 samples at a time
        pfrcpit1    mm1,mm2         //; finishes the reciprocal calculation
        pfrcpit2    mm1,mm2         //;  by refining the results with Newton-Raphson

        jnc         short loop1     //; Carry is set if the count was odd coming in
        movd        mm0,[eax]       //; For odd counts, process one sample out of the loop
                                    //; Since mm0 is known to be 0, just set it equal to the first sample
        add         eax,4           //; next sample
        jmp         short loop1

        ALIGN       16
loop1:
        movq        mm2,[eax]       //; fetch the sample pair
        prefetch    [eax+8]         //; prepare for the following sample
        pfadd       mm0,mm2         //; add the sample
        add         eax,8           //; advance to next pair
        dec         ecx             //; count off this pair
        jnz         short loop1     //; if ecx != 0, keep going

        pfacc       mm0,mm0         //; Combines the two accumulation pairs
        pfmul       mm0,mm1         //; multiply by 1/n (divide by n)
        ret
    }
}


//;******************************************************************************
//; Routine:  a_rms
//; Input:    ecx - samples count
//;           eax - input samples
//; Output:   mm0 - Root-Mean-Square of samples
//; Uses:     eax, ecx, mm0 - mm3
//; Comment:  RMS is a more accurate measure of overall signal strength than
//;           _average() would give you.  Average() is a better routine to
//;           discover an overall DC bias in a signal.
//;
//;           This routine operates in a similar manner to a_average,
//;           but sums squared versions of the samples, and take the
//;           square root of the average (thus "root mean squared").
//;
//;           See the a_bias() comments regarding odd sample lengths and
//;           buffer alignment
//;******************************************************************************
__declspec (naked) void a_rms()
{
    __asm
    {
        movd        mm1,ecx         //; take the original count
        prefetchw   [eax]
        pi2fd       mm1,mm1         //; convert the count into a float
        pxor        mm0,mm0         //; mm0 is an accumulator, start at 0
        pfrcp       mm2,mm1         //; find 1/count
        shr         ecx,1           //; process 2 samples per loop
        pfrcpit1    mm1,mm2         //; refine 1/count
        pfrcpit2    mm1,mm2         //; refine 1/count

        jnc         short loop1     //; Even number of samples
        movd        mm0,[eax]       //; fetch the sample
        add         eax,4           //; advance the pointer
        pfmul       mm0,mm0         //; square the sample
        jmp         short loop1     //; here, align makes a REALLY big hole - bypass it

        ALIGN       16
loop1:
        movq        mm3,[eax]       //; fetch the sample pair
        add         eax,8           //; advance the pointer
        pfmul       mm3,mm3         //; square the sample pair
        prefetch    [eax]           //; alert the mmu of our next access
        dec         ecx             //; count this pair
        pfadd       mm0,mm3         //; accumulate the new square
        jnz         short loop1     //; if ecx != 0, keep going

        pfacc       mm0,mm0         //; accumulate the two streams
        pfmul       mm0,mm1         //; compute the mean

        ; square root the result (standard pfrsqrt usage)
        pfrsqrt     mm1,mm0
        movq        mm2,mm1         //; save first 1/sqrt approximation for later
        pfmul       mm1,mm1         //; compute 1/(sqrt(x)^2) for pfrsqit1
        pfrsqit1    mm1,mm0         //; iterate for accuracy
        pfrcpit2    mm1,mm2         //; mm1 = 1 / sqrt(x)
        pfmul       mm0,mm1         //; sqrt(x) = x / sqrt(x)
        ret
    }
}


//;******************************************************************************
//; Routine:  _v_mult (int n, float *s1, float *s2, float *output)
//; Input:    n      - sample count (must be the same for all three samples)
//;           s1     - input samples
//;           s2     - input samples
//;           output - output samples
//; Output:   output = s1 * s2 for all 'n' samples
//; Uses:     eax, edx, ecx, mm0 - mm1
//; Comment:  This operation is often called "ring modulation".
//;           This routine is a C-callable wrapper for the a_v_mult routine.
//;           It preserves the appropriate registers, and (re)sets the FPU
//;           FEMMS state, so it can be used in any FPU mode.
//;******************************************************************************
void _v_mult (int n, float *s1, float *s2, float *output)
{
    __asm
    {
        femms                       //; reset FPU mode
        mov         ecx,[n]         //; load count
        mov         eax,[s1]        //; load sample set 1 ptr
        mov         edx,[s2]        //; load sample set 2 ptr
        mov         edi,[output]    //; load output sample ptr
        call        a_v_mult        //; do all the hard work
        femms                       //; reset FPU mode
    }
}


//;******************************************************************************
//; Routine:  a_v_mult
//; Input:    ecx - samples count (all samples must be equal in length)
//;           eax - input samples
//;           edx - input samples
//;           edi - output samples
//; Output:   [edi] = [eax] * [edx] for 'ecx' elements
//; Uses:     eax, edx, ecx, edi, mm0 - mm1
//; Comment:  This operation is often called "ring modulation".
//;******************************************************************************
__declspec (naked) void a_v_mult ()
{
    __asm
    {
        shr         ecx,1           //; processing 2 samples at a time

        //; Odd number of samples
        jnc         short loop1     //; if ecx was even, skip to the loop
        movd        mm0,[eax]       //; fetch sample A
        movd        mm1,[edx]       //; fetch sample B
        add         eax,4           //; advance A
        pfmul       mm0,mm1         //; Multiply samples
        add         edx,4           //; advance B
        add         edi,4           //; advance Dest
        movd        [edi-4],mm0     //; Store result (will delay due to pfmul latency)

        ALIGN       16
loop1:
        movq        mm0,[eax]       //; fetch sample pair A
        movq        mm1,[edx]       //; fetch sample pair B
        prefetchw   [edi]           //; tell mmu we intend to write [edi]
        pfmul       mm0,mm1         //; multiply the sample pairs
        add         eax,8           //; advance the A pointer
        add         edx,8           //; advance the B pointer
        add         edi,8           //; advance the Dest pointer
        dec         ecx             //; count this pair
        movq        [edi-8],mm0     //; store the result (pfmul has a 4/5 cycle latency,
                                    //;  so mm0 won't be ready for a while.  Thus, the
                                    //;  extreme measures to distance this movq from
                                    //;  the pfmul)
        jnz         short loop1     //; if ecx != 0, keep going
        nop
        ret
    }
}


//;******************************************************************************
//; Routine:  _minmax(int n, float *input, float *output)
//; Input:    n     - samples count
//;           input - input samples
//;           output- array[2] of float
//; Output:   output[0] = max, output[1] = min of samples
//; Uses:     eax, ecx, mm0 - mm2
//; Comment:  Can help determine values for signal normalization.
//;           This routine is a C-callable wrapper for the a_minmax routine.
//;           It preserves the appropriate registers, and (re)sets the FPU
//;           FEMMS state, so it can be used in any FPU mode.
//;******************************************************************************
void _minmax(int n, float *input, float *output)
{
    __asm
    {
        femms                           //; sync FPU state
        mov         ecx,[n]             //; ecx = count
        mov         eax,[input]         //; eax = input ptr
        call        a_minmax           //; do the deed
        mov         eax,[output]        //; eax = output array ptr
        movq        [eax],mm0           //; store the results
        femms                           //; sync FPU state
    }
}


//;******************************************************************************
//; Routine:  a_minmax
//; Input:    ecx - samples count
//;           eax - input samples
//; Output:   mm0 - packed (max|min) of samples
//; Uses:     eax, ecx, mm0 - mm2
//; Comment:  Can help determine values for signal normalization.
//;           As with most of these routines, the algorithm operates on
//;           a pair of results, combining them after the loop.  So at any
//;           stage of the iteration, we have two running min and max counters,
//;           stored in two mmx variables.  After the loop, the high and low
//;           parts of the streams are unpacked and combined with min/max.
//;           Finally, the results are re-packed into a qword for return
//;******************************************************************************
__declspec (naked) void a_minmax ()
{
static __m64      mxx;
static __m64      mnn;
	mxx.m64_f32[0]     = -FLT_MAX;
	mxx.m64_f32[1]     = -FLT_MAX;
	mnn.m64_f32[0]     = FLT_MAX;
	mnn.m64_f32[1]     = FLT_MAX;
    __asm
    {
        prefetch    [eax]
        movq        mm0,[mxx]       //; mm0 - min values, initialized to maxfloat|maxfloat
        shr         ecx,1           //; process 2 samples (inserted here to let the mmu breath a little)
        movq        mm1,[mnn]       //; mm1 - max values, initialized to minfloat|minfloat

        //; Correction for odd number of samples
        jnc         short loop1     //; If count was even, skip this stage
        movd        mm2,[eax]       //; only process 1 sample
        punpckldq   mm2,mm2         //; This sample counts twice - once in each "stream"
        add         eax,4           //; advance the pointer (let mm2 unpack finish)
        pfmax       mm0,mm2         //; accumulate max
        pfmin       mm1,mm2         //; accumulate min
        jmp         short loop1     //; skip the alignment padding

        ALIGN       16
loop1:
        prefetch    [eax+8]         //; inform mmu of our next load

        //; Compute pairs of min/max
        movq        mm2,[eax+0]     //; load sample pair
        add         eax,8           //; advance pointer (let mm2 load finish)
        pfmax       mm0,mm2         //; accumulate max
        dec         ecx             //; count down
        pfmin       mm1,mm2
        jnz         short loop1     //; if ecx != 0, keep going

        //; Merge the two min/max streams
        //; (these stages are interleaved to releave register dependencies a little)
        movq        mm2,mm0         //; copy max
        movq        mm3,mm1         //; copy min
        punpckhdq   mm2,mm2         //; unpack max.high
        punpckhdq   mm3,mm3         //; unpack min.high
        pfmax       mm0,mm2         //; combine max high and low
        pfmin       mm1,mm3         //; combine min high and low
        punpckldq   mm0,mm1         //; re-pack results
        ret
    }
}



//;******************************************************************************
//; Routine:  void _iir (float *pSamples, int nSamples, float *coef,
//;                      int nCoef, float *history)
//; Input:    pSamples - input sample vector
//;           nSamples - length of sample vector
//;           coef	 - pointer to filter coefficients (of length 4*n + 1)
//;	        nCoef    - number of sections in filter
//;           history  - history array (length 2*nCoef)
//;
//; Uses:     eax, edx, mm0-2, mm5, mm7
//;
//; Comment:  Perform IIR filtering sample by sample on floats
//;
//; Implements cascaded direct form II second order sections.
//; Requires arrays for history and coefficients.
//; The length (n) of the filter specifies the number of sections.
//; The size of the history array is 2*n.
//; The size of the coefficient array is 4*n + 1 because
//; the first coefficient is the overall scale factor for the filter.
//; Returns one output sample for each input sample.
//;
//; mm0 = accumulated output
//;******************************************************************************
void _iir (float *pSamples, int nSamples, float *coef, int nCoef, float *pHistory)
{
    __asm
    {
        femms
        mov         ecx, [nSamples]     //; sample count
        mov         ebx, [pSamples]     //; sample vector
        pxor        mm5, mm5            //; zero val for EZ accumulate

        ALIGN       4
sampleLoop:
        mov         eax, [coef]         //; *coefficients
        mov         edi, [pHistory]     //; *history
        movd        mm1, [ebx]          //; current sample
        movd        mm0, [eax]          //; *gain coefficient 
        mov         edx, [nCoef]        //; number of biquad sections
        pfmul       mm0, mm1            //; = input gain adjusted
        add         eax, 4              //; bump past the single gain coeff

; accumulate gain into first stage

        ALIGN       4
biquadLoop:
        movq        mm1, [edi]          //; history[0,1]
        movq        mm2, mm1            //; copy

        pfmul       mm1, [eax]          //; history[0,1] * alpha[0,1]
        movq        mm7, [edi]          //; hist[x,1] = hist[x,0] (hoisted from pack history update)
        pfadd       mm1, mm0            //; add previous stage's output (hiword is zero from movd)
        pfacc       mm1, mm5            //; = newHistory in mm1 LSW, MSW = 0 (so we can pfadd below)

; update the biquad stage history buffer

        pfmul       mm2, [eax+8]        //; history[0,1] * beta[0,1]

//; Pack history update into a quadword before writing.  Due to the K7
//; write-queue optimizations, this is more efficient since each
//; biquad must be accessed and updated for each sample.
        movq        mm3, mm1            //; mm3 is a handy scratch register
        punpckldq   mm3, mm7            //; mm3 = (hist[x,0] | newHistory)
        movq        [edi], mm3          //; hist[x,0] = newHistory
                                        //; hist[x,1] = hist[x,0]

//; old update cycle, left for reference
//;       movd        mm7, [edi]          //; hist[x,1] = hist[x,0]
//;       movd        [edi], mm1          //; hist[x,0] = newHistory
//;       movd        [edi+4], mm7

//; Note: mmx/x86 interleaved to relieve register contention on mm2
        pfadd       mm2, mm1            //; + newHist
        add         eax, 16             //; next biquad coeff section
        pfacc       mm2, mm5            //; = output in LSW with MSW = 0
        add         edi, 8              //; next history pair
        dec         edx
        movq        mm0, mm2            //; result for next stage

        jg          short biquadLoop

        movd        [ebx], mm0          //; result in-place
        add         ebx, 4              //; point to next sample

        dec         ecx
        jnz         short sampleLoop

        femms
    }
}


//;******************************************************************************
//;
//; K6-2 3DNow! block Finite Impulse Response filter
//;	
//;   void realFIR(	
//;       float   *data,          // data array to be filtered
//;       int nData,              // number of samples in data array
//;       FifoData *pFifoData);   // caller-preserved fifo structure
//; 
//;   This routine filters the caller's data with the supplied impulse
//;   response (coefficients).  The filter length must be odd.
//;   Coefficients are stored in two arrays, coeffE[] (even) and
//;   coeffO[] (odd).  The odd array has a trailing zero after the
//;   last coefficient, while the even array is preceded by a zero.
//;
//;  -------------------------------------------	
//;  | c0 c1 | c2 c3 | c4 c5 | ... | cn-1 0    |  coeffO
//;  -------------------------------------------	
//;
//;   coeff1 contains the packed coefficients offset by a zero-pad at the beginning:
//;
//;  -------------------------------------------	
//;  | 0  c0 | c1 c2 | c3 c4 | ... | cn-2 cn-1 |  coeffE
//;  -------------------------------------------
//;
//;   Register assignments
//;   mm0 = even coefficient
//;   mm1 = odd coefficient
//;   mm2 = even product accumulation
//;   mm3 = odd product accumulation
//;   mm4 = data
//;
//;   edi = data pointer
//;   esi = fifo pointer
//;   eax = loop counter
//;   ebx = ----
//;   ecx = even coefficient pointer
//;   edx = odd coefficient pointer
//;
//; FIRBAL uses a MAC kernel where one parallel MAC is computed per kernel.
//; The input data length must be a multiple of 2*N, where N is the number of
//; MAC kernels per loop.  Adjust samplesPerLoop to reflect the number of
//; MAC kernels in the loop.  Kernel address offsets are (8,16,24,...,120)
//; for the first, second, ... kernels.
//;
//;******************************************************************************

//;******************************************************************************

#define wrapCount	    0
#define fifoBase	    4
#define fifoHead	    8
#define nqTaps		    12
#define coeffE		    16
#define coeffO		    20
#define fifoSrcPtr	    24
#define fifoDstPtr	    28
#define samplesPerLoop	2
#define deltaLoop	    (8*samplesPerLoop)
void _realFIR (float *data, int nData, FifoData *fifo)
{
    //; fifo data structure offsets.  Must match sFifoData in firbal.h
    __int32 i;
    __asm
    {
        mov         edi, [data]		    //; input data array
        mov         esi, [fifo]		    //; fifo data structure pointer

        mov         eax, [nData]        //; initialize loop count
        shr         eax, 1              //; doing pairs of samples per loop
        mov         [i], eax		

	    femms

//; for each sample in data[]...
        ALIGN       4
sampleLoop:
    	mov	        ecx, [esi+coeffE]	//; even coefficient pointer
    	mov	        edx, [esi+coeffO]	//; odd coefficient pointer
//; flip sample pair for correct ordering in fifo.
//; This could be replaced by the new PFSWAP instruction
flipper:
    	movd	    mm1, [edi]		    //; current sample (t)
    	movd	    mm0, [edi+4]		//; current sample (t+1)
    	pfacc	    mm0, mm1
//; load fifo with new sample at head
    	mov	        ebx, [esi+fifoHead]
    	movq	    [ebx], mm0
//; load registers
    	movq	    mm4, mm0		    //; current sample
    	movq	    mm0, [ecx]		    //; even coefficient
    	movq	    mm1, [edx]		    //; odd coefficient
    	mov	        eax, [esi+nqTaps]	//; number of quadword taps
    	pxor	    mm2, mm2		    //; clear even acc
    	pxor	    mm3, mm3		    //; clear odd acc

	    ALIGN	    4

//; for each filter coefficient...Compute y(t) [even] and y(t+1) [odd]
filterLoop:
//;====== BEGIN KERNEL =====
    	pfmul	    mm0, mm4		    //; even
//;	    nop				                //; FREE DECODE SLOT
//;======
    	pfmul	    mm1, mm4		    //; odd
	    movq	    mm4, [ebx+8]	    //; next data
//;======
    	pfadd	    mm2, mm0		    //; accumulate even
	    movq	    mm0, [ecx+8]	    //; next even coff	
//;======
    	pfadd	    mm3, mm1		    //; accumulate odd
	    movq	    mm1, [edx+8]	    //; next odd coff
//;====== END KERNEL =====


//;====== BEGIN KERNEL =====
    	pfmul	    mm0, mm4		    //; even
//;	    nop				                //; FREE DECODE SLOT
//;======
    	pfmul	    mm1, mm4		    //; odd
	    movq	    mm4, [ebx+16]	    //; next data
//;======
    	pfadd	    mm2, mm0		    //; accumulate even
	    movq	    mm0, [ecx+16]	    //; next even coff	
//;======
    	pfadd	    mm3, mm1		    //; accumulate odd
	    movq	    mm1, [edx+16]	    //; next odd coff
//;====== END KERNEL =====

	    add	        ecx, deltaLoop
	    add	        edx, deltaLoop
	    add	        ebx, deltaLoop
	    sub	        eax, samplesPerLoop	//;  loop stride
	    jg	        filterLoop


//; Accumulate result and store in-place in input data array
//; mmreg[0..31] = even sample filtered output
//; mmreg[32..63] = odd sample filtered output
accumulate:
	    pfacc	    mm2, mm3		//; accumulate odd/even products
	    movq	    [edi], mm2		//; store filtered result in-place
	    add 	    edi, 8			//; bump to next input sample pair

//; test for fifo wrap; if so, blockmove the fifo to top of fifo memory area.
	    mov	        ebx, [esi+fifoHead]	//; dec fifo head for new sample 
	    sub	        ebx, 8			    //; one quad per complex data		; 
	    cmp	        ebx, [esi+fifoBase]	//; current head in bounds?
	    jge	        noMove
moveFifo:
	    mov	        eax, [esi+fifoSrcPtr]
	    mov	        edx, [esi+fifoDstPtr]
	    mov	        ecx, [esi+nqTaps]	//; blockmove count, quadwords
	    sub	        ecx,1				//; don't move first or last element
        ALIGN       4
blockMove:
	    movq	    mm0, [eax]		    //; imag components
	    sub         eax, 8
	    movq	    [edx], mm0
	    sub         edx, 8
        dec         ecx
        jnz         blockMove
        
	    mov	        ebx, [esi+fifoBase]	//; reset fifo head
	    add	        ebx, [esi+wrapCount]
	    mov	        [esi+fifoHead], ebx	//; restore fifoHead to initial position at top
	    dec	        [i]
	    jg	        sampleLoop
noMove:
    	mov	        [esi+fifoHead], ebx	//; to update fifoHead
    	dec	        [i]
    	jg	        sampleLoop
	
	    femms
    }
}
#undef wrapCount
#undef fifoBase
#undef fifoHead
#undef nqTaps
#undef coeffE
#undef coeffO
#undef fifoSrcPtr
#undef fifoDstPtr
#undef samplesPerLoop
#undef deltaLoop


//;******************************************************************************
//; FIR.ASM: K6-2 3DNow! block Finite Impulse Response filter
//;	
//;   void _complexFIR(
//;       COMPLEX *data,          // data array to be filtered
//;       int nData,              // number of samples in data array
//;       CFifoData *pFifoData);  // caller-preserved fifo structure
//; 
//;   This routine filters the caller's data with the supplied complex impulse
//;   response (coefficients).  The filter length must be odd.
//;	
//;   COMPLEX data is two floats (x.r, x.i).
//;   Data is stored in fifo as (x.r, -x.i) | (x.i, x.r) to facilitate computation.
//;
//;   The CFifoData structure must be initialized with the _initFifoData()
//;   function.  See the DSP Library documentation for details.
//;
//;******************************************************************************

#define fifoBase      0
#define fifoHead      4
#define nTaps         8
#define coeff         12
#define fifoIx        16
#define fifoTop       20
void _complexFIR (COMPLEX *data, int nData, CFifoData *fifo)
{
    __int32 i;
    __asm
    {
        mov         edi, [data]		    //; input data array
        mov         esi, [fifo]		    //; fifo data structure pointer
        mov         eax, [nData]        //; initialize loop count
        mov         [i], eax

    	femms

//; for each sample in data[]...put sample in fifo
//; using dual-quadword format (-x.q | x.i) (x.i | x.q)
//; to facilitate complex math on K6-x.
sampleLoop:
        mov	        eax, [esi+fifoIx]	//; eax => fifo index
    	mov	        ebx, [esi+fifoHead]	//; ebx => head
    	dec	        eax
    	jge	        noWrap
    	mov	        eax, [esi+nTaps]	//; reset fifo index
    	dec	        eax
    	mov	        ebx, [esi+fifoTop]	//; reset fifoHead
    	mov	        [esi+fifoHead], ebx

noWrap:
    	mov	        [esi+fifoIx], eax
    	mov	        ecx, [esi+coeff]	//; complex coefficient pointer
    	movd	    mm1, [edi+4]		//; x.q
    	movd	    mm0, [edi]		    //; x.i
    	mov	        ebx, [esi+fifoHead]
    	pxor	    mm5, mm5		    //; negate x.q
    	pfsub	    mm5, mm1		    //; -x.q
    	movq	    mm2, mm0
    	pfacc	    mm0, mm5		    //; mm0 = -x.q, x.i
    	pfacc	    mm1, mm2		    //; mm1 = x.i, x.q
    	movq	    [ebx], mm0
    	movq	    [ebx+8], mm1


//;  load registers
    	movq    	mm6, [ecx]		    //; coefficient (real,imag) array
    	mov	        edx, [esi+nTaps]	//; less number of taps, total
    	sub	        edx, eax		    //; = loop1 count
    	pxor    	mm5, mm5		    //; clear accumulator
    	movq    	mm7, mm6		    //; dup coeff to speed kernel

	    ALIGN	    4

//; for each filter coefficient...Compute complex output

loop1:
//;====== BEGIN KERNEL =====
//; mm7 - complex coefficient
//; mm0 - x.i, -x.q (real)
//; mm4 - x.q, x.i  (imag)

    	pfmul	    mm6, mm0		    //; real terms
    	movq	    mm0, [ebx+16]		//; next real terms
    	pfmul	    mm7, mm1		    //; imag terms
    	movq	    mm1, [ebx+24];		//; next imag terms
    	pfacc	    mm6, mm7		    //; sum imag and accumulate yielding x.i|x.q
    	pfadd	    mm5, mm6		    //; accumulate
    	movq	    mm6, [ecx+8]		//; next coefficient	
//;====== END KERNEL =====

    	add	        ebx, 16			    //; => next complex data pair (double qword format)
    	add	        ecx, 8			    //; => next coefficient
    	movq	    mm7, mm6		    //; dup coeff for kernel fetch overlap
    	dec	        edx
    	jg	        loop1

    	mov	        ebx, [esi+fifoBase]	//; point to base of fifo for second loop
    	movq	    mm0, [ebx]		    //; next real terms	
    	movq	    mm1, [ebx+8]		//; next imag terms	
	    jmp	        loop2Test

//; second fir loop

	ALIGN	4
loop2:
//;====== BEGIN KERNEL =====
//; mm7 - complex coefficient
//; mm0 - x.i, -x.q (real)
//; mm4 - x.q, x.i  (imag)

    	pfmul	    mm6, mm0		    //; real terms
    	movq	    mm0, [ebx+16]	    //; next real terms
    	pfmul	    mm7, mm1		    //; imag terms
    	movq	    mm1, [ebx+24];	    //; next imag terms
    	pfacc	    mm6, mm7		    //; sum imag and accumulate yielding x.i|x.q
    	pfadd	    mm5, mm6		    //; accumulate
    	movq	    mm6, [ecx+8]	    //; next coefficient	
//;====== END KERNEL =====

    	add	        ebx, 16			    //; => next complex data pair (double qword format)
    	add	        ecx, 8			    //; => next coefficient
    	movq	    mm7, mm6		    //; dup coeff for kernel fetch overlap
loop2Test:
    	dec	        eax
    	jge	        loop2

    	movq	    [edi], mm5		    //; store filtered result in-place
    	Add	        edi, 8			    //; bump to next complex input
    	mov	        eax, [esi+fifoHead]
    	sub	        eax, 16
    	mov         [esi+fifoHead], eax
	    dec	        [i]
    	jg	        sampleLoop

    	femms
    }
}

#undef fifoBase
#undef fifoHead
#undef nTaps
#undef coeff
#undef fifoIx
#undef fifoTop


//;******************************************************************************
//; Routine:  void _cvt_8bit (int n, unsigned char *input, float *output);
//; Input:    n       - number of samples to convert
//;           input   - 8 bit unsigned integer samples, 'n' elements long
//;           output  - 32 bit float samples, 'n' elements long
//; Output:   32 bit float samples stored in the output
//; Uses:     eax, ecx, edx, mm0 - mm2
//; Comment:  This routine is unrolled to process 4 samples per "loop".
//;           8 bit samples are biased by 128.  That is, the conversion is
//;           fsample = float (isample - 128) / 128.0
//;           or, equivalently
//;           fsample = float (isample) / 128.0 - 1.0f
//;           This gives a range of nearly -1.0 - 1.0
//;******************************************************************************
void _cvt_8bit (int n, unsigned char *input, float *output)
{
static __m64      ones;
	ones.m64_f32[0] = 1.0f;
	ones.m64_f32[1] = 1.0f;
    __asm
    {
        prefetch    DWORD PTR[ones]
        femms
        mov         ecx,[n]
        mov         eax,[input]
        mov         edx,[output]

        pxor        mm0,mm0         //; a handy source of zero bytes
        movq        mm3,[i128]      //; mm3 = reciprocal 8 bit bias value
        shr         ecx,2           //; For now, assume dword-divisible count
        movq        mm4,[ones]      //; mm4 = (1.0 | 1.0)
        cmp         ecx,0           //; if we seem to have no samples, we may simply have < 4 samples
        je          short leftovers //; if so, process them
        jmp         short loop1     //; jump around alignment padding (too many bytes to ignore)

        ALIGN       16
loop1:
        prefetchw   [edx]
        movd        mm1,[eax]
        add         eax,4       //; four samples converted
        punpcklbw   mm1,mm0     //; mm1 = d:c:b:a
        movq        mm2,mm1     //; mm2 = d:c:b:a
        punpcklwd   mm1,mm0     //; mm1 = b:a
        punpckhdq   mm2,mm2     //; mm2 = d:c:d:c
        pi2fd       mm1,mm1     //; mm1 = b:a
        punpcklwd   mm2,mm0     //; mm2 = d:c
        pfmul       mm1,mm3     //; mm1 /= 128
        pi2fd       mm2,mm2
        pfsub       mm1,mm4     //; mm1 -= 1.0 (re-centers values around 0)
        pfmul       mm2,mm3
        movq        [edx],mm1   //; store b:a samples
        pfsub       mm2,mm4
        movq        [edx+8],mm2 //; store d:c samples
        add         edx,16
        dec         ecx
        jnz         short loop1

        ALIGN       16
leftovers:
        //; Handle any unaligned trailing samples
        //; (done at the end so as to not misalign the `movd's above)
        mov         ecx,[n]
        shr         ecx,1
        jnc         short not_odd

        //; Handle odd case
        movzx       ebx,BYTE PTR [eax]
        inc         eax
        movd        mm1,ebx
        punpcklbw   mm1,mm0     //; mm1 = d:c:b:a
        punpcklwd   mm1,mm0     //; mm1 = b:a
        pi2fd       mm1,mm1     //; mm1 = b:a
        pfmul       mm1,mm3     //; mm1 /= 128
        pfsub       mm1,mm4     //; mm1 -= 1.0 (re-centers value around 0)
        movd        [edx],mm1   //; store single sample
        add         edx,4       //; advance ptr

not_odd:
        shr         ecx,1
        jnc         short the_end

        //; Handle /2 case
        movzx       ebx,WORD PTR [eax]
        add         eax,2
        movd        mm1,ebx
        punpcklbw   mm1,mm0     //; mm1 = d:c:b:a
        punpcklwd   mm1,mm0     //; mm1 = b:a
        pi2fd       mm1,mm1     //; mm1 = b:a
        pfmul       mm1,mm3     //; mm1 /= 128
        pfsub       mm1,mm4     //; mm1 -= 1.0 (re-centers value around 0)
        movq        [edx],mm1   //; store sample pair

the_end:
        femms
    }
}


//;******************************************************************************
//; Routine:  void _cvt_16bit (int n, unsigned char *input, float *output);
//; Input:    n       - number of samples to convert
//;           input   - 16 bit signed integer samples, 'n' elements long
//;           output  - 32 bit float sample buffer, 'n' elements long
//; Output:   32 bit float samples, stored in output
//; Uses:     eax, ecx, edx, mm0 - mm2
//; Comment:  This routine is unrolled to process 4 samples per "loop"
//;           16 bit samples are signed, so the conversion is
//;           fsample = float (isample) / 32768.0
//;           This gives a range of nearly -1.0 - 1.0
//;******************************************************************************
void _cvt_16bit (int n, unsigned char *input, float *output)
{
    __asm
    {
        femms
        mov         ecx,[n]
        mov         eax,[input]
        mov         edx,[output]

        pxor        mm0,mm0     //; a handy source of zero bytes
        shr         ecx,2       //; For now, assume dword-divisible count
        movq        mm3,[i32k]
        cmp         ecx,0
        je          short leftovers
        jmp         short loop1 //; jump around alignment padding (too many bytes to ignore)

        ALIGN       16
loop1:
        prefetchw   [edx]
        pxor        mm0,mm0
        movq        mm1,[eax]
        add         eax,8       //; four samples converted
        pcmpgtw     mm0,mm1
        movq        mm2,mm1     //; mm2 = d:c:b:a
        punpcklwd   mm1,mm0     //; mm1 = b:a
        pxor        mm0,mm0
        punpckhdq   mm2,mm2     //; mm2 = d:c:d:c
        pi2fd       mm1,mm1     //; mm1 = b:a
        pcmpgtw     mm0,mm2
        punpcklwd   mm2,mm0     //; mm2 = d:c
        pfmul       mm1,mm3
        pi2fd       mm2,mm2
        movq        [edx],mm1
        pfmul       mm2,mm3
        movq        [edx+8],mm2
        add         edx,16
        dec         ecx
        jnz         short loop1

        ALIGN       16
leftovers:
        //; Handle any unaligned trailing samples
        //; (done at the end so as to not misalign the `movd's above)
        mov         ecx,[n]
        pxor        mm0,mm0
        shr         ecx,1
        jnc         short not_odd

        //; Handle odd case
        movsx       ebx,WORD PTR [eax]
        add         eax,2
        movd        mm1,ebx
        pcmpgtw     mm0,mm1
        punpcklwd   mm1,mm0     //; mm1 = ?:a
        pi2fd       mm1,mm1     //; mm1 = ?:a
        pxor        mm0,mm0
        pfmul       mm1,mm3
        movd        [edx],mm1
        add         edx,4

not_odd:
        shr         ecx,1
        jnc         short the_end

        //; Handle /2 case
        movd        mm1,[eax]
        add         eax,4
        pcmpgtw     mm0,mm1
        punpcklwd   mm1,mm0     //; mm1 = b:a
        pi2fd       mm1,mm1     //; mm1 = b:a
        pfmul       mm1,mm3
        movq        [edx],mm1
        add         edx,8

the_end:
        femms
    }
}

