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

 ATRAN.H

 AMD3D 3D library code: 3D Transformation

*******************************************************************************/

#ifndef _AMD_ATRAN_H
#define _AMD_ATRAN_H

#include <d3dtypes.h>

#ifdef __cplusplus
extern "C" {
#endif
 
#ifdef __cplusplus
void _D3DTrans_4x4  (D3DTLVERTEX &res, const D3DTLVERTEX &pt, const D3DMATRIX &mtx);
void _D3DTrans_a4x4 (D3DTLVERTEX *res, const D3DTLVERTEX *pt, const D3DMATRIX &mtx, int npt);
#endif
void _glTrans_4x4   (float *res, const float *pt,  const float *mtx);
void _glTrans_a4x4  (float *res, const float *pts, const float *mtx, int npt);

void _D3DTrans_3x3 (float *res, const float *pt, const float *mtx);
void _glTrans_3x3  (float *res, const float *pt, const float *mtx);
void _D3DTrans_a3x3(float *res, const float *pt, const float *mtx, int npt);
void _glTrans_a3x3 (float *res, const float *pt, const float *mtx, int npt);


#ifdef __cplusplus
void D3DTrans_4x4 (D3DTLVERTEX &res, const D3DTLVERTEX &pt, const D3DMATRIX &mtx);
void D3DTrans_a4x4(D3DTLVERTEX *res, const D3DTLVERTEX *pt, const D3DMATRIX &mtx, int npt);
#endif
void glTrans_4x4  (float *res, const float *pt,  const float *mtx);
void glTrans_a4x4 (float *res, const float *pts, const float *mtx, int npt);

void D3DTrans_3x3 (float *res, const float *pt, const float *mtx);
void glTrans_3x3  (float *res, const float *pt, const float *mtx);
void D3DTrans_a3x3(float *res, const float *pt, const float *mtx, int npt);
void glTrans_a3x3 (float *res, const float *pt, const float *mtx, int npt);

#ifdef __cplusplus
}
#endif

#endif

/* eof - ATRANS.H */
