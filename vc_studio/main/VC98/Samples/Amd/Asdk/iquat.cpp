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
//; QUAT.ASM
//;
//; AMD3D 3D library code: Quaternion operations.
//;
//; See quat_lib.c for algorithm details and quat.h for function prototypes.
//;
//; Quaternions are represented as four float values, <x,y,z,w>
//;
//;******************************************************************************

#include "amath.h"
#include "aquat.h"

#pragma warning (disable:4799)  // No (F)EMMS

//;******************************************************************************
//; Defines:  Parameter offsets for all routines
//;******************************************************************************

#define MATRIX_D_00 0
#define MATRIX_D_01 1*4
#define MATRIX_D_02 2*4
#define MATRIX_D_03 3*4
#define MATRIX_D_10 4*4
#define MATRIX_D_11 5*4
#define MATRIX_D_12 6*4
#define MATRIX_D_13 7*4
#define MATRIX_D_20 8*4
#define MATRIX_D_21 9*4
#define MATRIX_D_22 10*4
#define MATRIX_D_23 11*4
#define MATRIX_D_30 12*4
#define MATRIX_D_31 13*4
#define MATRIX_D_32 14*4
#define MATRIX_D_33 15*4

//;******************************************************************************
//; Some handy constants
//;******************************************************************************

const float TWOS[2] = { 2.0f, 2.0f };
const float ONES[2] = { 1.0f, 1.0f };
const int   NEG2[2] = { 0x80000000, 0 };
const float HALFS[2] = { 0.5f, 0.5f };
const float DELTA_1 = -0.999;
const float DELTA_2 = 0.999;
const float HALF_PI = 1.570796f;
const float DEG2RAD2[2] = { 0.0087266462599716478846184538f, 0.0087266462599716478846184538f };
const float RAD2DEG2[2] = { 114.59155902616464175359630962f, 114.59155902616464175359630962f };

//DEG2RAD2    real4   0.0087266462599716478846184538, 0.0087266462599716478846184538 ; (pi/360.0), (pi/360.0)
//RAD2DEG2    real4   114.59155902616464175359630962, 114.59155902616464175359630962 ; 2 * (180/pi)


//;******************************************************************************
//; Routine:  void _add_quat (D3DRMQUATERNION *r, const D3DRMQUATERNION *a, const D3DRMQUATERNION *b)
//; Input:    r - resultant quaternion
//;           a - input quaternion
//;           b - input quaternion
//; Output:   r[i] = a[i] + b[i]
//; Uses:     eax,edx,mm0,mm1
//; Comment:
//;   Quaternion addition is defined as (aw + bw, ax + bx, ay + by, az + bz)
//;   This routine is heavily memory i/o bound, and uses the load-execute
//;   instruction model (efficient on K7, not too bad on K6).
//;******************************************************************************
void _add_quat (D3DRMQUATERNION *r, const D3DRMQUATERNION *a, const D3DRMQUATERNION *b)
{
    __asm
    {
        femms
        mov         eax,[a]         //; eax = a
        mov         edx,[b]         //; edx = b
        movq        mm0,[eax]       //; mm0 = (a.x | a.w)
        movq        mm1,[eax+8]     //; mm1 = (a.z | a.y)
        pfadd       mm0,[edx]       //; mm0 = (a.x + b.x | a.w + b.w)
        mov         eax,[r]         //; eax = r
        pfadd       mm1,[edx+8]     //; mm1 = (a.z + b.z | a.y + b.y)
        movq        [eax],mm0       //; store (r.x | r.w)
        movq        [eax+8],mm1     //; store (r.z | r.y)
        femms
    }
}


//;******************************************************************************
//; Routine:  void _sub_quat (D3DRMQUATERNION *r, const D3DRMQUATERNION *a, const D3DRMQUATERNION *b)
//; Input:    r - resultant quaternion
//;           a - input quaternion
//;           b - input quaternion
//; Output:   r[i] = a[i] - b[i]
//; Uses:     eax, edx, mm0, mm1
//; Comment:
//;   Quaternion subtraction is defined as (aw - bw, ax - bx, ay - by, az - bz)
//;   This routine is heavily memory i/o bound, and uses the load-execute
//;   instruction model (efficient on K7, not too bad on K6).
//;******************************************************************************
void _sub_quat (D3DRMQUATERNION *r, const D3DRMQUATERNION *a, const D3DRMQUATERNION *b)
{
    __asm
    {
        femms
        mov         eax,[a]         //; eax = a
        mov         edx,[b]         //; edx = b
        movq        mm0,[eax]       //; mm0 = (a.x | a.w)
        movq        mm1,[eax+8]     //; mm1 = (a.z | a.y)
        pfsub       mm0,[edx]       //; mm0 = (a.x - b.x | a.w - b.w)
        mov         eax,[r]         //; eax = r
        pfsub       mm1,[edx+8]     //; mm1 = (a.z - b.z | a.y - b.y)
        movq        [eax],mm0       //; store (r.x | r.w)
        movq        [eax+8],mm1     //; store (r.z | r.y)
        femms
    }
}


