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
//; MATRX.ASM
//;
//; AMD3D 3D library code: matrix math primitives
//;
//; Direct3D types are used for all 4x4 matrix operations for better type
//; compatability with game programs.  The 3x3 matrices use (float *) parameters,
//; which are assumed to point to appropriate values.
//;
//;******************************************************************************

#include "amatrix.h"

// D3D Matrix offsets
#define a_11 0
#define a_12 1*4
#define a_13 2*4
#define a_14 3*4
#define a_21 4*4
#define a_22 5*4
#define a_23 6*4
#define a_24 7*4
#define a_31 8*4
#define a_32 9*4
#define a_33 10*4
#define a_34 11*4
#define a_41 12*4
#define a_42 13*4
#define a_43 14*4
#define a_44 15*4

//;******************************************************************************
//; Routine:  void _glMul_3x3 (float *r, const float *a, const float *b)
//; Input:    r - matrix (3x3) address
//;           a - matrix (3x3) address
//;           b - matrix (3x3) address
//; Output:   r = a * b, using standard matrix multiplication rules
//; Uses:     eax, ecx, edx, mm0 - mm7
//;******************************************************************************
void _glMul_3x3 (float *r, const float *a, const float *b)
{
    __asm
    {
        femms
        mov         edx,[b]
        mov         eax,[r]
        mov         ecx,[a]

        movd        mm7,[edx+8]		//; 0    | b_31
        movd        mm6,[edx+32]	//; 0    | b_33
        punpckldq   mm7,[edx+20]	//; b_31 | b_31
        movq        mm0,[ecx]		//; a_21 | a_11
        movd        mm3,[ecx+8]		//; 0    | a_31
        movq        mm1,mm0			//; a_21 | a_11
        pfmul       mm0,mm7			//; a_21 * b_31 | a_11 * b_31
        movq        mm2,mm1			//; a_21 | a_11
        punpckldq   mm1,mm1			//; a_11 | a_11
        pfmul       mm1,[edx]		//; a_11 * b_21 | a_11 * b_11 
        punpckhdq   mm2,mm2			//; a_21 | a_21
        pfmul       mm2,[edx+12]	//; a_21 * b_22 | a_21 * b_12 
        pfacc       mm0,mm0			//; XXX  | a_21 * b_31 + a_11 * b_31
        movq        mm4,mm3			//; 0    | a_31
        punpckldq   mm3,mm3			//; a_31 | a_31
        pfmul       mm3,[edx+24]	//; a_31 * b_23 | a_31 * b_13
        pfadd       mm2,mm1			//; a_11 * b_21 + a_21 * b_22 | a_11 * b_11 + a_21 * b_12
        pfmul       mm4,mm6			//; 0    | a_31 * b_33
        movq        mm5,[ecx+12]	//; a_22 | a_12
        pfadd       mm2,mm3			//; a_31 * b_23 + a_11 * b_21 + a_21 * b_22 | a_31 * b_13 + a_11 * b_11 + a_21 * b_12
        movd        mm3,[ecx+20]	//; 0    | a_32
        pfadd       mm4,mm0			//; XXX  | a_31 * b_33 + a_21 * b_31 + a_11 * b_31
        movq        mm1,mm5			//; a_22 | a_12
        movq        [eax],mm2		//; r_21 | r_11
        pfmul       mm5,mm7			//; a_22 * b_31 | a_12 * b_31
        movd        [eax+8],mm4		//; 0    | r_31
        movq        mm2,mm1			//; a_22 | a_12
        punpckldq   mm1,mm1			//; a_12 | a_12
        movq        mm0,[ecx+24]	//; a_23 | a_13
        pfmul       mm1,[edx]		//; a_12 * b_21 | a_12 * b_11
        punpckhdq   mm2,mm2			//; a_22 | a_22
        pfmul       mm2,[edx+12]	//; a_22 * b_22 | a_22 * b_12
        pfacc       mm5,mm5			//; XXX  | a_22 * b_31 + a_12 * b_31
        movq        mm4,mm3			//; 0    | a_32
        punpckldq   mm3,mm3			//; a_32 | a_32
        pfmul       mm3,[edx+24]	//; a_32 b_23 | a_32 * b_13
        pfadd       mm2,mm1			//; a_22 * b_22 + a_12 * b_21 | a_22 * b_12 + a_12 * b_11
        pfmul       mm4,mm6			//; 0    | a_32 * b_33
        movq        mm1,mm0			//; a_23 | a_13
        pfadd       mm2,mm3			//; a_32 b_23 + a_22 * b_22 + a_12 * b_21 | a_32 * b_13 + a_22 * b_12 + a_12 * b_11
        movd        mm3,[ecx+32]	//; 0    | a_33
        pfadd       mm4,mm5			//; XXX  | a_32 * b_33 + a_22 * b_31 + a_12 * b_31
        pfmul       mm0,mm7			//; a_23 * b_31 | a_13 * b_31
        movq        [eax+12],mm2	//; r_22 | r_12
        movq        mm2,mm1			//; a_23 | a_13
        movd        [eax+20],mm4	//; 0    | r_32
        punpckldq   mm1,mm1			//; a_13 | a_13
        pfmul       mm1,[edx]		//; a_13 * b_21 | a_13 * b_11 
        punpckhdq   mm2,mm2			//; a_23 | a_23
        pfmul       mm2,[edx+12]	//; a_23 * b_22 | a_23 * b_12
        pfacc       mm0,mm0			//; XXX  | a_23 * b_31 + a_13 * b_31
        pfmul       mm6,mm3			//;  0   | a_33 * b_33
        punpckldq   mm3,mm3			//; a_33 | a_33
        pfmul       mm3,[edx+24]	//; a_33 * b_23 | a_33 * b_13
        pfadd       mm2,mm1			//; a_13 * b_21 + a_23 * b_22 | a_13 * b_11 + a_23 * b_12
        pfadd       mm6,mm0			//;  0   | a_33 * b_33 + a_23 * b_31 + a_13 * b_31
        pfadd       mm2,mm3			//;  a_33 * b_23 + a_13 * b_21 + a_23 * b_22 | a_33 * b_13 + a_13 * b_11 + a_23 * b_12
        movd        [eax+32],mm6	//; 0    | r_33
        movq        [eax+24],mm2	//; r_23 | r_13
        femms
    }
}


