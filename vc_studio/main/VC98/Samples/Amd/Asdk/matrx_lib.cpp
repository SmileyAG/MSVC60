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

 MATRX_LIB.C

 AMD3D 3D library code: Matrix math

******************************************************************************/

#include "amatrix.h"
#include "amath.h"


/******************************************************************************
 Routine:   det_3x3
 Input:     m - matrix (3x3) address
 Output:    returns the determinant of 'm'
******************************************************************************/
float det_3x3 (const float *mat)
{
    float det;

    det = mat[0] * (mat[4] * mat[8] - mat[7] * mat[5])
        - mat[1] * (mat[3] * mat[8] - mat[6] * mat[5])
        + mat[2] * (mat[3] * mat[7] - mat[6] * mat[4]);

    return det;
}


/******************************************************************************
 Routine:   inverse_3x3
 Input:     m - matrix (3x3) address
 Output:    returns the inverse matrix of 'm'
******************************************************************************/
void inverse_3x3 (float *res, const float *a)
{
    float det = det_3x3 (res);

    if (_fabs (det) < 0.0005f)
    {
        res[0] = 1.0f;
        res[1] = 0.0f;
        res[2] = 0.0f;
        res[3] = 0.0f;
        res[4] = 1.0f;
        res[5] = 0.0f;
        res[6] = 0.0f;
        res[7] = 0.0f;
        res[8] = 1.0f;
        return;
    }

    det = 1.0f / det;
    res[0] =   a[4]*a[8] - a[5]*a[7]  * det;
    res[1] = -(a[1]*a[8] - a[7]*a[2]) * det;
    res[2] =   a[1]*a[5] - a[4]*a[2]  * det;
      
    res[3] = -(a[3]*a[8] - a[5]*a[6]) * det;
    res[4] =   a[0]*a[8] - a[6]*a[2]  * det;
    res[5] = -(a[0]*a[5] - a[3]*a[2]) * det;
      
    res[6] =   a[3]*a[7] - a[6]*a[4]  * det;
    res[7] = -(a[0]*a[7] - a[6]*a[1]) * det;
    res[8] =   a[0]*a[4] - a[1]*a[3]  * det;
}



