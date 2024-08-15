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

 TRANS_LIB.C

 AMD3D 3D library code: Transformation routines

******************************************************************************/

#include "atrans.h"
#include "amath.h"


/******************************************************************************
 Routine:   D3DTrans_4x4
 Input:     r - output D3D vector
            a - input D3D vector
            m - transformation D3D matrix
 Output:    r = a * m
******************************************************************************/
void D3DTrans_4x4 (D3DTLVERTEX &r, const D3DTLVERTEX &a, const D3DMATRIX &m)
{
    r.sx = a.sx * m._11 + a.sy * m._21 + a.sz * m._31 + a.rhw * m._41;
    r.sy = a.sx * m._12 + a.sy * m._22 + a.sz * m._32 + a.rhw * m._42;
    r.sz = a.sx * m._13 + a.sy * m._23 + a.sz * m._33 + a.rhw * m._43;
    r.rhw= a.sx * m._14 + a.sy * m._24 + a.sz * m._34 + a.rhw * m._44;
}

/******************************************************************************
 Routine:   glTrans_4x4
 Input:     r - output vector, float[4]
            a - input vector, float[4]
            m - transformation matrix, float[9], in column major (gl) order
 Output:    r = a * m
******************************************************************************/
void glTrans_4x4 (float *r, const float *a, const float *m)
{
    r[0] = a[0] * m[0]  + a[1] * m[1]  + a[2] * m[2]  + a[3] * m[3];
    r[1] = a[0] * m[4]  + a[1] * m[5]  + a[2] * m[6]  + a[3] * m[7];
    r[2] = a[0] * m[8]  + a[1] * m[9]  + a[2] * m[10] + a[3] * m[11];
    r[3] = a[0] * m[12] + a[1] * m[13] + a[2] * m[14] + a[3] * m[15];
}


/******************************************************************************
 Routine:   D3DTrans_3x3
 Input:     r - output vector, float[3]
            a - input vector, float[3]
            m - transformation matrix, float[9], in row major (D3D) order
 Output:    r = a * m
******************************************************************************/
void D3DTrans_3x3 (float *r, const float *a, const float *m)
{
    r[0] = a[0] * m[0] + a[1] * m[3] + a[2] * m[6];
    r[1] = a[0] * m[1] + a[1] * m[4] + a[2] * m[7];
    r[2] = a[0] * m[2] + a[1] * m[5] + a[2] * m[8];
}


/******************************************************************************
 Routine:   glTrans_3x3 (gl)
 Input:     r - output vector, float[3]
            a - input vector, float[3]
            m - transformation matrix, float[9], in column major (gl) order
 Output:    
******************************************************************************/
void glTrans_3x3 (float *r, const float *a, const float *m)
{
    r[0] = a[0] * m[0] + a[1] * m[1] + a[2] * m[2];
    r[1] = a[0] * m[3] + a[1] * m[4] + a[2] * m[5];
    r[2] = a[0] * m[6] + a[1] * m[7] + a[2] * m[8];
}


/******************************************************************************
 Routine:   D3DTrans_a3x3
 Input:     r - output vector, float[3][n]
            a - input vector, float[3][n]
            m - transformation matrix, float[9], in row major (D3D) order
            n - number of vectors to transform
 Output:    r[i] = a[i] * m[i] where i = 0..n-1
******************************************************************************/
void D3DTrans_a3x3 (float *r, const float *a, const float *m, int n)
{   
    int i;
    for (i = 0; i < n; i++)
    {
        int idx = 3*i;
        r[idx+0] = a[idx] * m[0] + a[idx+1] * m[3] + a[idx+2] * m[6];
        r[idx+1] = a[idx] * m[1] + a[idx+1] * m[4] + a[idx+2] * m[7];
        r[idx+2] = a[idx] * m[2] + a[idx+1] * m[5] + a[idx+2] * m[8];
    }
}


/******************************************************************************
 Routine:   glTrans_a3x3
 Input:     r - output vector, float[3][n]
            a - input vector, float[3][n]
            m - transformation matrix, float[9], in column major (gl) order
            n - number of vectors to transform
 Output:    r[i] = a[i] * m[i] where i = 0..n-1
******************************************************************************/
void glTrans_a3x3 (float *r, const float *a, const float *m, int n)
{
    int i;
    for (i = 0; i < n; i++)
    {
        const int idx = i*3;
        r[idx+0] = a[idx] * m[0] + a[idx+1] * m[1] + a[idx+2] * m[2];
        r[idx+1] = a[idx] * m[3] + a[idx+1] * m[4] + a[idx+2] * m[5];
        r[idx+2] = a[idx] * m[6] + a[idx+1] * m[7] + a[idx+2] * m[8];
    }
}


/******************************************************************************
 Routine:   D3DTrans_a4x4
 Input:     r - 'n' output D3D vectors
            a - 'n' input D3D vectors
            m - D3D transformation matrix
            n - number of vectors to transform
 Output:    r[i] = a[i] * m[i] where i = 0..n-1
******************************************************************************/
void D3DTrans_a4x4 (D3DTLVERTEX *r, const D3DTLVERTEX *a, const D3DMATRIX &m, int n)
{
    int i;
    for (i = 0; i < n; i++)
    {
        const D3DTLVERTEX *pa = &a[i];
        D3DTLVERTEX *pr = &r[i];
        r[i].sx  = pa->sx * m._11 + pa->sy * m._21 + pa->sz * m._31 + pa->rhw * m._41;
        r[i].sy  = pa->sx * m._12 + pa->sy * m._22 + pa->sz * m._32 + pa->rhw * m._42;
        r[i].sz  = pa->sx * m._13 + pa->sy * m._23 + pa->sz * m._33 + pa->rhw * m._43;
        r[i].rhw = pa->sx * m._14 + pa->sy * m._24 + pa->sz * m._34 + pa->rhw * m._44;
    }
}


/******************************************************************************
 Routine:   glTrans_a4x4
 Input:     r - output vector, float[4][n]
            a - input vector, float[4][n]
            m - transformation matrix, float[16], in row major (D3D) order
            n - number of vectors to transform
 Output:    r[i] = a[i] * m[i] where i = 0..n-1
******************************************************************************/
void glTrans_a4x4 (float *r, const float *a, const float *m, int n)
{
    int i;
    for (i = 0; i < n; i++)
    {
        const int idx = i*4;
        const float *pa = &a[idx];
        r[idx+0] = pa[0] * m[0]  + pa[1] * m[1]  + pa[2] * m[2]  + pa[3] * m[3];
        r[idx+1] = pa[0] * m[4]  + pa[1] * m[5]  + pa[2] * m[6]  + pa[3] * m[7];
        r[idx+2] = pa[0] * m[8]  + pa[1] * m[9]  + pa[2] * m[10] + pa[3] * m[11];
        r[idx+3] = pa[0] * m[12] + pa[1] * m[13] + pa[2] * m[14] + pa[3] * m[15];
    }
}

// eof