//;******************************************************************************
//; Routine:  void _glMul_4x4 (float *r, const float *a, const float *b)
//; Input:    r - matrix (4x4) address
//;           a - matrix (4x4) address
//;           b - matrix (4x4) address
//; Output:   r = a * b, using standard matrix multiplication rules
//; Uses:     eax, ecx, edx, mm0 - mm7
//;******************************************************************************
void _glMul_4x4 (float *r, const float *a, const float *b)
{
    __asm
    {
        femms
        mov         ecx,[a]
        mov         eax,[r]
        mov         edx,[b]

        movq        mm0,[ecx]		//; a_21	| a_11
        movq        mm1,[ecx+8]		//; a_41	| a_31
        movq        mm4,[edx]		//; b_21	| b_11
        punpckhdq   mm2,mm0			//; a_21	| 
        movq        mm5,[edx+16]	//; b_22	| b_12
        punpckhdq   mm3,mm1			//; a_41	| 
        movq        mm6,[edx+32]	//; b_23	| b_13
        punpckldq   mm0,mm0			//; a_11	| a_11
        punpckldq   mm1,mm1			//; a_31	| a_31
        pfmul       mm4,mm0			//; a_11 * b_21 | a_11 * b_11
        punpckhdq   mm2,mm2			//; a_21	| a_21
        pfmul       mm0,[edx+8]		//; a_11 * b_41 | a_11 * b_31
        movq        mm7,[edx+48]	//; b_24	| b_14
        pfmul       mm5,mm2			//; a_21 * b_22 | a_21 * b_12
        punpckhdq   mm3,mm3			//; a_41	| a_41
        pfmul       mm2,[edx+24]	//; a_21 * b_42 | a_21 * b_32
        pfmul       mm6,mm1			//; a_31 * b_23 | a_31 * b_13 
        pfadd       mm5,mm4			//; a_21 * b_22 + a_11 * b_21 | a_21 * b_12 + a_11 * b_11
        pfmul       mm1,[edx+40]	//; a_31 * b_43 | a_31 * b_33
        pfadd       mm2,mm0			//; a_21 * b_42 + a_11 * b_41 | a_21 * b_32 + a_11 * b_31
        pfmul       mm7,mm3			//; a_41 * b_24 | a_41 * b_14 
        pfadd       mm6,mm5			//; a_21 * b_22 + a_11 * b_21 + a_31 * b_23 | a_21 * b_12 + a_11 * b_11 + a_31 * b_13
        pfmul       mm3,[edx+56]	//; a_41 * b_44 | a_41 * b_34
        pfadd       mm2,mm1			//; a_21 * b_42 + a_11 * b_41 + a_31 * b_43 | a_21 * b_32 + a_11 * b_31 + a_31 * b_33 
        pfadd       mm7,mm6			//; a_41 * b_24 + a_21 * b_22 + a_11 * b_21 + a_31 * b_23 |  a_41 * b_14 + a_21 * b_12 + a_11 * b_11 + a_31 * b_13
        movq        mm0,[ecx+16]	//; a_22	| a_12
        pfadd       mm3,mm2			//; a_41 * b_44 + a_21 * b_42 + a_11 * b_41 + a_31 * b_43 | a_41 * b_34 + a_21 * b_32 + a_11 * b_31 + a_31 * b_33 
        movq        mm1,[ecx+24]	//; a_42	| a_32
        movq        [eax],mm7		//; r_21	| r_11 
        movq        mm4,[edx]		//; b_21	| b_11
        movq        [eax+8],mm3		//; r_41	| r_31

        punpckhdq   mm2,mm0			//; a_22	| XXX
        movq        mm5,[edx+16]	//; b_22	| b_12
        punpckhdq   mm3,mm1			//; a_42	| XXX
        movq        mm6,[edx+32]	//; b_23	| b_13
        punpckldq   mm0,mm0			//; a_12	| a_12
        punpckldq   mm1,mm1			//; a_32	| a_32
        pfmul       mm4,mm0			//; a_12 * b_21 | a_12 * b_11
        punpckhdq   mm2,mm2			//; a_22	| a_22
        pfmul       mm0,[edx+8]		//; a_12 * b_41 | a_12 * b_31
        movq        mm7,[edx+48]	//; b_24	| b_14
        pfmul       mm5,mm2			//; a_22 * b_22 | a_22 * b12
        punpckhdq   mm3,mm3			//; a_42	| a_42
        pfmul       mm2,[edx+24]	//; a_22 * b_42 | a_22 * b_32
        pfmul       mm6,mm1			//; a_32 * b_23 | a_32 * b_13
        pfadd       mm5,mm4			//; a_12 * b_21 + a_22 * b_22 | a_12 * b_11 + a_22 * b12
        pfmul       mm1,[edx+40]	//; a_32 * b_43 | a_32 * b_33
        pfadd       mm2,mm0			//; a_12 * b_41 + a_22 * b_42 | a_12 * b_11 + a_22 * b_32
        pfmul       mm7,mm3			//; a_42 * b_24 | a_42 * b_14
        pfadd       mm6,mm5			//; a_32 * b_23 + a_12 * b_21 + a_22 * b_22 | a_32 * b_13 + a_12 * b_11 + a_22 * b12
        pfmul       mm3,[edx+56]	//; a_42 * b_44 | a_42 * b_34
        pfadd       mm2,mm1			//; a_32 * b_43 + a_12 * b_41 + a_22 * b_42 | a_32 * b_33 + a_12 * b_11 + a_22 * b_32
        pfadd       mm7,mm6			//; a_42 * b_24 + a_32 * b_23 + a_12 * b_21 + a_22 * b_22 | a_42 * b_14 + a_32 * b_13 + a_12 * b_11 + a_22 * b12
        movq        mm0,[ecx+32]	//; a_23 | a_13
        pfadd       mm3,mm2			//; a_42 * b_44 + a_32 * b_43 + a_12 * b_41 + a_22 * b_42 | a_42 * b_34 + a_32 * b_33 + a_12 * b_11 + a_22 * b_32
        movq        mm1,[ecx+40]	//; a_43 | a_33
        movq        [eax+16],mm7	//; r_22 | r_12
        movq        mm4,[edx]		//; b_21	| b_11
        movq        [eax+24],mm3	//; r_42 | r_32

        punpckhdq   mm2,mm0			//; a_23 | XXX
        movq        mm5,[edx+16]	//; b_22 | b_12
        punpckhdq   mm3,mm1			//; a_43 | XXX
        movq        mm6,[edx+32]	//; b_23 | b_13
        punpckldq   mm0,mm0			//; a_13 | a_13
        punpckldq   mm1,mm1			//; a_33 | a_33
        pfmul       mm4,mm0			//; a_13 * b_21 | a_13 * b_11
        punpckhdq   mm2,mm2			//; a_23 | a_23
        pfmul       mm0,[edx+8]		//; a_13 * b_41 | a_13 * b_31
        movq        mm7,[edx+48]	//; b_24 | b_14
        pfmul       mm5,mm2			//; a_23 * b_22 | a_23 * b_12
        punpckhdq   mm3,mm3			//; a_43 | a_43
        pfmul       mm2,[edx+24]	//; a_23 * b_42 | a_23 * b_32
        pfmul       mm6,mm1			//; a_33 * b_23 | a_33 * b_13
        pfadd       mm5,mm4			//; a_23 * b_22 + a_13 * b_21 | a_23 * b_12 + a_13 * b_11
        pfmul       mm1,[edx+40]	//; a_33 * b_43 | a_33 * b_33 
        pfadd       mm2,mm0			//; a_13 * b_41 + a_23 * b_42 | a_13 * b_31 + a_23 * b_32
        pfmul       mm7,mm3			//; a_43 * b_24 | a_43 * b_14
        pfadd       mm6,mm5			//; a_33 * b_23 + a_23 * b_22 + a_13 * b_21 | a_33 * b_13 + a_23 * b_12 + a_13 * b_11
        pfmul       mm3,[edx+56]	//; a_43 * b_44 | a_43 * b_34
        pfadd       mm2,mm1			//; a_33 * b_43 * a_13 * b_41 + a_23 * b_42 | a_33 * b_33 + a_13 * b_31 + a_23 * b_32
        pfadd       mm7,mm6			//; a_43 * b_24 + a_33 * b_23 + a_23 * b_22 + a_13 * b_21 | a_43 * b_14 + a_33 * b_13 + a_23 * b_12 + a_13 * b_11
        movq        mm0,[ecx+48]	//; a_24 | a_14
        pfadd       mm3,mm2			//; a_43 * b_44 + a_33 * b_43 * a_13 * b_41 + a_23 * b_42 | a_43 * b_34 + a_33 * b_33 + a_13 * b_31 + a_23 * b_32
        movq        mm1,[ecx+56]	//; a_44 | a_34
        movq        [eax+32],mm7	//; r_23 | r_13
        movq        mm4,[edx]		//; b_21 | b_11
        movq        [eax+40],mm3	//; r_43 | r_33

        punpckhdq   mm2,mm0			//; a_24 | XXX
        movq        mm5,[edx+16]	//; b_22 | b_12
        punpckhdq   mm3,mm1			//; a_44 | XXX
        movq        mm6,[edx+32]	//; b_23 | b_13
        punpckldq   mm0,mm0			//; a_14 | a_14
        punpckldq   mm1,mm1			//; a_34 | a_34
        pfmul       mm4,mm0			//; a_14 * b_21 | a_14 * b_11
        punpckhdq   mm2,mm2			//; a_24 | a_24
        pfmul       mm0,[edx+8]		//; a_14 * b_41 | a_14 * b_31
        movq        mm7,[edx+48]	//; b_24 | b_14
        pfmul       mm5,mm2			//; a_24 * b_22 | a_24 * b_12
        punpckhdq   mm3,mm3			//; a_44 | a_44
        pfmul       mm2,[edx+24]	//; a_24 * b_ 42 | a_24 * b_32
        pfmul       mm6,mm1			//; a_34 * b_23 | a_34 * b_13
        pfadd       mm5,mm4			//; a_14 * b_21 + a_24 * b_22 | a_14 * b_11 + a_24 * b_12
        pfmul       mm1,[edx+40]	//; a_34 * b_43 | a_34 * b_33
        pfadd       mm2,mm0			//; a_14 * b_41 + a_24 * b_ 42 | a_14 * b_31 + a_24 * b_32
        pfmul       mm7,mm3			//; a_44 * b_24 | a_44 * b_14
        pfadd       mm6,mm5			//; a_34 * b_23 + a_14 * b_21 + a_24 * b_22 | a_34 * b_13 + a_14 * b_11 + a_24 * b_12
        pfmul       mm3,[edx+56]	//; a_44 * b_44 | a_44 * b_34
        pfadd       mm2,mm1			//; a_34 * b_43 + a_14 * b_41 + a_24 * b_ 42 | a_34 * b_33 + a_14 * b_31 + a_24 * b_32
        pfadd       mm7,mm6			//; a_44 * b_24 + a_14 * b_23 + a_24 * b_ 42 | a_44 * b_14 + a_14 * b_31 + a_24 * b_32
        pfadd       mm3,mm2			//; a_44 * b_44 + a_34 * b_43 + a_14 * b_41 + a_24 * b_42 | a_44 * b_34 + a_34 * b_33 + a_14 * b_31 + a_24 * b_32
        movq        [eax+48],mm7	//; r_24 | r_14
        movq        [eax+56],mm3	//; r_44 | r_34
        femms
    }
}