//;******************************************************************************
//; Routine:  void _mult_quat (D3DRMQUATERNION *r, const D3DRMQUATERNION *a, const D3DRMQUATERNION *b)
//; Input:    r - resultant quaternion
//;           a - input quaternion
//;           b - input quaternion
//; Output:   r = a * b, using the definition of quaternion multiplication
//; Uses:     eax, ecx, edx, mm0-mm7
//; Comment:
//;   Quaternion multiplication is defined as:
//;   rw = aw * bw - ax * bx - ay * by - az * bz
//;   rx = ax * bw + bx * aw + ay * bz - az * by;
//;   ry = ay * bw + by * aw + az * bx - ax * bz;
//;   rz = az * bw + bz * aw + ax * by - ay * bx;
//;
//;   Each x,y,z is equal to sqrt(-1) (quaternions are a kind of complex number),
//;   and multiplication is defined such that xy = -z, xz = -y, and yz = -x.  The 'w'
//;   value is a real number.  The above relations come from algebraic multiplication
//;   of the quaternions.
//;
//;******************************************************************************
void _mult_quat (D3DRMQUATERNION *r, const D3DRMQUATERNION *a, const D3DRMQUATERNION *b)
{
    __asm
    {
        femms
        mov         eax,[a]
        mov         ecx,[b]

        movq        mm0,[eax]       //; mm0 = ax|aw
        movq        mm1,[ecx]       //; mm1 = bx|bw
                                          
        pfmul       mm0,mm1         //; mm0 = ax*bx|aw*bw
        movq        mm2,[eax+8]     //; mm2 = az|ay
        movq        mm3,[ecx+8]     //; mm3 = bz|by
                                          
        mov         edx,[r]           
        movq        mm5,mm1         //; mm5 = bx|bw
                                          
        movq        mm4,mm0         //; mm4 = ax*bx|aw*bw
        punpckhdq   mm0,mm0         //; mm0 = ax*bx|ax*bx
                                          
        pfmul       mm2,mm3         //; mm2 = az*bz|ay*by
        pfsub       mm4,mm0         //; mm4 = ???|aw*bw-ax*bx
                                          
        punpckhdq   mm5,mm5         //; mm5 = bx|bx
        movq        mm6,mm3         //; mm6 = bz|by
                                          
        pfacc       mm2,mm2         //; mm2 = ???|az*bz+ay*by
        punpckldq   mm5,mm1         //; mm5 = bw|bx
                                          
        pfmul       mm5,[eax]       //; mm5 = ax*bw|aw*bx
        punpckhdq   mm6,mm6         //; mm6 = bz|bz
                                          
        pfsub       mm4,mm2         //; mm4 = ???|rw
        movq        mm7,[eax+8]     //; mm7 = az|ay
                                          
        pfacc       mm5,mm5         //; mm5 = ???|ax*bw+aw*bx
        punpckldq   mm6,mm3         //; mm6 = by|bz
                                          
        pfmul       mm6,mm7         //; mm6 = az*by|ay*bz
        movd        [edx],mm4       //; mm4 = ???|rw
                                          
        movd        mm2,[eax]       //; mm2 = ???|aw
        movq        mm0,mm1         //; mm0 = bx|bw

        pfadd       mm5,mm6         //; mm5 = ???|ax*bw+aw*bx+ay*bz
        punpckhdq   mm6,mm6         //; mm6 = ay*bz|ay*bz
                                          
        punpckldq   mm0,mm0         //; mm0 = bw|bw
        punpckldq   mm2,mm2         //; mm2 = aw|aw
                                          
        pfsub       mm5,mm6         //; mm5 = ???|rx
        pfmul       mm0,mm7         //; mm0 = az*bw|ay*bw
                                          
        punpckhdq   mm7,[eax+0]     //; mm7 = ax|az
        pfmul       mm2,mm3         //; mm2 = bz*aw|by*aw
                                          
        punpckhdq   mm1,mm1         //; mm1 = bx|bx
        movd        [edx+4],mm5     //; mm5 = ???|rx

        pfadd       mm2,mm0         //; mm2 = az*bw+bz*aw|by*aw+ay*bw
        punpckldq   mm1,mm3         //; mm1 = by|bx

        pfmul       mm7,mm1         //; mm7 = ax*by|az*bx
        movd        mm4,[eax+4]     //; mm3 = ???|ax

        punpckhdq   mm3,[ecx+8]     //; mm3 = bx|bz

        pfadd       mm2,mm7         //; mm2 = az*bw+bz*aw+ax*by|by*aw+ay*bw+az*bx
        punpckldq   mm4,[eax+4]     //; mm4 = ay|ax
        pfmul       mm3,mm4         //; mm3 = bx*ay|bz*ax
        pfsub       mm2,mm3         //; mm2 = rz|ry
        movq        [edx+8],mm2     //; mm2 = rz|ry
        femms
    }
}



