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

 AMATRIX.H

 AMD3D 3D library code: Maxtrix Manipulation

*******************************************************************************/

#ifndef _AMD_AMATRIX_H
#define _AMD_AMATRIX_H

#include <d3dtypes.h>

#ifdef __cplusplus
extern "C" {
#endif
 
void _glMul_3x3 (float *res, const float *a, const float *b);
void _glMul_4x4  (float *res, const float *a, const float *b);
void glMul_3x3 (float *res, const float *a, const float *b);
void glMul_4x4 (float *res, const float *a, const float *b);

#ifdef __cplusplus
void _D3DMul_4x4 (D3DMATRIX &res, const D3DMATRIX &a, const D3DMATRIX &b);
void D3DMul_4x4 (D3DMATRIX &res, const D3DMATRIX &a, const D3DMATRIX &b);

float det_3x3    (const float *);
void  inverse_3x3(float *, const float *);
void  submat_4x4 (float *, const D3DMATRIX &, int i, int j);
float det_4x4    (const D3DMATRIX &);
int   inverse_4x4(D3DMATRIX &, const D3DMATRIX &);
#endif

#ifdef __cplusplus
}
#endif

#endif

/* eof - AMATRIX.H */
