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

 AQUAT.H

 AMD3D 3D library code: Quaternion Math

*******************************************************************************/

#ifndef __AQUAT_H__
#define __AQUAT_H__

#include <d3dtypes.h>
#include <d3drmdef.h>

typedef D3DRMQUATERNION Quat;

#ifdef __cplusplus
extern "C" {
#endif

// C quaternion functions
void D3DMat2quat(Quat *dst, const D3DMATRIX *src);
void glMat2quat (Quat *dst,  const float *src);
void quat2D3DMat(D3DMATRIX *dst, const Quat *src);
void quat2glMat (float *dst, const Quat *src);
void euler2quat (Quat *dst, const float *rot);
void euler2quat2(Quat *dst, const float *rot);
void quat2axis_angle (float *axisAngle, const Quat *quat);
void axis_angle2quat (Quat *dst, const float *axisAngle);

void add_quat (Quat *dst, const Quat *a, const Quat *b);
void sub_quat (Quat *dst, const Quat *a, const Quat *b);
void mult_quat(Quat *dst, const Quat *a, const Quat *b);
void norm_quat(Quat *dst, const Quat *a);
void slerp_quat (Quat *result, const Quat *a, const Quat *b, float slerp);
void trans_quat (D3DVECTOR *result, const D3DVECTOR *v, const Quat *q);

// 3DNow! accelerated quat functions
//void _D3DMat2quat(Quat *, const D3DMATRIX *);
//void _glMat2quat (Quat *, const float *);
void _quat2D3DMat(D3DMATRIX *, const Quat *);
//void _quat2glMat (float *, const Quat *);
void _euler2quat (Quat *quat, const float *rot);
void _quat2axis_angle (float *axisAngle, const Quat *quat);
void _axis_angle2quat (Quat *quat, const float *axisAngle);

void _add_quat (Quat *dst, const Quat *a, const Quat *b);
void _sub_quat (Quat *dst, const Quat *a, const Quat *b);
void _mult_quat(Quat *dst, const Quat *a, const Quat *b);
void _norm_quat(Quat *dst, const Quat *a);
void _slerp_quat (Quat *result, const Quat *a, const Quat *b, float slerp);
void _trans_quat (D3DVECTOR *result, const D3DVECTOR *v, const Quat *q);

#ifdef __cplusplus
}
#endif

#endif

// eof