//;******************************************************************************
//; Routine:  void _norm_quat (D3DRMQUATERNION *r, const D3DRMQUATERNION *a)
//; Input:    r - resultant quaternion
//;           a - input quaternion
//; Output:   r = normalize(a) such that (x^2 + y^2 + z^2 + w^2 = 1)
//; Uses:     eax,mm0-mm4
//; Comment:
//;   Normal quaternion form is (x^2 + y^2 + z^2 + w^2 = 1).
//;******************************************************************************
void _norm_quat (D3DRMQUATERNION *r, const D3DRMQUATERNION *a)
{
    __asm
    {
        mov         eax,[a]
        femms
        movq        mm0,[eax]       ; mm0 = (x|w)
        movq        mm1,[eax+8]     ; mm1 = (z|y)
        movq        mm2,mm0         ; save (x|w)
        pfmul       mm0,mm0         ; mm0 = (x^2|w^2)
        movq        mm3,mm1         ; save (z|y)
        pfmul       mm1,mm1         ; mm1 = (z^2|y^2)
        mov         eax,[r]
        pfadd       mm1,mm0         ; mm1 = (x^2 + z^2 | w^2 + y^2)
        pfacc       mm1,mm1         ; mm1 = x^2 + y^2 + z^2 + w^2
        pfrsqrt     mm0,mm1         ; mm0 = 1/sqrt(mm1)
        movq        mm4,mm0         ; +++ sqrt calc
        pfmul       mm0,mm0         ; +++ sqrt calc
        pfrsqit1    mm0,mm1         ; +++ sqrt calc
        pfrcpit2    mm0,mm4         ; mm0 = 1/sqrt(mm0)
        punpckldq   mm0,mm0         ; mm0 = (1/sqrt(mm0)|1/sqrt(mm0))
        pfmul       mm2,mm0         ; mm2 = (x/sqrt(...)|w/sqrt(...))
        pfmul       mm3,mm0         ; mm3 = (z/sqrt(...)|y/sqrt(...))
        movq        [eax],mm2
        movq        [eax+8],mm3
        femms
    }
}



