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

 DSP_LIB.C

 AMD3D 3D library code: Digital Signal Processing

*******************************************************************************/

#include <stdlib.h>
#include "amath.h"
#include "adsp.h"

#pragma warning (disable:4730)  // not really happening

#define ITER 2
#define EPS  1.E-9f
#define PI  3.141592653589

static float zero[2] = { 0.0f, 0.0f };

#define _m_to_float(f) ((f).m64_f32[0])

/******************************************************************************
 Routine:   _rms
 Input:     n - sample count
            v - sample address
 Output:    return Root-Mean-Square of samples
*******************************************************************************/
float _rms (int n, float *v)
{
    float res;
    _m_femms();
    res = _m_to_float (_m_rms (n, (__m64 *)v));
    _m_femms();

    return res;
}


/******************************************************************************
 Routine:   _average
 Input:     n - sample count
            v - sample address
 Output:    return average value of samples
*******************************************************************************/
float _average (int n, float *v)
{
    float res;
    _m_femms();
    res = _m_to_float (_m_average (n, (__m64 *)v));
    _m_femms();
    return res;
}


/******************************************************************************
 Routine:   _convolve
 Input:     n      - sample count (must be same for all sample buffers)
            input1 - first sample buffer
            input2 - second sample buffer
            output - sample buffer for result
 Output:    output contains convolved signal
*******************************************************************************/
void _convolve (int n, float *input1, float *input2, float *output) 
{
    int     i, last, nscaled;
    float   *tmpx = input2 - n/2;

    // Round to next highest even number
    n += n & 1;

    nscaled = n/2;
    last = nscaled-1;
    for (i = 0; i < nscaled; i++)
    {
        float tmp = input2[i];
        input2[i] = input2[last-i];
        input2[last-i] = tmp;
    }

    _m_femms();
    nscaled = n/4;
    for (i = 0; i < n; i++, tmpx++)
    {
         __asm {
            align   4
            mov     eax,[input1]
            mov     ebx,[tmpx]
            pxor    mm0,mm0
            mov     ecx,[nscaled]
            align   4
loop1:
            movq    mm1,[eax]
            movq    mm2,[ebx]
            add     eax,8
            add     ebx,8
            pfmul   mm1,mm2
            dec     ecx
            pfadd   mm0,mm1
            jnz     short loop1

            mov     eax,[output]
            pfacc  mm0,mm0
            movd    [eax],mm0
            add     eax,4
            mov     [output],eax
        }
    }
    _m_femms();
}


/******************************************************************************
 Routine:   _iirInit
 Input:     coeff  - Coefficients (4*n+1 elements)
            n      - number of element biquad groups
 Output:    'coeff' initialized for use with _iir().
*******************************************************************************/
void _iirInit (float *coeff, int n)
{
	int i;
	float *pCoeff = &coeff[1];  // skip gain constant

	for (i = 0; i < n; i++)
	{
		pCoeff[i*4]		= -pCoeff[i*4];	
		pCoeff[1+i*4]	= -pCoeff[1+i*4];
	}
}


/******************************************************************************
 Routine:   _initCFifoData
 Input:     pFifoData   - Data structure to initialize
            nTaps       - number of elements
            coeff       - filter coefficients (used to init pFifoData)
 Output:    Returns nonzero iff error.
*******************************************************************************/
int _initCFifoData (CFifoData *pFifoData, int nTaps, float *coeff)
{
	int totalSize;

	pFifoData->nTaps = nTaps;
	pFifoData->coeff = coeff;  // copy?

	// NB extra space allocated for alignment *and* a one quadword prefetch done by FIR algorithm
	// K7 instructions allow history fifo to be one quad per complex, while K6-2 needs
	// two quadwords
	totalSize = 16*pFifoData->nTaps + 16;

	pFifoData->fifoBaseAlloc = (float *)malloc (totalSize);
	pFifoData->fifoBase = (float *)(((long)pFifoData->fifoBaseAlloc+8) & 0xFFFFFFF8);
	if (pFifoData->fifoBase)
	{	
		//pFifoData->fifoTop = (float *)((int)pFifoData->fifoBase + (nTaps-1)*16);
		pFifoData->fifoTop = &pFifoData->fifoBase[(nTaps-1)*4];
		pFifoData->fifoHead = pFifoData->fifoBase;
		pFifoData->fifoIx = 0;
		return (0);
	}
	else
		return (-4);
}