//;*********************************************************************
//; VOID _D3DMul_4x4 (D3DMATRIX &q, D3DMATRIX& a, D3DMATRIX& b)
//; REPLACES D3DMath_MatrixMultiply IN DIRECTX 6.0 D3DFRAME.LIB
//; MULTIPLY 2 - 4 x 4 MATRICES
//; LOCAL STACK USED FOR TEMPS TO PREVENT OVERWRITE IN CASE
//; WHERE SOURCE AND DEST WERE SAME MATRIX
//;*********************************************************************
void _D3DMul_4x4 (D3DMATRIX &r, const D3DMATRIX &a, const D3DMATRIX &b)
{
    D3DMATRIX tmp;
    __asm
    {
        femms
        mov         eax,[a]			//;source a
        mov         ecx,[b]			//;source b
        mov         edx,[r]			//;result r

        sub         esp,64				//;T_ local work space to store temp results

        movd        mm0,[eax + a_21]    //;       | a_21
        movd        mm1,[eax + a_11]    //;       | a_11
        movd        mm6,[eax + a_12]    //;       | a_12
        punpckldq   mm1,mm0             //; a_21  | a_11  
        movd        mm5,[eax + a_22]    //;       | a_22
        pfmul       mm1,[ecx]           //; a_21 * b_12 | a_11 * b_11     
        punpckldq   mm6,mm5             //; a_22  | a_12      
        movd        mm7,[eax + a_32]    //;       | a_32
        movd        mm5,[eax + a_42]    //;       | a_42
        pfmul       mm6,[ecx]           //; a_22 * b_12 | a_12 * b_11     
        movd        mm2,[eax + a_31]    //;       | a_31
        punpckldq   mm7,mm5             //; a_42  | a_32
        movd        mm0,[eax + a_41]    //;       | a_41
        pfmul       mm7,[ecx+8]         //; a_42 * b_14 | a_32 * b13
        punpckldq   mm2,mm0             //; a_41  | a_31
        pfadd       mm6,mm7				//; a_42 * b_14 + a_22 * b_12 | a_32 * b13 + a_12 * b_11
        pfmul       mm2,[ecx+8]         //; a_41 * b_14 | a_31 * b13
        pfacc       mm6,mm6				//;		| a_12 * b_11 + a_22 * b_12 + a_32 * b_13 + a_42 * b_14  
        pfadd       mm1,mm2				//; a_21 * b_12 + a_41 * b_14 | a_11 * b_11 + a_31 * b13
        movd        [tmp+4],mm6         //; T_12   
        pfacc       mm1,mm1				//;       |  a_21 * b_12 + a_41 * b_14 + a_11 * b_11 + a_31 * b13
        movd        [tmp],mm1           //; T_11

        movd        mm0,[eax + a_23]    //;       | a_23
        movd        mm1,[eax + a_13]    //;       | a_13
        movd        mm6,[eax + a_14]    //;       | a_14
        punpckldq   mm1,mm0             //; a_23  | a_13  
        movd        mm5,[eax + a_24]    //;       | a_24
        pfmul       mm1,[ecx]           //; a_23 * b_12 | a_13 * b_11     
        punpckldq   mm6,mm5             //; a_24  | a_14      
        movd        mm7,[eax + a_34]    //;       | a_34
        movd        mm5,[eax + a_44]    //;       | a_44
        pfmul       mm6,[ecx]           //; a_24 * b_12 | a_14 * b_11     
        movd        mm2,[eax + a_33]    //;       | a_33
        punpckldq   mm7,mm5             //; a_44  | a_34
        movd        mm0,[eax + a_43]    //;       | a_43
        pfmul       mm7,[ecx+8]         //; a_44 * b_14 | a_34 * b_13
        punpckldq   mm2,mm0             //; a_43  | a_33
        pfadd       mm6,mm7				//; a_44 * b_14 + a_24 * b_12 | a_34 * b_13 + a_14 * b_11
        pfmul       mm2,[ecx+8]         //; a_43 * b_12 | a_33 * b11
        pfacc       mm6,mm6				//;		| a_44 * b_14 + a_24 * b_12 + a_34 * b_13 + a_14 * b_11
        pfadd       mm1,mm2				//; a_43 * b_12 + a_23 * b_12 | a_33 * b11 + a_13 * b_11
        movd        [tmp+12],mm6		//; T_14
        pfacc       mm1,mm1				//;		| a_43 * b_12 + a_23 * b_12 + a_33 * b11 + a_13 * b_11
        movd        [tmp+8],mm1			//; T_13

        movd        mm0,[eax + a_21]    //;       | a_21
        movd        mm1,[eax + a_11]    //;       | a_11
        movd        mm6,[eax + a_12]    //;       | a_12
        punpckldq   mm1,mm0             //; a_21  | a_11  
        movd        mm5,[eax + a_22]    //;       | a_22
        pfmul       mm1,[ecx+16]        //; a_21 * b_22 | a_11 * b_21     
        punpckldq   mm6,mm5             //; a_22  | a_12      
        movd        mm7,[eax + a_32]    //;       | a_32
        movd        mm5,[eax + a_42]    //;       | a_42
        pfmul       mm6,[ecx+16]        //; a_22 * b_22 | a_12 * b_21     
        movd        mm2,[eax + a_31]    //;       | a_31
        punpckldq   mm7,mm5             //; a_42  | a_32
        movd        mm0,[eax + a_41]    //;       | a_41
        pfmul       mm7,[ecx+24]        //; a_42 * b_24 | a_32 * b_23
		punpckldq   mm2,mm0             //; a_41  | a_31
        pfadd       mm6,mm7				//; a_42 * b_24 + a_22 * b_22 | a_32 * b_23 + a_12 * b_21
        pfmul       mm2,[ecx+24]        //; a_41 * b_24 | a_31 * b_23
        pfacc       mm6,mm6				//;       | a_42 * b_24 + a_22 * b_22 + a_32 * b_23 + a_12 * b_21
        pfadd       mm1,mm2				//; a_41 * b_24 + a_21 * b_22 | a_31 * b_23 + a_11 * b_21
        movd        [tmp+20],mm6		//; T_22
        pfacc       mm1,mm1				//;		|a_41 * b_24 + a_21 * b_22 + a_31 * b_23 + a_11 * b_21
        movd        [tmp+16],mm1		//; T_21

        movd        mm0,[eax + a_23]    //;       | a_23
        movd        mm1,[eax + a_13]    //;       | a_13
        movd        mm6,[eax + a_14]    //;       | a_14
        punpckldq   mm1,mm0             //; a_23  | a_13  
        movd        mm5,[eax + a_24]    //;       | a_24
        pfmul       mm1,[ecx+16]        //; a_23 * b_22 | a_13 * b_21 
        punpckldq   mm6,mm5             //; a_24  | a_14      
        movd        mm7,[eax + a_34]    //;       | a_34
        movd        mm5,[eax + a_44]    //;       | a_44
        pfmul       mm6,[ecx+16]        //; a_24 * b_22 | a_14 * b_21     
        movd        mm2,[eax + a_33]    //;       | a_33
        punpckldq   mm7,mm5             //; a_44  | a_34
        movd        mm0,[eax + a_43]    //;       | a_43
        pfmul       mm7,[ecx+24]        //; a_44 * b_24 | a_34 * b_23
        punpckldq   mm2,mm0             //; a_43  | a_33
        pfadd       mm6,mm7				//; a_24 * b_22 + a_44 * b_24 | a_14 * b_21 + a_34 * b_23
        pfmul       mm2,[ecx+24]        //; a_43 * b_24 | a_33 * b_23
        pfacc       mm6,mm6				//;		|a_24 * b_22 + a_44 * b_24 + a_14 * b_21 + a_34 * b_23
        pfadd       mm1,mm2				//; a_43 * b_24 + a_23 * b_22 | a_33 * b_23 + a_14 * b_21
        movd        [tmp+28],mm6		//; T_24
        pfacc       mm1,mm1				//;		| a_43 * b_24 + a_23 * b_22 + a_33 * b_23 + a_14 * b_21
        movd        [tmp+24],mm1		//; T_23

        movd        mm0,[eax + a_21]    //;       | a_21
        movd        mm1,[eax + a_11]    //;       | a_11
        movd        mm6,[eax + a_12]    //;       | a_12
        punpckldq   mm1,mm0             //; a_21  | a_11  
        movd        mm5,[eax + a_22]    //;       | a_22
        pfmul       mm1,[ecx+32]        //; a_21 * b_32 | a_11 * b_31     
        punpckldq   mm6,mm5             //; a_22  | a_12      
        movd        mm7,[eax + a_32]    //;       | a_32
        movd        mm5,[eax + a_42]    //;       | a_42
        pfmul       mm6,[ecx+32]        //; a_22 * b_32 | a_12 * b_31 
        movd        mm2,[eax + a_31]    //;       | a_31
        punpckldq   mm7,mm5             //; a_42  | a_32
        movd        mm0,[eax + a_41]    //;       | a_41
        pfmul       mm7,[ecx+40]        //; a_42 * b_34 | a_32 * b33
        punpckldq   mm2,mm0             //; a_41  | a_31
        pfadd       mm6,mm7				//; a_42 * b_34 + a_22 * b_32 | a_32 * b33 + a_12 * b_31 
        pfmul       mm2,[ecx+40]        //; a_41 * b_34 | a_31 * b33
        pfacc       mm6,mm6				//;		|a_42 * b_34 + a_22 * b_32 + a_32 * b33 + a_12 * b_31 
        pfadd       mm1,mm2				//; a_41 * b_34 + a_21 * b_32 | a_31 * b33 + a_11 * b_31
        movd        [tmp+36],mm6		//; T_32
        pfacc       mm1,mm1				//;		|a_41 * b_34 + a_21 * b_32 + a_31 * b33 + a_11 * b_31
        movd        [tmp+32],mm1		//; T_31

        movd        mm0,[eax + a_23]    //;       | a_23
        movd        mm1,[eax + a_13]    //;       | a_13
        movd        mm6,[eax + a_14]    //;       | a_14
        punpckldq   mm1,mm0             //; a_23  | a_13  
        movd        mm5,[eax + a_24]    //;       | a_24
        pfmul       mm1,[ecx+32]        //; a_21 * b_32 | a_11 * b_31     
        punpckldq   mm6,mm5             //; a_22  | a_12      
        movd        mm7,[eax + a_34]    //;       | a_34
        movd        mm5,[eax + a_44]    //;       | a_44
        pfmul       mm6,[ecx+32]        //; a_22 * b_32 | a_12 * b_31     
        movd        mm2,[eax + a_33]    //;       | a_33
        punpckldq   mm7,mm5             //; a_42  | a_32
        movd        mm0,[eax + a_43]    //;       | a_43
        pfmul       mm7,[ecx+40]        //; a_42 * b_34 | a_32 * b_33
        punpckldq   mm2,mm0             //; a_43  | a_33
        pfadd       mm6,mm7				//; a_42 * b_34 + a_22 * b_32 | a_32 * b_33 + a_12 * b_31
        pfmul       mm2,[ecx+40]        //; a_41 * b_34 | a_31 * b_33
        pfacc       mm6,mm6				//;		|a_42 * b_34 + a_22 * b_32 + a_32 * b_33 + a_12 * b_31
        pfadd       mm1,mm2				//; a_41 * b_34 + a_21 * b_32 | a_31 * b_33 + a_11 * b_31
        movd        [tmp+44],mm6		//; T_34
        pfacc       mm1,mm1				//;		|a_41 * b_34 + a_21 * b_32 + a_31 * b_33 + a_11 * b_31
        movd        [tmp+40],mm1		//; T_33

        movd        mm0,[eax + a_21]    //;       | a_21
        movd        mm1,[eax + a_11]    //;       | a_11
        movd        mm6,[eax + a_12]    //;       | a_12
        punpckldq   mm1,mm0             //; a_21  | a_11  
        movd        mm5,[eax + a_22]    //;       | a_22
        pfmul       mm1,[ecx+48]        //; a_21 * b_42 | a_11 * b_41     
        punpckldq   mm6,mm5             //; a_22  | a_12      
        movd        mm7,[eax + a_32]    //;       | a_32
        movd        mm5,[eax + a_42]    //;       | a_42
        pfmul       mm6,[ecx+48]        //; a_22 * b_42 | a_12 * b_41     
        movd        mm2,[eax + a_31]    //;       | a_31
        punpckldq   mm7,mm5             //; a_42  | a_32
        movd        mm0,[eax + a_41]    //;       | a_41
        pfmul       mm7,[ecx+56]        //; a_42 * b_44 | a_32 * b_43
        punpckldq   mm2,mm0             //; a_41  | a_31
        pfadd       mm6,mm7				//; a_42 * b_44 + a_22 * b_42 | a_32 * b_43 + a_12 * b_41
        pfmul       mm2,[ecx+56]        //; a_41 * b_44 | a_31 * b_43
        pfacc       mm6,mm6				//;		|a_42 * b_44 + a_22 * b_42 + a_32 * b_43 + a_12 * b_41
        pfadd       mm1,mm2				//; a_41 * b_44 + a_21 * b_42 | a_31 * b_43 + a_11 * b_41
        movd        [tmp+52],mm6		//; T_42
        pfacc       mm1,mm1				//;		| a_41 * b_44 + a_21 * b_42 + a_31 * b_43 + a_11 * b_41
        movd        [tmp+48],mm1		//; T_41


        movd        mm0,[eax + a_23]    //;       | a_23
        movd        mm1,[eax + a_13]    //;       | a_13
        movd        mm6,[eax + a_14]    //;       | a_14
        punpckldq   mm1,mm0             //; a_23  | a_13  
        movd        mm5,[eax + a_24]    //;       | a_24
        pfmul       mm1,[ecx+48]        //; a_21 * b_42 | a_11 * b_41     
        punpckldq   mm6,mm5             //; a_22  | a_12      
        movd        mm7,[eax + a_34]    //;       | a_34
        movd        mm5,[eax + a_44]    //;       | a_44
        pfmul       mm6,[ecx+48]        //; a_22 * b_42 | a_12 * b_41     
        movd        mm2,[eax + a_33]    //;       | a_33
        punpckldq   mm7,mm5             //; a_42  | a_32
        movd        mm0,[eax + a_43]    //;       | a_43
        pfmul       mm7,[ecx+56]        //; a_42 * b_44 | a_32 * b_43
        punpckldq   mm2,mm0             //; a_43  | a_33
        pfadd       mm6,mm7				//; a_42 * b_44 + a_22 * b_42 | a_32 * b_43 + a_12 * b_41
        pfmul       mm2,[ecx+56]        //; a_41 * b_44 | a_31 * b_43
        pfacc       mm6,mm6				//;		|a_42 * b_44 + a_22 * b_42 + a_32 * b_43 + a_12 * b_41
        pfadd       mm1,mm2				//; a_41 * b_44 + a_21 * b_42 | a_31 * b_43 + a_11 * b_41 
        movd        [tmp+60],mm6		//; T_44
        pfacc       mm1,mm1				//; a_41 * b_44 + a_21 * b_42 + a_31 * b_43 + a_11 * b_41 
        movd        [tmp+56],mm1		//; T_43


        movq        mm3,[tmp]			//;MOVE FROM LOCAL TEMP MATRIX TO ADDRESS OF RESULT
        movq        mm4,[tmp+8]
        movq        [edx],mm3
        movq        [edx+8],mm4

        movq        mm3,[tmp+16]
        movq        mm4,[tmp+24]
        movq        [edx+16],mm3
        movq        [edx+24],mm4

        movq        mm3,[tmp+32]
        movq        mm4,[tmp+40]
        movq        [edx+32],mm3
        movq        [edx+40],mm4

        movq        mm3,[tmp+48]
        movq        mm4,[tmp+56]
        movq        [edx+48],mm3
        movq        [edx+56],mm4
        femms
    }
}

// eof