//;******************************************************************************
//; Routine:  void _quat2D3DMat (D3DMATRIX *A1, const D3DRMQUATERNION *A0)
//; Input:    A0 - quaternion address
//;           A1 - matrix (4x4) address
//; Output:   A1 = rotation matrix from A0
//; Uses:     eax, mm0-mm7
//;******************************************************************************
void _quat2D3DMat (D3DMATRIX *A1, const D3DRMQUATERNION *A0)
{
    __asm
    {
        femms
        mov         eax,[A0]
        movq        mm0,[eax]               //; mm0 qx|qw
        movq        mm1,[eax+8]             //; mm1 qz|qy
        movq        mm2,mm0                 //; mm2 qx|qw
        movq        mm3,mm1                 //; mm3 qz|qy
        pfmul       mm2,mm2                 //; mm2 qx*qx|qw*qw

        movq        mm5,mm0                 //; mm5 = qx|qw
        pfmul       mm3,mm3                 //; mm3 = qz*qz|qy*qy

        pfacc       mm3,mm2                 //; mm3 = qx*qx+qw*qw|qz*qz+qy*qy

        pfacc       mm3,mm3                 //; mm3 = qx*qx+qw*qw|qz*qz+qy*qy|qx*qx+qw*qw|qz*qz+qy*qy

        movq        mm2,[TWOS]              //; mm2 = 2.0|2.0
        pfrcp       mm4,mm3

        pfrcpit1    mm3,mm4
        mov         eax,[A1]

        pfrcpit2    mm3,mm4
        movq        mm7,mm0                 //; mm7 = qx|qw

        pfmul       mm2,mm3                 //; mm2 = s|s
        punpckhdq   mm7,mm7                 //; mm7 = qx|qx
                                                
        movq        mm6,mm0                 //; mm6 = qx|qw
        pfmul       mm5,mm2                 //; mm5 = qx * s|???

        pfmul       mm2,mm1                 //; mm2 = qz*s(zs)|qy*s(ys)
        punpckldq   mm7,mm0                 //; mm7 = qw|qx

        punpckhdq   mm5,mm5                 //; mm5 = xs|xs

        movq        mm3,mm1                 //; mm3 = qz|qy
        pfmul       mm5,mm0                 //; mm5 = qx*xs(xx)|qw * xs(wx)

        pfmul       mm6,mm2                 //; mm6 = qx*zs(xz)|qw*ys(wy)

        pfmul       mm7,mm2                 //; mm7 = qw*zs(wz)|qx*ys(xy)

        pfmul       mm3,mm2                 //; mm3 = qz*zs(zz)|qy*ys(yy)
        punpckhdq   mm2,mm2                 //; mm2 = zs|zs

        pfmul       mm2,mm1                 //; mm2 = ???|q.y*zs(yz)
        movq        mm0,mm3                 //; mm0 = zz|yy

        movq        mm1,[ONES]
        punpckhdq   mm0,mm5                 //; mm0 = xx|zz

        pfacc       mm0,mm3                 //; mm0 = zz+yy|xx+zz

        punpckldq   mm4,mm3                 //; mm4 = yy|???

        pfsub       mm1,mm0                 //; mm1 = 1-(zz+yy)|1-(xx+zz)
        pfadd       mm4,mm5                 //; mm4 = xx+yy|???

        movq        mm0,[ONES]
        movq        mm3,mm6                 //; mm3 = xz | wy

        movd        [eax+MATRIX_D_11],mm1
        punpckhdq   mm1,mm1                 //; mm1 = ???|1-(zz+yy)

        pfsub       mm0,mm4                 //; mm0 = 1-(xx+yy)|???
        movd        [eax+MATRIX_D_00],mm1   //; mm1 = ???|1-(zz+yy)

        movq        mm1,mm5                 //; mm1 = xx | wx
        pfacc       mm3,mm3                 //; mm3 = ???|xz+wy

        punpckhdq   mm0,mm0                 //; mm0 = ???|1-(xx+yy)
        punpckldq   mm1,mm2                 //; mm1 = yz | wx

        movd        [eax+MATRIX_D_22],mm0   //; mm0 = ???|1-(xx+yy)
        pfacc       mm1,mm7                 //; mm1 = wz+xy|yz+wx

        movq        mm0,mm6                 //; mm0 = xz | wy
        movd        [eax+MATRIX_D_02],mm3   //; mm3 = ???|xz+wy

        punpckhdq   mm0,mm0                 //; mm0 = xz | xz
        pfsub       mm2,mm5                 //; mm2 = ???|yz-wx

        movq        mm4,mm7                 //; mm4 = wz | xy
        pfsub       mm0,mm6                 //; mm0 = ???|xz-wy

        movd        [eax+MATRIX_D_21],mm1   //; mm1 = wz+xy|yz+wx
        punpckhdq   mm4,mm4                 //; mm4 = wz | wz

        punpckhdq   mm1,mm1                 //; mm1 = wz+xy|wz+xy
        pfsub       mm7,mm4                 //; mm7 = ???| xy-wz

        pxor        mm3,mm3
        movd        [eax+MATRIX_D_10],mm1   //; mm1 = wz+xy|wz+xy
        movd        [eax+MATRIX_D_20],mm0   //; mm0 = ???|xz-wy
        movd        [eax+MATRIX_D_12],mm2   //; mm2 = ???|yz-wx
        movd        [eax+MATRIX_D_01],mm7   //; mm7 = ???| xy-wz

        //; Clear out remaining entries (collapse into movq's?)
        movq        mm0,[ONES]
        movd        [eax+MATRIX_D_03],mm3
        movd        [eax+MATRIX_D_13],mm3
        movd        [eax+MATRIX_D_23],mm3
        movd        [eax+MATRIX_D_33],mm3
        movd        [eax+MATRIX_D_30],mm3
        movd        [eax+MATRIX_D_31],mm3
        movd        [eax+MATRIX_D_32],mm3
        movd        [eax+MATRIX_D_33],mm0

        femms
    }
}