#if 0   // Not working right in this version!
/******************************************************************************
 Routine:   _initCFifoData_k7
 Input:     pFifoData   - Data structure to initialize
            nTaps       - number of elements
            coeff       - filter coefficients (used to init pFifoData)
 Output:    Returns nonzero iff error.
*******************************************************************************/
int _initCFifoData_k7 (CFifoData *pFifoData, int nTaps, float *coeff)
{
	int totalSize;

	pFifoData->nTaps = nTaps;
	pFifoData->coeff = coeff;  // copy?

	// NB extra space allocated for alignment *and* a one quadword prefetch done by FIR algorithm
	// K7 instructions allow history fifo to be one quad per complex, while K6-2 needs
	// two quadwords
	totalSize = 8*pFifoData->nTaps + 16;

	pFifoData->fifoBaseAlloc = (float *)malloc (totalSize);
	pFifoData->fifoBase = (float *)(((long)pFifoData->fifoBaseAlloc+8) & 0xFFFFFFF8);
	if (pFifoData->fifoBase)
	{	
//		pFifoData->fifoTop = (float *)((int)pFifoData->fifoBase + (nTaps-1)*8);
		pFifoData->fifoTop = &pFifoData->fifoBase[(nTaps-1)*2];
		pFifoData->fifoHead = pFifoData->fifoBase;
		pFifoData->fifoIx = 0;
		return (0);
	}
	else
		return (-4);
}
#endif


/******************************************************************************
 Routine:   _initFifoData
 Input:     pFifoData   - Data structure to initialize
            nTaps       - number of elements
            coeff       - filter coefficients (used to init pFifoData)
 Output:    Returns nonzero iff error.
            -1 = coeffE allocation failed
            -2 = coeffO allocation failed
            -3 = fifoBase allocation failed
            -4 = nTaps not even
            -5 = blockSize not even
*******************************************************************************/
int _initFifoData(FifoData *pFifoData, int nTaps, int blockSize, float *coeff)
{
	int i, totalSize;

	if (blockSize & 1)
		return(-5);
	if (!(nTaps & 1))
		return (-4);

	pFifoData->wrapCount = (blockSize/2 - 1) * 8;
	pFifoData->nqTaps = (nTaps+1)/2;
	pFifoData->coeffE = (float *)calloc(pFifoData->nqTaps, 8);
	if (pFifoData->coeffE == 0)
		return  (-1);

	pFifoData->coeffO = (float *)calloc(pFifoData->nqTaps, 8);
	if (pFifoData->coeffO == 0)
		return  (-2);

	// Initialize local copy of coefficients.  Even coefficient array has zero pad
	// at start; odd array has zero pad at end.  Two arrays are required as FIR
	// is calculated on two samples in parallel.  Thus one coefficient array is
	// equivalent to the impulse response h(t) and the other, h(t-1), allowing
	// parallel calculation of sample stream x(t).

	pFifoData->coeffE[0] = 0.0;	
	for (i=0; i<nTaps; i++)
	{
		pFifoData->coeffE[i+1] = pFifoData->coeffO[i] = coeff[i];
	}
	pFifoData->coeffO[i] = 0.0;


	//fifoQuadWords = pFifoData->nTaps + WRAP_COUNT;
	
	// NB extra space allocated for alignment *and* a one quadword prefetch done by FIR algorithm
	
	totalSize = 8 * pFifoData->nqTaps + pFifoData->wrapCount + 16;
	pFifoData->fifoBaseAlloc = (float *)malloc(totalSize);
	pFifoData->fifoBase = (float *)(((long)pFifoData->fifoBase+8) & 0xFFFFFFF8);
	if (pFifoData->fifoBaseAlloc)
	{	
		pFifoData->fifoHead = (float *)((int)pFifoData->fifoBase + pFifoData->wrapCount);
		pFifoData->fifoSrcPtr = (float *)((long)pFifoData->fifoBase + (pFifoData->nqTaps-2)*8);
		pFifoData->fifoDstPtr = (float *)((long)pFifoData->fifoSrcPtr + pFifoData->wrapCount + 8);
		return (0);
	}
	else
		return (-3);
}


/******************************************************************************
 Routine:   _releaseCFifoData
 Input:     pFifoData   - Data structure to release
 Output:    none
 Comment:   Data allocated in _initCFifoData is released by this routine.
            Do not attempt to use the fifo data pointed to by pFifoData after
            calling this routine.
*******************************************************************************/
void _releaseCFifoData (CFifoData *pFifoData)
{
    if (pFifoData->fifoBaseAlloc)
        free (pFifoData->fifoBaseAlloc);
}