/******************************************************************************
 Routine:   submat_4x4
 Input:     mr  - matrix (4x4) address
            mb  - matrix (3x3) address
            i,j - matrix coordinates 
 Output:    returns the 3x3 subset of 'mr' without column 'i' and row 'j'
******************************************************************************/
void submat_4x4 (float *mb, const D3DMATRIX &mr, int i, int j)
{
    // assert (i >= 0 && i < 4);
    // assert (j >= 0 && j < 4);

#if 0
    int idst, jdst, ti, tj;

    idst = 0;
    for (ti = 0; ti < 4; ti++)
    {
        if (ti != i)
        {
            jdst = 0;
            for (tj = 0; tj < 4; tj++)
            {
                if (tj != j)
                {
                    mb[idst + jdst] = ((float *)&mr._11)[ti*4 + tj];
                    jdst++;
                }
            }
            idst += 3;
        }
    }

#else
    // Unrolled - big, but very fast (one indexed jump, one unconditional jump)
    switch (i*4+j)
    {
    // i == 0
    case 0:     // 0,0
        mb[0] = mr._22; mb[1] = mr._23; mb[2] = mr._24;
        mb[3] = mr._32; mb[4] = mr._33; mb[5] = mr._34;
        mb[6] = mr._42; mb[7] = mr._43; mb[8] = mr._44;
        break;
    case 1:     // 0,1
        mb[0] = mr._21; mb[1] = mr._23; mb[2] = mr._24;
        mb[3] = mr._31; mb[4] = mr._33; mb[5] = mr._34;
        mb[6] = mr._41; mb[7] = mr._43; mb[8] = mr._44;
        break;
    case 2:     // 0,2
        mb[0] = mr._21; mb[1] = mr._22; mb[2] = mr._24;
        mb[3] = mr._31; mb[4] = mr._32; mb[5] = mr._34;
        mb[6] = mr._41; mb[7] = mr._42; mb[8] = mr._44;
        break;
    case 3:     // 0,3
        mb[0] = mr._21; mb[1] = mr._22; mb[2] = mr._23;
        mb[3] = mr._31; mb[4] = mr._32; mb[5] = mr._33;
        mb[6] = mr._41; mb[7] = mr._42; mb[8] = mr._43;
        break;

    // i == 1
    case 4:     // 1,0
        mb[0] = mr._12; mb[1] = mr._13; mb[2] = mr._14;
        mb[3] = mr._32; mb[4] = mr._33; mb[5] = mr._34;
        mb[6] = mr._42; mb[7] = mr._43; mb[8] = mr._44;
        break;
    case 5:     // 1,1
        mb[0] = mr._11; mb[1] = mr._13; mb[2] = mr._14;
        mb[3] = mr._31; mb[4] = mr._33; mb[5] = mr._34;
        mb[6] = mr._41; mb[7] = mr._43; mb[8] = mr._44;
        break;
    case 6:     // 1,2
        mb[0] = mr._11; mb[1] = mr._12; mb[2] = mr._14;
        mb[3] = mr._31; mb[4] = mr._32; mb[5] = mr._34;
        mb[6] = mr._41; mb[7] = mr._42; mb[8] = mr._44;
        break;
    case 7:     // 1,3
        mb[0] = mr._11; mb[1] = mr._12; mb[2] = mr._13;
        mb[3] = mr._31; mb[4] = mr._32; mb[5] = mr._33;
        mb[6] = mr._41; mb[7] = mr._42; mb[8] = mr._43;
        break;

    // i == 2
    case 8:     // 2,0
        mb[0] = mr._12; mb[1] = mr._13; mb[2] = mr._14;
        mb[3] = mr._22; mb[4] = mr._23; mb[5] = mr._24;
        mb[6] = mr._42; mb[7] = mr._43; mb[8] = mr._44;
        break;
    case 9:     // 2,1
        mb[0] = mr._11; mb[1] = mr._13; mb[2] = mr._14;
        mb[3] = mr._21; mb[4] = mr._23; mb[5] = mr._24;
        mb[6] = mr._41; mb[7] = mr._43; mb[8] = mr._44;
        break;
    case 10:    // 2,2
        mb[0] = mr._11; mb[1] = mr._12; mb[2] = mr._14;
        mb[3] = mr._21; mb[4] = mr._22; mb[5] = mr._24;
        mb[6] = mr._41; mb[7] = mr._42; mb[8] = mr._44;
        break;
    case 11:    // 2,3
        mb[0] = mr._11; mb[1] = mr._12; mb[2] = mr._13;
        mb[3] = mr._21; mb[4] = mr._22; mb[5] = mr._23;
        mb[6] = mr._31; mb[7] = mr._32; mb[8] = mr._33;
        break;

    // i == 3
    case 12:    // 3,0
        mb[0] = mr._12; mb[1] = mr._13; mb[2] = mr._14;
        mb[3] = mr._22; mb[4] = mr._23; mb[5] = mr._24;
        mb[6] = mr._32; mb[7] = mr._33; mb[8] = mr._34;
        break;
    case 13:    // 3,1
        mb[0] = mr._11; mb[1] = mr._13; mb[2] = mr._14;
        mb[3] = mr._21; mb[4] = mr._23; mb[5] = mr._24;
        mb[6] = mr._31; mb[7] = mr._33; mb[8] = mr._34;
        break;
    case 14:    // 3,2
        mb[0] = mr._11; mb[1] = mr._12; mb[2] = mr._14;
        mb[3] = mr._21; mb[4] = mr._22; mb[5] = mr._24;
        mb[6] = mr._31; mb[7] = mr._32; mb[8] = mr._34;
        break;
    case 15:    // 3,3
        mb[0] = mr._11; mb[1] = mr._12; mb[2] = mr._13;
        mb[3] = mr._21; mb[4] = mr._22; mb[5] = mr._23;
        mb[6] = mr._31; mb[7] = mr._32; mb[8] = mr._33;
        break;
    }

#endif
}


/******************************************************************************
 Routine:   det_4x4
 Input:     m - matrix (4x4) address
 Output:    returns the determinant of 'm'
******************************************************************************/
float det_4x4 (const D3DMATRIX &mr)
{
    float   res = 0.0f, det;

#if 0
    float   msub3[9];

    submat_4x4 (msub3, mr, 0, 0);
    res += mr._11 * det_3x3 (msub3);

    submat_4x4 (msub3, mr, 0, 1);
    res -= mr._12 * det_3x3 (msub3);

    submat_4x4 (msub3, mr, 0, 2);
    res += mr._13 * det_3x3 (msub3);

    submat_4x4 (msub3. mr, 0, 3);
    res -= mr._14 * det_3x3 (msub3);

#else

    // 28 muls total - totally inline-expanded and factored
    // Ugly (and nearly incomprehensible) but efficient
    const float mr_3344_4334 = mr._33 * mr._44 - mr._43 * mr._34;
    const float mr_3244_4234 = mr._32 * mr._44 - mr._42 * mr._34;
    const float mr_3243_4233 = mr._32 * mr._43 - mr._42 * mr._33;
    const float mr_3144_4134 = mr._31 * mr._44 - mr._41 * mr._34;
    const float mr_3143_4133 = mr._31 * mr._43 - mr._41 * mr._33;
    const float mr_3142_4132 = mr._31 * mr._42 - mr._41 * mr._32;

    //submat_4x4 (msub3, mr, 0, 0);
    //res += mr._11 * det_3x3 (msub3);
    det = mr._22 * mr_3344_4334 - mr._23 * mr_3244_4234 + mr._24 * mr_3243_4233;
    res += mr._11 * det;

    //submat_4x4 (msub3, mr, 0, 1);
    //res -= mr._12 * det_3x3 (msub3);
    det = mr._21 * mr_3344_4334 - mr._23 * mr_3144_4134 + mr._24 * mr_3143_4133;
    res -= mr._12 * det;

    //submat_4x4 (msub3, mr, 0, 2);
    //res += mr._13 * det_3x3 (msub3);
    det = mr._21 * mr_3244_4234 - mr._22 * mr_3144_4134 + mr._24 * mr_3142_4132;
    res += mr._13 * det;

    //submat_4x4 (msub3, mr, 0, 3);
    //res -= mr._14 * det_3x3 (msub3);
    det = mr._21 * mr_3243_4233 - mr._22 * mr_3143_4133 + mr._23 * mr_3142_4132;
    res -= mr._14 * det;

#endif

    return res;
}