//;******************************************************************************
//; Routine:  void _euler2quat (D3DRMQUATERNION *A1, const float *A0)
//; Input:    A1 - resultant quaternion
//;           A0 - euler angles address (array of 3 floats)
//; Output:   A1 = quaternion performing the specified rotations
//; Uses:     eax, ecx, edx, mm0-mm7
//;******************************************************************************
void _euler2quat (D3DRMQUATERNION *A1, const float *A0)
{
    __m64 csx, cssc;
    __asm
    {
        femms
        mov         eax,[A0]
        movd        mm0,[eax]
        pfmul       mm0,QWORD PTR [DEG2RAD2]
        call        a_sincos
        movq        [csx],mm0       //; stash for later (csx)

        mov         eax,[A0]
        movd        mm0,[eax+8]
        pfmul       mm0,QWORD PTR [DEG2RAD2]
        call        a_sincos       //; (csz)
        movq        mm2,[csx]

        //; cc = csx[0] * csz[0];
        //; ss = csx[1] * csz[1];
        movq        mm1,mm2
        pfmul       mm2,mm0         //; mm2 = cc:ss

        //; cs = csx[0] * csz[1];
        //; sc = csx[1] * csz[0];
        movq        mm3,mm0         //; mm3 = swap(mm0), fast method (trash mm0)
        punpckldq   mm0,mm0
        punpckhdq   mm3,mm0
        movq        [csx],mm2
        pfmul       mm3,mm1         //; mm3 = cs:sc
        movq        [cssc],mm3

        mov         eax,[A0]
        movd        mm0,[eax+4]
        pfmul       mm0,QWORD PTR [DEG2RAD2]
        call        a_sincos       //; mm0 = cy:sy
        mov         eax,[A1]
        movq        mm1,[csx]       //; mm1 = cc:ss
        movq        mm3,mm0
        movq        mm2,[cssc]      //; mm2 = cs:sc
        pxor        mm3,[NEG2]      //; mm3 = cy/-sy

        movq        mm4,mm1
        movq        mm5,mm2
        punpckhdq   mm4,mm4
        punpckhdq   mm5,mm5
        punpckldq   mm4,mm1         //; mm4 = ss:cc
        punpckldq   mm5,mm2         //; mm5 = sc:cs

        //; final computation
        //; quat[0] = (cy * cc) + (sy * ss);
        pfmul       mm1,mm0
        pfacc       mm1,mm1
        movd        [eax],mm1

        //; quat[1] = (cy * sc) - (sy * cs);
        pfmul       mm5,mm3
        pfacc       mm5,mm5
        movd        [eax+4],mm5

        //; quat[2] = (cy * ss) + (sy * cc);
        pfmul       mm4,mm0
        pfacc       mm4,mm4
        movd        [eax+8],mm4

        //; quat[3] = (cy * cs) - (sy * sc);
        pfmul       mm2,mm3
        pfacc       mm2,mm2
        movd        [eax+12],mm2
        femms
    }
}



//;******************************************************************************
//; Routine:  void _axis_angle2quat (D3DRMQUATERNION *quat, const float *axis)
//; Input:    quat - output quaternion
//;           axis - axis address (1x4 of <x,y,z> and w angle in degrees)
//; Output:   quat = rotation around given axis
//; Uses:     eax, ecx, edx, mm0-mm7
//;******************************************************************************
void _axis_angle2quat (D3DRMQUATERNION *quat, const float *axis)
{
    __asm
    {
        femms
        mov         eax,[axis]
        movd        mm0,[eax+12]
        pfmul       mm0,[HALFS]
        call        a_sincos
        mov         eax,[quat]
        movq        mm1,mm0
        punpckldq   mm0,mm0
        mov         edx,[axis]
        movq        mm3,[edx]
        punpckhdq   mm1,mm1
        movd        mm2,[edx+8]
        movd        [eax],mm1       //; stores sin(angle) in w
        punpckldq   mm2,mm2
        pfmul       mm3,mm0
        pfmul       mm2,mm0
        punpckhdq   mm2,mm3
        movd        [eax+4],mm3
        movq        [eax+8],mm2
        femms
    }
}