/******************************************************************************
 Routine:   _releaseFifoData
 Input:     pFifoData   - Data structure to release
 Output:    none
 Comment:   Data allocated in _initFifoData or _initCFifoData is released
            by this routine.  Do not attempt to use the fifo data pointed
            to by pFifoData after calling this routine.
*******************************************************************************/
void _releaseFifoData (FifoData *pFifoData)
{
    if (pFifoData->fifoBaseAlloc)
        free (pFifoData->fifoBaseAlloc);
    if (pFifoData->coeffE)
        free (pFifoData->coeffE);
    if (pFifoData->coeffO)
        free (pFifoData->coeffO);
}


/******************************************************************************
 Routine:   _fftInit
 Input:     m      - FFT size 2^m
            w      - COMPLEX twiddle factor table of length 2^m
            forward - direction of transformation
 Output:    'w' initialized for FFT
*******************************************************************************/
void _fftInit(int m, COMPLEX *w, int forward)
{
    COMPLEX *pcW;
    float  cos_sin[2], w_real, w_imag, wrecur_real, wrecur_imag, wtemp_real;
    int j, le, n;

	forward = forward;	// not used in this function
    n = 1 << m;
    le = n/2;
    _sincos ((float)(PI/le), cos_sin); 
    wrecur_real = w_real =  cos_sin[0];  // cosine
    wrecur_imag = w_imag = -cos_sin[1];  // -sine
    pcW = w;
    for (j = 1; j < le; j++)
    {
        pcW->real = wrecur_real;
        pcW->imag = wrecur_imag;
        pcW++;
        wtemp_real = wrecur_real*w_real - wrecur_imag*w_imag;
        wrecur_imag = wrecur_real*w_imag + wrecur_imag*w_real;
        wrecur_real = wtemp_real;
    }
}


/******************************************************************************
 Routine:   _fft    - perform an in-place FFT on complex data
 Input:     m       - FFT size 2^m
            w       - COMPLEX twiddle factor table of length 2^m, initialized
                        by _fftInit()
            x       - COMPLEX data set
            forward - direction of transformation, 1 = forward, -1 = inverse
 Output:    `x' transformed
*******************************************************************************/
void _fft(int m, COMPLEX *w, COMPLEX *x, int forward)
{
    static int n = 1;           
    static const float pm[] = {+1,-1};

    COMPLEX u,ux;
    COMPLEX *xi,*xip, *xj, *wptr;
    float divisor, temp;
    register int i,j;
    int k,l,le,windex;

    n = 1 << m;
    le = n;
    windex = 1;
    __asm {
        FEMMS;
        movq    mm6,pm;
    }
    for (l = 0; l < m; l++)
    {
        le >>= 1;

        for (i = 0; i < n; i += 2*le)
        {
            xi = &x[i];
            xip = &xi[le];

            __asm {
                mov     eax,xi
                mov     edi,xip
                movq    mm0,[eax]
                movq    mm1,[edi]
                movq    mm2,mm0
                pfadd   mm0,mm1
                pfsub   mm2,mm1
                movq    [eax],mm0
                movq    [edi],mm2
            }
        }

        wptr = &w[windex - 1];
        for (j = 1; j < le; j++) 
        {
            u = *wptr;
            ux.real = wptr->imag;
            ux.imag = wptr->real;   
            __asm movq  mm7,ux;
            for (i = j; i < n; i += 2*le) 
            {
                xi = x + i;
                xip = xi + le;
                __asm {
                    ALIGN 4
                    mov     eax,xi
                    mov     edi,xip
                    movq    mm0,[eax]
                    movq    mm1,[edi]
                    movq    mm2,mm0
                    pfadd   mm0,mm1
                    pfsub   mm2,mm1
                    movq    mm3,[u]
                    movq    [eax],mm0
                    pfmul   mm3,mm2
                    pfmul   mm3,mm6
                    pfacc   mm3,mm3
                    pfmul   mm2,mm7
                    movd    [edi],mm3
                    pfacc   mm2,mm2
                    movd    [edi+4],mm2
                }
            }
            wptr += windex;
        }
        windex <<= 1;
    }            

    j = 0;
    for (i = 1; i < (n-1); i++) 
    {
        k = n >> 1;
        while (k <= j) 
        {
            j -= k;
            k >>= 1;
        }
        j += k;
        if (i < j)
        {
            xi = &x[i];
            xj = &x[j];
            __asm {
                mov     eax,xi;
                movq    mm0,[eax];
                mov     ecx,xj;
                movq    mm1,[ecx];
                movq    [ecx],mm0;
                movq    [eax],mm1;
            }
        }
    }

    __asm femms;

    // correct adjust for inverse fft (JBN 990428)
    if (forward == -1)
    {
        divisor = -1.0f / (float)n;
        for (i = 0; i < n; i++)
        {
            temp = x[i].real*divisor;
            x[i].real = x[i].imag*divisor;
            x[i].imag = temp;
        }
    }
}

// eof
