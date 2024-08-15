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
#include "amath.h"
#include "adsp.h"

// It's OK not to use FEMMS in these routines
#pragma warning (disable:4799)
#pragma warning (disable:4730)
#pragma warning (disable:4700)


//;******************************************************************************
//;******************************************************************************
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


static const __m64i     i128    = { 0x3c000000, 0x3c000000 };
static const __m64i     i32k    = { 0x38000000, 0x38000000 };
static __m64      ones;
static __m64      mxx;
static __m64      mnn;

void init_idsp()
{
	ones.m64_f32[0] = 1.0f;
	ones.m64_f32[1] = 1.0f;
	mxx.m64_f32[0] = -FLT_MAX;
	mxx.m64_f32[1] = -FLT_MAX;
	mnn.m64_f32[0] = FLT_MAX;
	mnn.m64_f32[1] = FLT_MAX;
}

//;******************************************************************************
//;******************************************************************************
#define _m_from_floats(f) (*(__m64 *)(f))
#define _m_from_float(f)  _m_from_int (*(int *)&(f))


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
    _m_femms();
    __m64 bias;
    bias.m64_f32[0] = av;
    bias.m64_f32[1] = av;

    // Process the samples in pairs
    for (int i = n >> 1; i != 0; --i)
    {
        float *y = x + 2;
		_m_prefetch (y);
        *(__m64 *)x = _m_pfsub (_m_from_floats (x), bias);
        x = y;
    }

    // If an odd number, then process the odd element
    if (n & 1)
    {
        *x = _m_pfsub (_m_from_float (*x), bias).m64_f32[0];
    }
    _m_femms();
}