//;******************************************************************************
//; Routine:  _quat2axis_angle (float *axis, const D3DRMQUATERNION *quat)
//; Input:    axis    - resultant float[4] <x,y,z,angle>
//;           quat    - input quaternion
//; Output:   axisAngle representation of 'quat'
//; Uses:     eax, ecx, edx, mm0-mm7
//;******************************************************************************
void _quat2axis_angle (float *axis, const D3DRMQUATERNION *quat)
{
    __asm
    {
        femms
        mov         eax,[quat]
        movd        mm0,[eax]
        call        a_acos
        mov         eax,[axis]
        movq        mm1,mm0

        //; axisAngle[3] = tw * 2.0f * RAD2DEG;
        pfmul       mm1,QWORD PTR [RAD2DEG2]
        movd        [eax+12],mm1
        call        a_sin

        pfrcp       mm1,mm0
        pfrcpit1    mm0,mm1
        mov         eax,[quat]
        pfrcpit2    mm0,mm1             //; mm0 = scale
        movd        mm1,[eax+4]
        punpckldq   mm0,mm0
        movq        mm2,[eax+8]
        mov         edx,[axis]
        pfmul       mm1,mm0
        movd        [edx],mm1
        pfmul       mm2,mm0
        movq        [edx+8],mm2
        femms
    }
}