/******************************************************************************
 Routine:   inverse_4x4
 Input:     m - matrix (4x4) address
 Output:    returns the inverse matrix of 'm'
******************************************************************************/
int inverse_4x4 (D3DMATRIX &mr, const D3DMATRIX &ma)
{
    float   mdet = det_4x4 (ma);
    float   mtemp[9];

    if (_fabs (mdet) < 0.0005f)
        return 0;

    mdet = 1.0f / mdet;

    submat_4x4 (mtemp, ma, 0, 0);
    mr._11 =  det_3x3 (mtemp) * mdet;

    submat_4x4 (mtemp, ma, 0, 1);
    mr._12 = -det_3x3 (mtemp) * mdet;

    submat_4x4 (mtemp, ma, 0, 2);
    mr._13 =  det_3x3 (mtemp) * mdet;

    submat_4x4 (mtemp, ma, 0, 3);
    mr._14 = -det_3x3 (mtemp) * mdet;


    submat_4x4 (mtemp, ma, 1, 0);
    mr._21 = -det_3x3 (mtemp) * mdet;

    submat_4x4 (mtemp, ma, 1, 1);
    mr._22 =  det_3x3 (mtemp) * mdet;

    submat_4x4 (mtemp, ma, 1, 2);
    mr._23 = -det_3x3 (mtemp) * mdet;

    submat_4x4 (mtemp, ma, 1, 3);
    mr._24 =  det_3x3 (mtemp) * mdet;


    submat_4x4 (mtemp, ma, 2, 0);
    mr._31 =  det_3x3 (mtemp) * mdet;

    submat_4x4 (mtemp, ma, 2, 1);
    mr._32 = -det_3x3 (mtemp) * mdet;

    submat_4x4 (mtemp, ma, 2, 2);
    mr._33 =  det_3x3 (mtemp) * mdet;

    submat_4x4 (mtemp, ma, 2, 3);
    mr._34 = -det_3x3 (mtemp) * mdet;


    submat_4x4 (mtemp, ma, 3, 0);
    mr._41 = -det_3x3 (mtemp) * mdet;

    submat_4x4 (mtemp, ma, 3, 1);
    mr._42 =  det_3x3 (mtemp) * mdet;

    submat_4x4 (mtemp, ma, 3, 2);
    mr._43 = -det_3x3 (mtemp) * mdet;

    submat_4x4 (mtemp, ma, 3, 3);
    mr._44 =  det_3x3 (mtemp) * mdet;

    return 1;
}



/******************************************************************************
 Routine:   glMul_3x3
 Input:     r, a, b - pointer to 3x3 float matrix
 Output:    'r' points to the multiplication of 'a' and 'b'
******************************************************************************/
void glMul_3x3 (float *r, const float *a, const float *b)
{
    float tmp[9];
    int i, j;
 
    for (i = 0; i < 3; i++)
    {
        const int   idx = i * 3;
        const float *pa = &a[idx];
        for(j = 0; j < 3; j++)
        {
            const float *pb = &b[j];
            tmp[idx+j] = a[idx+0] * b[0+j]
                       + a[idx+1] * b[3+j]
                       + a[idx+2] * b[6+j];
        }
    }
    memcpy (r, tmp, 3 * 3 * sizeof (float));
}