//;******************************************************************************
//; Routine:  _m_average
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
__m64 _m_average (int count, __m64 *samples)
{
    __m64 icount, accum, tmp;

    // Start the accumulator off empty
    accum = _m_pxor (accum, accum);

    // Compute icount = 1.0f / float (count)
    icount = _m_pi2fd (_m_from_int (count));
    tmp    = _m_pfrcp (icount);
    icount = _m_pfrcpit2 (_m_pfrcpit1 (icount, tmp), tmp);

    // Process sample pairs
    for (int i = count >> 1; i != 0; --i)
    {
		__m64* s = samples + 1;
        _m_prefetch (s);
        accum = _m_pfadd (accum, *samples);
        samples = s;
    }

    // Process the odd numbered element, if present
    if (count & 1)
    {
        accum = _m_pfadd (accum, _m_from_float (*(float *)samples));
    }

    accum = _m_pfacc (accum, accum);
    return _m_pfmul (accum, icount);
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
__m64 _m_rms (int count, __m64 *samples)
{
    __m64 icount, accum, tmp;

    // Start the accumulator off empty
    accum  = _m_pxor (accum, accum);

    // Compute icount = 1.0f / float (count)
    icount = _m_pi2fd (_m_from_int (count));
    tmp    = _m_pfrcp (icount);
    icount = _m_pfrcpit2 (_m_pfrcpit1 (icount, tmp), tmp);

    // Process sample pairs
    for (int i = count >> 1; i != 0; --i)
    {
		__m64* s = samples + 1;
        _m_prefetch (s);
        tmp   = *samples;
        accum = _m_pfadd (accum, _m_pfmul (tmp, tmp));
        samples = s;
    }

    // Process the odd numbered element, if present
    if (count & 1)
    {
        tmp   = _m_from_float (*(float *)samples);
        accum = _m_pfadd (accum, _m_pfmul (tmp, tmp));
    }

    // Accumulate the two streams
    accum = _m_pfmul (_m_pfacc (accum, accum), icount);

    // Take the square root
    tmp = _m_pfrsqrt (accum);
    tmp = _m_pfrcpit2 (_m_pfrsqit1 (_m_pfmul (tmp, tmp),
                                    accum),
                       tmp);
    return _m_pfmul (accum, tmp);
}



//;******************************************************************************
//; Routine:  _m_v_mult
//; Input:    ecx - samples count (all samples must be equal in length)
//;           eax - input samples
//;           edx - input samples
//;           edi - output samples
//; Output:   [edi] = [eax] * [edx] for 'ecx' elements
//; Uses:     eax, edx, ecx, edi, mm0 - mm1
//; Comment:  This operation is often called "ring modulation".
//;******************************************************************************
void _m_v_mult (int count, const __m64 *a, const __m64 *b, __m64 *out)
{
    for (int i = count >> 1; i != 0; --i)
    {
        *out = _m_pfmul (*a, *b);
        ++a;
        ++b;
        ++out;
    }

    if (count & 1)
    {
        *(float *)out = _m_pfmul (_m_from_float (*(float *)a),
                                  _m_from_float (*(float *)b)).m64_f32[0];
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
    _m_femms();
    _m_v_mult (n, (const __m64 *)s1,  (const __m64 *)s2, (__m64 *)output);
    _m_femms();
}


//;******************************************************************************
//; Routine:  _m_minmax
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
__m64 _m_minmax (int count, const __m64 *samples)
{
    __m64 max, min, tmp;

    max = mxx;
    min = mnn;

    for (int i = count >> 1; i != 0; --i)
    {
        tmp = *samples;
        max = _m_pfmax (max, tmp);
        min = _m_pfmin (min, tmp);
        ++samples;
    }

    if (count & 1)
    {
        tmp = _m_from_int (*(int *)samples);
        max = _m_pfmax (max, tmp);
        min = _m_pfmin (min, tmp);
    }

    //; Merge the two min/max streams
    max = _m_pfmax (max, _m_punpckhdq (max, max));
    min = _m_pfmin (min, _m_punpckhdq (min, min));
    return _m_punpckldq (max, min);
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
    _m_femms();
    *(__m64 *)output = _m_minmax (n, (const __m64 *)input);
    _m_femms();
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
void _iir (float *pSamples, int nSamples, float *pCoef, int nSections, float *pHistory)
{
    _m_femms();

    __m64 r0, r1, r2, r3, r5, r7;

    float *ebx = pSamples;                      //; sample vector

    r5 = _m_pxor (r5, r5);                      //; zero val for EZ accumulate

    for (int i = nSamples; i > 0; --i)
    {
        __m64 *eax = (__m64 *)(pCoef+1);        //; *coefficients, skipping gain coeff
        __m64 *edi = (__m64 *)pHistory;         //; *history

        r1 = _m_from_float (*ebx);              //; current sample
        r0 = _m_from_float (pCoef[0]);          //; *gain coefficient 
        r0 = _m_pfmul (r0, r1);                 //; = input gain adjusted

        //; accumulate gain into first stage
        for (int j = nSections; j > 0; --j)
        {
            r2 = _m_from_floats (edi);          //; history[0,1]
            r1 = _m_pfmul (r2,_m_from_floats (eax));        //; history[0,1] * alpha[0,1]
            r7 = _m_from_floats (edi);          //; hist[x,1] = hist[x,0] (hoisted from pack history update)
            r1 = _m_pfadd (r1,r0);              //; add previous stage's output (hiword is zero from movd)
            r1 = _m_pfacc (r1,r5);              //; = newHistory in mm1 LSW, MSW = 0 (so we can pfadd below)

            //; update the biquad stage history buffer
            r2 = _m_pfmul (r2,_m_from_floats (eax + 1));    //; history[0,1] * beta[0,1]

            //; Pack history update into a quadword before writing.  Due to the K7
            //; write-queue optimizations, this is more efficient since each
            //; biquad must be accessed and updated for each sample.
            r3 = _m_punpckldq (r1,r7);          //; (hist[x,0] | newHistory)
            *edi = r3;                          //; hist[x,0] = newHistory
                                                //; hist[x,1] = hist[x,0]

            //; Note: mmx/x86 interleaved to relieve register contention on mm2
            r2 = _m_pfadd (r2, r1);             //; + newHist
            eax += 2;                           //; next biquad coeff section
            r2 = _m_pfacc (r2, r5);             //; = output in LSW with MSW = 0
            edi += 2;                           //; next history pair
            r0 = r2;                            //; result for next stage
        }

        *ebx++ = r0.m64_f32[0];                 //; result in-place, point to next sample
    }

    _m_femms ();
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

//; fifo data structure offsets.  Must match sFifoData in firbal.h
#define wrapCount	0	//; number of samples filtered before blockmove
#define fifoBase	4	//; fifo memory block base address
#define fifoHead	8	//; fifo address of current sample
#define nqTaps		12	//; number of taps in fifo, = (number of filter taps+1)/2
#define coeffE		16	//; even coeffcient array pointer
#define coeffO		20	//; odd coeffcient array pointer
#define fifoSrcPtr	24	//; blockMove pre-computed src ptr
#define fifoDstPtr	28	//; blockMove pre-computed dst ptr

//; filter parameters
#define samplesPerLoop	2
#define deltaLoop	    8*samplesPerLoop	//; pointer increment

//;******************************************************************************

void _realFIR (float *data, int nData, FifoData *fifo)
{
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
//flipper:
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
//accumulate:
	    pfacc	    mm2, mm3		//; accumulate odd/even products
	    movq	    [edi], mm2		//; store filtered result in-place
	    add 	    edi, 8			//; bump to next input sample pair

//; test for fifo wrap; if so, blockmove the fifo to top of fifo memory area.
	    mov	        ebx, [esi+fifoHead]	//; dec fifo head for new sample 
	    sub	        ebx, 8			    //; one quad per complex data		; 
	    cmp	        ebx, [esi+fifoBase]	//; current head in bounds?
	    jge	        noMove
//moveFifo:
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

//; filter parameter limits
//; fifo data structure offsets.  Must match CFifoData in adsp.h
#define fifoBase	0	//; fifo memory block base address
#define fifoHead	4	//; fifo address of current sample
#define nTaps		8	//; number of taps in fifo
#define coeff		12	//; complex coeffcient array pointer (=coeffE if real FIR)
#define fifoIx		16	//; current fifo array head element index
#define fifoTop		20	//; addr of top dual-quadword in fifo

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


#if 0
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
    int     *eax = (__m64 *)input;
    __m64   *edx = (__m64 *)output;
    int     ecx  = n / 4;

    _m_femms();

    r0 = _m_pxor (r0, r0);          //; a handy source of zero bytes

    while (ecx > 0)
    {
        _m_prefetchw (edx);
        r1 = _m_from_int (*eax);
        eax += 1;                       //; four samples converted
        r1 = punpcklbw  (r1,r0)         //; r1 = d:c:b:a
        r2 = r1;                        //; r2 = d:c:b:a
        r1 = punpcklwd  (r1,r0);        //; r1 = b:a
        r2 = punpckhdq  (r2,r2);        //; r2 = d:c:d:c
        r2 = punpcklwd  (r2,r0);        //; r2 = d:c



//        r1 = punpcklbw  (r1,r0)                         // d:c:b:a
//        r2 = _m_punpcklwd (_m_punpckhdq (r1, r1), r0);  // d:c 
//        r1 = _m_punpcklwd (_m_punpcklbw (r1, r0), r0);  // b:a



        r1 = pfmul      (pi2fd (r1),i128);  //; r1 /= 128
        r1 = pfsub      (r1,ones);          //; r1 -= 1.0 (re-centers values around 0)
        r2 = pfmul      (pi2fd (r2),i128)
        edx[0] = r1;                    //; store b:a samples
        r2 = _m_pfsub   (r2, ones);
        edx[1] = r2;                    //; store d:c samples
        ++edx;
        --ecx;
    }

leftovers:
        //; Handle any unaligned trailing samples
        //; (done at the end so as to not misalign the `movd's above)
        mov         ecx,[esp+n]
        shr         ecx,1
        jnc         short not_odd

        //; Handle odd case
        movzx       ebx,BYTE PTR [eax]
        inc         eax
        movd        mm1,ebx
        punpcklbw   mm1,mm0     //; mm1 = d:c:b:a
        punpcklwd   mm1,mm0     //; mm1 = b:a
        pi2fd       mm1,mm1     //; mm1 = b:a
        pfmul       mm1,i128     //; mm1 /= 128
        pfsub       mm1,ones     //; mm1 -= 1.0 (re-centers value around 0)
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
        pfmul       mm1,i128     //; mm1 /= 128
        pfsub       mm1,ones     //; mm1 -= 1.0 (re-centers value around 0)
        movq        [edx],mm1   //; store sample pair


    _m_femms();
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

        ALIGN       32
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
        mov         ecx,[esp+n]
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
        jnc         short short the_end

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

#endif