//;******************************************************************************
//; Routine:  void _slerp_quat (D3DRMQUATERNION *result,
//;                             const D3DRMQUATERNION *quat1, const D3DRMQUATERNION *quat2,
//;                             float slerp)
//; Input:    result - resultant quaternion
//;           q1     - input quaternion (slerp == 0.0)
//;           q2     - input quaternion (slerp == 1.0)
//;           slerp  - float interp factor
//; Output:   result = quaternion spherically interpolate between q1 and q2
//; Uses:     eax, ecx, edx, mm0-mm7
//;******************************************************************************
void _slerp_quat (D3DRMQUATERNION *result,
                  const D3DRMQUATERNION *q1, const D3DRMQUATERNION *q2,
                  float slerp)
{
    float omega, scale0;
    float isinom[2];

    __asm
    {
        femms
        //;cosom =    q1[0] * q2[0] +
        //;           q1[1] * q2[1] +
        //;           q1[2] * q2[2] +
        //;           q1[3] * q2[3];
        //; DOT the quats to get the cosine of the angle between them
        mov         eax,[q1]
        mov         edx,[q2]
        movq        mm0,[eax]
        pfmul       mm0,[edx]
        movq        mm2,[eax+8]
        movd        mm1,[DELTA_1]
        pfmul       mm2,[edx+8]
        pfadd       mm0,mm2
        pfacc       mm0,mm0
        //; mm0 = cosom

        //; Two special cases:
        //; if (cosom <= DELTA - 1.0f) do perpendiclar
        //; if (cosom >= 1.0f - DELTA) do linear
        pfcmpgt     mm1,mm0
        movd        mm2,[DELTA_2]   ; 1.0 - DELTA
        movd        eax,mm1
        pfcmpge     mm2,mm0
        test        eax,eax
        movd        eax,mm2
        jnz         do_perp
        test        eax,eax
        jz          do_linear

        //; otherwise, SLERP away!
        //; omega = (float)acos (cosom);
        call        a_acos                 //; cosom already im mm0
        movd        [omega],mm0

        //; isinom = 1.0f / (float)sin (omega);
        call        a_sin                  //; omega already im mm0
        pfrcp       mm1,mm0
        pfrcpit1    mm0,mm1
        pfrcpit2    mm0,mm1
        movq        [isinom],mm0            //; isinom = 1/sin(o) | 1/sin(o)

        //; scale0 = (float)sin ((1.0f - slerp) * omega) * isinom;
        //; scale1 = (float)sin (slerp * omega) * isinom;
        //; (this code is terribly sisd, but I'm can't puzzle out a better way)
        movd        mm0,dword ptr [ONES]
        movd        mm1,[slerp]
        pfsub       mm0,mm1
        movd        mm2,[omega]
        pfmul       mm0,mm2
        call        a_sin
        movd        [scale0],mm0
        movd        mm0,[slerp]
        movd        mm1,[omega]
        pfmul       mm0,mm1
        call        a_sin
        movd        mm1,[scale0]
        movq        mm2,[isinom]
        punpckldq   mm1,mm0
        pfmul       mm1,mm2
        movd        [scale0],mm1
        punpckhdq   mm1,mm1                 //; mm1 = scale1 | scale1

next:
        //; q2x = quat2[0] * scale1;
        //; q2y = quat2[1] * scale1;
        //; q2z = quat2[2] * scale1;
        //; q2w = quat2[3] * scale1;
        mov         eax,[q2]
        movq        mm2,mm1
        pfmul       mm1,[eax]
        pfmul       mm2,[eax+8]
        //; mm1 = q2x|q2y
        //; mm2 = q2z|q2w

slerp_me:
        //; result[0] = scale0 * quat1[0] + q2x;
        //; result[1] = scale0 * quat1[1] + q2y;
        //; result[2] = scale0 * quat1[2] + q2z;
        //; result[3] = scale0 * quat1[3] + q2w;
        mov         edx,[q1]
        movd        mm0,[scale0]
        movq        mm3,[edx]
        punpckldq   mm0,mm0
        movq        mm4,mm0
        movq        mm5,[edx+8]
        pfmul       mm0,mm3
        mov         eax,[result]
        pfmul       mm4,mm5
        pfadd       mm0,mm1
        pfadd       mm4,mm2
        movq        [eax],mm0
        movq        [eax+8],mm4
        femms
    }
    return;

    __asm
    {
//; Special cases, jump back up into main routine to return
//; Note that these cases rarely arise, so their re-entry points are not
//; aligned, as that would just take up space for no purpose.  We do align
//; their entry points, since it costs us nothing
        ALIGN       16
do_linear:
        //; Set scale0, scale1 for linear interpolation
        //; scale0 = 1.0f - slerp;
        //; scale1 = slerp;
        movd        mm1,[slerp]
        movd        mm0,dword ptr [ONES]
        pfsub       mm0,mm1                 //; mm0 = -slerp | 1 - slerp
        punpckldq   mm1,mm1                 //; mm1 = scale1 = slerp | slerp
        movd        [scale0],mm0            //; store 1 - slerp
        jmp         next                    //; perform the interpolation

        ALIGN       16
do_perp:
        //; set scale0, scale1 for slerp to a perpendicular quat
        //; scale0 = _sin ((1.0f - slerp) * HALF_PI);
        movd        mm1,dword ptr [ONES]
        movd        mm2,[slerp]
        movd        mm0,[HALF_PI]
        pfsub       mm1,mm2                 //; mm1 = 1 - slerp | -slerp
        pfmul       mm0,mm1                 //; mm0 = 1/2 * PI (1 - slerp)
        call        a_sin                  //; mm0 = sin (1/2 * PI (1 - slerp))
        movd        [scale0],mm0            //; save mm0 in scale0

        //; scale1 = _sin (slerp * HALF_PI);
        movd        mm1,[HALF_PI]
        movd        mm0,[slerp]
        pfmul       mm0,mm1                 //; mm0 = 1/2 * PI * slerp
        call        a_sin                  //; mm0 = sin (1/2 * PI * slerp)
        mov         eax,[q2]            //; eax = &q2
        punpckldq   mm0,mm0                 //; mm0 = scale1 | scale1

        //; q2w = quat2[3] *  scale1
        //; q2x = quat2[2] * -scale1
        //; q2y = quat2[1] *  scale1
        //; q2z = quat2[0] * -scale1
        movq        mm1,[eax+8]
        movq        mm2,[eax]
        movq        mm4,mm1
        movq        mm2,mm3
        punpckldq   mm1,mm1
        punpckldq   mm2,mm2
        punpckhdq   mm1,mm4
        punpckhdq   mm2,mm3
        pfmul       mm1,mm0                 //; mm1 = q2w|q2x
        pfmul       mm2,mm0                 //; mm2 = q2y|q2z

        //; mm1 = q2w|q2x
        //; mm2 = q2y|q2z
        jmp         slerp_me
    }
}


