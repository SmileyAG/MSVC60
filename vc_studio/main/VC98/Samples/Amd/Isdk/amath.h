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

 AMATH.H

 AMD3D 3D library code: Math routines

*******************************************************************************/

#ifndef _AMD_AMATH_H
#define _AMD_AMATH_H

#include <mm3dnow.h>

#ifdef __cplusplus
extern "C" {
#endif

float	_atan(float);
float	_atan2(float,float);
float	_acos(float);
float	_asin(float);
float   _log(float);
float   _log10(float);
float   _pow(float, float);
float	_exp(float);
float   _sqrt(float);
float 	_fabs(float);
float   _ceil(float);
float 	_floor(float);
float 	_frexp(float, int *);
float	_ldexp(float, int);
float   _modf(float, float *);
float	_fmod(float, float);
void	_sincos(float, float *);
float	_sin(float);
float	_cos(float);
float   _tan(float);

// "fastcall" register called routines.
__m64	_m_atan(__m64);	        // mm0 -> mm0
__m64	_m_atan2(__m64); // mm0 * mm1 -> mm0
__m64	_m_acos(__m64);	        // mm0 -> mm0
__m64	_m_asin(__m64);	        // mm0 -> mm0
__m64	_m_log(__m64);	        // mm0 -> mm0
__m64	_m_log10(__m64);        // mm0 -> mm0
__m64	_m_exp(__m64);	        // mm0 -> mm0
__m64	_m_sqrt(__m64);	        // mm0 -> mm0
__m64 	_m_fabs(__m64);	        // mm0 -> mm0
__m64	_m_ceil(__m64);	        // mm0 -> mm0
__m64 	_m_floor(__m64);	    // mm0 -> mm0
__m64 	_m_frexp(__m64);	    // mm0 -> mm0 (mantissa|exponent)
__m64	_m_ldexp(__m64, __m64);	// mm0 * mm1 -> mm0
__m64	_m_modf(__m64);	        // mm0 -> mm0 (mod|rem)
__m64	_m_fmod(__m64, __m64);	// mm0 * mm1 -> mm0
__m64	_m_sincos(__m64);	    // mm0 -> mm0 (cos|sin)
__m64	_m_sin(__m64);	        // mm0 -> mm0
__m64	_m_cos(__m64);	        // mm0 -> mm0
__m64	_m_tan(__m64);	        // mm0 -> mm0
__m64	_m_pow(__m64, __m64);	// mm0 * mm1 -> mm0
#ifdef __cplusplus
}
#endif

#endif

/* eof - AMATH.H */