/******************************************************************************
 Routine:   glMul_4x4
 Input:     r, a, b - pointer to 4x4 float matrix
 Output:    'r' points to the multiplication of 'a' and 'b'
******************************************************************************/
void glMul_4x4 (float *r, const float *a, const float *b)
{
    float tmp[16];

    tmp[0]  = b[0] * a[0]  + b[4] * a[1]  + b[8]  * a[2]  + b[12] * a[3];
    tmp[1]  = b[1] * a[0]  + b[5] * a[1]  + b[9]  * a[2]  + b[13] * a[3];
    tmp[2]  = b[2] * a[0]  + b[6] * a[1]  + b[10] * a[2]  + b[14] * a[3];
    tmp[3]  = b[3] * a[0]  + b[7] * a[1]  + b[11] * a[2]  + b[15] * a[3];
    tmp[4]  = b[0] * a[4]  + b[4] * a[5]  + b[8]  * a[6]  + b[12] * a[7];
    tmp[5]  = b[1] * a[4]  + b[5] * a[5]  + b[9]  * a[6]  + b[13] * a[7];
    tmp[6]  = b[2] * a[4]  + b[6] * a[5]  + b[10] * a[6]  + b[14] * a[7];
    tmp[7]  = b[3] * a[4]  + b[7] * a[5]  + b[11] * a[6]  + b[15] * a[7];
    tmp[8]  = b[0] * a[8]  + b[4] * a[9]  + b[8]  * a[10] + b[12] * a[11];
    tmp[9]  = b[1] * a[8]  + b[5] * a[9]  + b[9]  * a[10] + b[13] * a[11];
    tmp[10] = b[2] * a[8]  + b[6] * a[9]  + b[10] * a[10] + b[14] * a[11];
    tmp[11] = b[3] * a[8]  + b[7] * a[9]  + b[11] * a[10] + b[15] * a[11];
    tmp[12] = b[0] * a[12] + b[4] * a[13] + b[8]  * a[14] + b[12] * a[15];
    tmp[13] = b[1] * a[12] + b[5] * a[13] + b[9]  * a[14] + b[13] * a[15];
    tmp[14] = b[2] * a[12] + b[6] * a[13] + b[10] * a[14] + b[14] * a[15];
    tmp[15] = b[3] * a[12] + b[7] * a[13] + b[11] * a[14] + b[15] * a[15];

    r[0]  = tmp[0];
    r[1]  = tmp[1];
    r[2]  = tmp[2];
    r[3]  = tmp[3];
    r[4]  = tmp[4];
    r[5]  = tmp[5];
    r[6]  = tmp[6];
    r[7]  = tmp[7];
    r[8]  = tmp[8];
    r[9]  = tmp[9];
    r[10] = tmp[10];
    r[11] = tmp[11];
    r[12] = tmp[12];
    r[13] = tmp[13];
    r[14] = tmp[14];
    r[15] = tmp[15];
}


/******************************************************************************
 Routine:   D3DMul_4x4
 Input:     r, a, b - pointer to 4x4 float matrix
 Output:    'r' points to the multiplication of 'a' and 'b'
******************************************************************************/
void D3DMul_4x4 (D3DMATRIX &r, const D3DMATRIX &a, const D3DMATRIX &b)
{
    D3DMATRIX tmp;

    tmp._11 = b._11 * a._11 + b._12 * a._21 + b._13 * a._31 + b._14 * a._41;
    tmp._12 = b._11 * a._12 + b._12 * a._22 + b._13 * a._32 + b._14 * a._42;
    tmp._13 = b._11 * a._13 + b._12 * a._23 + b._13 * a._33 + b._14 * a._43;
    tmp._14 = b._11 * a._14 + b._12 * a._24 + b._13 * a._34 + b._14 * a._44;
                                                                          
    tmp._21 = b._21 * a._11 + b._22 * a._21 + b._23 * a._31 + b._24 * a._41;
    tmp._22 = b._21 * a._12 + b._22 * a._22 + b._23 * a._32 + b._24 * a._42;
    tmp._23 = b._21 * a._13 + b._22 * a._23 + b._23 * a._33 + b._24 * a._43;
    tmp._24 = b._21 * a._14 + b._22 * a._24 + b._23 * a._34 + b._24 * a._44;
                                                         
    tmp._31 = b._31 * a._11 + b._32 * a._21 + b._33 * a._31 + b._34 * a._41;
    tmp._32 = b._31 * a._12 + b._32 * a._22 + b._33 * a._32 + b._34 * a._42;
    tmp._33 = b._31 * a._13 + b._32 * a._23 + b._33 * a._33 + b._34 * a._43;
    tmp._34 = b._31 * a._14 + b._32 * a._24 + b._33 * a._34 + b._34 * a._44;
                                                         
    tmp._41 = b._41 * a._11 + b._42 * a._21 + b._43 * a._31 + b._44 * a._41;
    tmp._42 = b._41 * a._12 + b._42 * a._22 + b._43 * a._32 + b._44 * a._42;
    tmp._43 = b._41 * a._13 + b._42 * a._23 + b._43 * a._33 + b._44 * a._43;
    tmp._44 = b._41 * a._14 + b._42 * a._24 + b._43 * a._34 + b._44 * a._44;

    r = tmp;
}