//;******************************************************************************
//; Routine:  void _trans_quat (D3DVECTOR *d, const D3DVECTOR *a, const D3DRMQUATERNION *b)
//; Input:    d - translated vector
//;           a - input vector
//;           b - rotation quaternion
//; Output:   d = 'a' rotated with 'b'
//; Comments: Note that this is equivalent to using quat2mat to make a rotation
//;           matrix, and then multiplying the vector by the matrix.  This form
//;           is more compact, and equally efficient when only transforming a
//;           single vector.  For other cases, it is advisable to construct
//;           a rotation matrix.
//; Uses:     eax,edx,mm0-mm7
//;******************************************************************************
void _trans_quat (D3DVECTOR *d, const D3DVECTOR *a, const D3DRMQUATERNION *b)
{
    __asm
    {
        femms
        mov         eax,[b]         //; eax = quat
        mov         edx,[a]         //; edx = vect

        movq        mm0,[eax]       //; mm0 = qx|qw

        movq        mm1,[eax+8]     //; mm1 = qz|qy
        movd        mm5,[edx]       //; mm5 = 0 |vx
        movq        mm2,mm0         //; mm2 = qx|qw
        movq        mm3,mm1         //; mm3 = qz|qy
        pfmul       mm2,mm2         //; mm2 = qx*qx|qw*qw

        punpckhdq   mm0,mm0         //; mm0 = qx|qx
        pfmul       mm3,mm3         //; mm3 = qz*qz|qy*qy

        movq        mm4,mm2         //; mm4 = =qx*qx|qw*qw
        pfmul       mm0,mm5         //; mm0 = 0|qx*vx

        pfacc       mm3,mm3         //; mm3 = qz*qz+qy*qy|qz*qz+qy*qy
        punpckhdq   mm4,mm4         //; mm4 = ???|qx*qx

        movq        mm6,[edx+4]     //; mm6 = vz|vy
        pfsub       mm2,mm4         //; mm2 = ???|qw*qw - qx*qx
                                         
        pfsub       mm2,mm3         //; mm2 = ???|dot_prod1
        movq        mm7,mm6         //; mm7 = vz|vy
                                         
        pfmul       mm5,mm2         //; mm5 = ???|vx*dot_prod1(dst.x)
        punpckldq   mm2,mm2         //; mm2 = dot_prod1|dot_prod1

        pfmul       mm2,mm6         //; mm2 = vz*dot_prod1(dst.z)|vy*dot_prod1(dst.y)

        pfmul       mm6,mm1         //; mm6 = qz * vz|qy * vy
        movq        mm3,[TWOS]

        movq        mm4,mm1         //; mm1 = qz|qy
        pfacc       mm6,mm6         //; mm6 = ???|qz * vz+qy * vy

        punpckhdq   mm4,mm4         //; mm1 = qz|qz

        pfadd       mm6,mm0         //; mm6 = ???|qz * vz+qy * vy+qx*vx
        movd        mm0,[eax+4]     //; mm0 = ???|qx

        punpckldq   mm4,mm1         //; mm4 = qy|qz

        pfmul       mm6,mm3         //; mm6 = ???|dot_prod2

        pfmul       mm4,mm7         //; mm4 = qy*vz|qz*vy
        movd        mm7,DWORD PTR [TWOS]

        pfmul       mm0,mm6         //; mm0 = ???|qx * dot_prod2
        punpckldq   mm6,mm6         //; mm6 = dot_prod2|dot_prod2

        pfmul       mm6,mm1         //; mm6 = qz*dot_prod2|qy*dot_prod2
        movq        mm3,mm4         //; mm3 = qy*vz|qz*vy

        pfadd       mm5,mm0         //; mm5 = ???dst.x
        punpckhdq   mm3,mm3         //; mm3 = qy*vz|qy*vz

        pfadd       mm2,mm6         //; mm2 = dst.z|dst.y
        movd        mm0,[eax]       //; mm0 = ???|qw

        movd        mm6,[eax+4]     //; mm6 = ???|qx
        pfsub       mm3,mm4         //; mm3 = ???|qy * vz-qz * vy

        pfmul       mm7,mm0         //; mm7 = ???|qw * 2.0
        punpckldq   mm6,mm1         //; mm6 = qy|qx

        mov         eax,[d]
        punpckhdq   mm1,mm1         //; mm1 = qz|qz

        pfmul       mm3,mm7         //; mm3 = ???|(qy * vz - qz * vy)* qw * 2.0
        movq        mm0,[edx]       //; mm0 = vy|vx

        punpckldq   mm1,mm6         //; mm1 = qx|qz

        pfadd       mm5,mm3         //; mm5 = ???|dst.x
        movd        mm3,[edx+8]     //; mm3 = ???|vz

        pfmul       mm1,mm0         //; mm1 = qx * vy|qz * vx
        punpckldq   mm3,mm0         //; mm3 = vx|vz

        punpckldq   mm7,mm7         //; mm7 = qw * 2.0|qw * 2.0
        pfmul       mm6,mm3         //; mm6 = qy*vx|qx*vz

        pfsub       mm1,mm6         //; mm1 = qx * vy-qy*vx|qz * vx-qx*vz
        movd        [eax],mm5       //; mm5 = ???|dst.x
        pfmul       mm1,mm7
        pfadd       mm2,mm1         //; mm2 = dst.z|dst.y
        movq        [eax+4],mm2     //; mm2 = dst.z|dst.y
        femms
    }
}
