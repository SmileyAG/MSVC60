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

 ADSP.H

 AMD3D 3D library code: Digital Signal Processing

******************************************************************************/

#ifndef _AMD_ADSP_H
#define _AMD_ADSP_H

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack (push,4)

typedef struct COMPLEX {
	float real;
	float imag;
} COMPLEX, *COMPLEX_PTR;


// Data structure used by the _complexFIR() routine
typedef struct CFIFODATA {
	float	*fifoBase;		// fifo storage base address
	float	*fifoHead;		// current fifo base of (addr of newest sample)
	int		nTaps;			// number of FIR taps
	float	*coeff;			// even coeffcient array pointer
	int		fifoIx;			// current fifo head array index
	float	*fifoTop;		// pre-calculated top dualquadword of fifo address
    float   *fifoBaseAlloc; // Original location of fifoBase (for freeing)
} CFifoData;


// Data structure used by the _realFIR() routine
typedef struct FIFODATA {
	int		wrapCount;		// init to FIFO_SIZE
	float	*fifoBase;		// fifo storage base address
	float	*fifoHead;		// current fifo base of (addr of newest sample)
	int		nqTaps;			// number of FIR taps
	float	*coeffE;		// even coeffcient array pointer
	float	*coeffO;		// odd coeffcient array pointer
	float	*fifoSrcPtr;	// blockMove source pointer
	float	*fifoDstPtr;	// blockMove destination pointer
    float   *fifoBaseAlloc; // Original location of fifoBase (for freeing)
} FifoData;


// Filtering routines
void _iirInit (float *coeff, int n);
void _iir (float *pSamples, int nSamples, float *coef, int nCoef, float *history);

int  _initFifoData (FifoData *pFifoData, int nTaps, int blockSize, float *coeff);
void _realFIR(float *data, int nData, FifoData *pFifoData);
void _releaseFifoData (FifoData *);

int  _initCFifoData (CFifoData *pFifoData, int nTaps, float *coeff);
void _complexFIR(COMPLEX *data, int nData, CFifoData *pFifoData);
void _releaseCFifoData (CFifoData *);

// Fast Fourier Transformation
void	_fftInit(int m, COMPLEX *w, int forward);
void	_fft(int m, COMPLEX *w, COMPLEX *x, int forward);


// Miscellaneous routines
float	_average(int, float *);
float	_rms	(int, float *);
void	_bias	(int n, float *data, float bias);
void	_v_mult	(int n, float *input1, float *input2, float *output);
void	_minmax	(int n, float *input, float *output);
void	_convolve(int n, float *input1, float *input2, float *output);
void	_cvt_8bit  (int n, unsigned char *input, float *output);
void	_cvt_16bit (int n, short *input, float *output);


// fastcall routines
// (registers are in same order as corresponding parameters above)
void    a_average();// ecx * eax -> mm0
void    a_rms ();	// ecx * eax -> mm0
void    a_v_mult();	// ecx * eax * edx * edi -> ()
void    a_minmax();	// ecx * eax -> mm0 (max|min)

#pragma pack (pop)

#ifdef __cplusplus
}
#endif

#endif

/* eof - ADSP.H */
