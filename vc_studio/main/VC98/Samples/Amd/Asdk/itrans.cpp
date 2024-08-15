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
//; TRANS.ASM
//;
//; AMD3D 3D library code: vector translation
//;
//; The 4x4 versions of these routines have been re-declared to use Direct3D
//; types for greater compatability with most game programs.  The 3x3 versions
//; have been retained, but with the float * interface.  D3D* functions use
//; Direct3D-style row major matrices, and gl* functions use OpenGL-style
//; column major matrices.
//;
//; There are no 4x3 matrix routines currently implemented.
//;
//;******************************************************************************

#include "atrans.h"

//;******************************************************************************
//; Routine:  void _D3DTrans_4x4 (D3DTLVERTEX &r, const D3DTLVERTEX &a,
//;                               const D3DMATRIX &m)
//; Input:    r - result vector (1x4)
//;           a - input  vector (1x4)
//;           m - matrix (4x4)
//; Output:   r = a * m (matrix multiplication rules)
//; Uses:     eax, ecx, edx, mm0 - mm7
//;******************************************************************************
void _D3DTrans_4x4 (D3DTLVERTEX &r, const D3DTLVERTEX &a, const D3DMATRIX &m)
{
    __asm
    {
        femms
        mov         eax,[r]		    //; result
        mov         edx,[m]		    //; matrix
        mov         ecx,[a]		    //; vertex

        movq        mm0,[ecx]		//;  y   |  x
        movq        mm1,[ecx+8]		//; rhw  |  z
        movq        mm4,[edx]		//; m_12 | m_11
        punpckhdq   mm2,mm0			//;  y   |  ?
        movq        mm5,[edx+16]	//; m_22 | m_21
        punpckldq   mm0,mm0			//;  x   |  x
        movq        mm6,[edx+32]	//; m_32 | m_31
        pfmul       mm4,mm0			//; x * m_12 | x * m_11
        movq        mm7,[edx+48]	//; m_42 | m_41
        punpckhdq   mm2,mm2			//;  y   |  y
        punpckhdq   mm3,mm1			//; rhw  |  ?
        pfmul       mm5,mm2			//; y * m_22 | y * m_21
        punpckldq   mm1,mm1			//;  z   |  z
        pfmul       mm0,[edx+8]		//; x * m_14 | x * m_13
        punpckhdq   mm3,mm3			//; rhw  | rhw
        pfmul       mm2,[edx+24]	//; y * m_24 | y * m_23
        pfmul       mm6,mm1			//; z * m_32 | z * m_31
        pfadd       mm5,mm4			//; x * m_12 + y * m_22 | x * m_11 + y * m_21
        pfmul       mm1,[edx+40]	//; z * m_34 | z * m_33
        pfadd       mm2,mm0			//; x * m_14 + y * m_24 | x * m_13 +  y * m_23
        pfmul       mm7,mm3			//; rhw * m_42 | rhw * m_41
        pfadd       mm6,mm5			//; x * m_12 + y * m_22 +  z * m_32 | x * m_11 + y * m_21 + z * m_31
        pfmul       mm3,[edx+56]	//; rhw * m_44  | rhw * m_43
        pfadd       mm2,mm1			//; x * m_14 + y * m_24 + z * m_34 | x * m_13 +  y * m_23 + z * m_33
        pfadd       mm7,mm6			//; x * m_12 + y * m_22 + z * m_32 + rhw * m_42 | x * m_11 + y * m_21 + z * m_31 + rhw * m_41
        pfadd       mm3,mm2			//; x * m_14 + y * m_24 + z * m_34 + rhw * m_44 | x * m_13 + y * m_23 + z * m_33 + rhw * m_43
        movq        [eax],mm7		//; r_y   | r_x
        movq        [eax+8],mm3		//; r_rhw | r_z
        femms
    }
}


//;******************************************************************************
//; Routine:  void _glTrans_4x4 (float *r, const float *a, const float *m)
//; Input:    r - result vector (4x1), 4 floats
//;           a - input  vector (4x1), 4 floats
//;           m - matrix (4x4), 16 floats
//; Output:   r = a * m (matrix multiplication rules)
//; Uses:     eax, ecx, edx, mm0 - mm7
//;******************************************************************************
void _glTrans_4x4 (float *r, const float *a, const float *m)
{
    __asm
    {
        femms
        mov         eax,[r]
        mov         edx,[m]
        mov         ecx,[a]

        movq        mm0,[ecx]		//; y   |  x
        movq        mm4,[ecx+8]		//; rhw |  z
        movq        mm1,mm0			//; y   |  x
        pfmul       mm0,[edx]		//; y * m_21 | x * m_11
        movq        mm5,mm4			//; rhw |  z
        pfmul       mm4,[edx+8]		//; rhw * m_41 | z * m_31
        movq        mm2,mm1			//; y   |  x
        pfmul       mm1,[edx+16]	//; y * m_22 | x * m_12
        movq        mm6,mm5			//; rhw |  z
        pfmul       mm5,[edx+24]	//; rhw * m_42 | z * m_32
        movq        mm3,mm2			//; y   |  x
        pfmul       mm2,[edx+32]	//; y * m_23 | x * m_13
        movq        mm7,mm6			//; rhw | z
        pfmul       mm6,[edx+40]	//; rhw * m_43 | z * m_33
        pfmul       mm3,[edx+48]	//; y * m_24 | x * m_14
        pfacc       mm0,mm4			//; rhw * m_41 + z * m_31 | y * m_21 + x * m_11 
        pfmul       mm7,[edx+56]	//; rhw * m_44 | z * m_34
        pfacc       mm1,mm5			//; rhw * m_42 + z * m_32 | y * m_22 + x * m_12
        pfacc       mm2,mm6			//; rhw * m_43 + z * m_33 | y * m_23 + x * m_13
        pfacc       mm3,mm7			//; rhw * m_44 + z * m_34 | y * m_24 | x * m_14 
        pfacc       mm0,mm1			//; rhw * m_41 + z * m_31 + y * m_21 + x * m_11 | rhw * m_42 + z * m_32 + y * m_22 + x * m_12
        pfacc       mm2,mm3			//; rhw * m_44 + z * m_34 + y * m_24 | x * m_14 | rhw * m_43 + z * m_33 + y * m_23 + x * m_13
        movq        [eax],mm0		//; r_y   | r_x
        movq        [eax+8],mm2		//; r_rhw | r_z
        femms
    }
}



//;******************************************************************************
//; Routine:  void _D3DTrans_3x3 (float *r, const float *a, const float *m)
//; Input:    r - result vector (1x3)
//;           a - input  vector (1x3)
//;           m - matrix (3x3)
//; Output:   r = a * m (matrix multiplication rules)
//; Uses:     eax, ecx, edx, mm0 - mm4
//; Note:     D3D matrix format - RHW not affected - basic 3x3 
//;******************************************************************************
void _D3DTrans_3x3 (float *r, const float *a, const float *m)
{
    __asm
    {
        femms
        mov         eax,[r]		    //; result
        mov         edx,[m]		    //; matrix
        mov         ecx,[a]		    //; vector

        movq        mm0,[ecx]		//;  y   |  x
        movd        mm1,[ecx+8]		//;  0   |  z
        movd        mm4,[edx+8]		//;  0   | m_13
        movq        mm3,mm0			//;  y   |  x
        movd        mm2,[edx+32]	//;  0   | m_33
        punpckldq   mm0,mm0			//;  x   |  x
        punpckldq   mm4,[edx+20]	//; m_31 | m_23
        pfmul       mm0,[edx]		//;  x * m_12 | x * m_11
        punpckhdq   mm3,mm3			//;  y   |  y
        pfmul       mm2,mm1			//;  0   |  z * m_33
        punpckldq   mm1,mm1			//;  z   |  z
        pfmul       mm4,[ecx]		//; y * m_31 | x * m_23
        pfmul       mm3,[edx+12]	//; y * m_22 | y * m_21
        pfmul       mm1,[edx+24]	//; z * m_32 | z * m_32
        pfacc       mm4,mm4			//;  ?   | y * m_31 + x * m_23
        pfadd       mm3,mm0			//;  x * m_12 + y * m_22 | x * m_11 + y * m_21
        pfadd       mm4,mm2			//;  ?   | y * m_31 + x * m_23 + z * m_33
        pfadd       mm3,mm1			//;  x * m_12 + y * m_22 + z * m_32 | x * m_11 + y * m_21 + z * m_32
        movd        [eax+8],mm4		//; r_z
        movq        [eax],mm3		//; r_y  | r_x
        femms
    }
}



//;******************************************************************************
//; Routine:  void _glTrans_3x3 (float *r, const float *a, const float *m)
//; Input:    r - result vector (3x1), 3 floats
//;           a - input  vector (3x1), 3 floats
//;           m - matrix (3x3), 9 floats
//; Output:   r = a * m (matrix multiplication rules)
//; Uses:     eax, ecx, edx, mm0 - mm5
//;******************************************************************************
void _glTrans_3x3 (float *r, const float *a, const float *m)
{
    __asm
    {
        femms
        mov         eax,[r]		    //; result
        mov         edx,[m]		    //; matrix
        mov         ecx,[a]		    //; vector

        movq        mm0,[ecx]		//;  y   |  x
        movd        mm4,[edx+8]		//;  0   | m_31
        movd        mm3,[ecx+8]		//;  0   |  z
        movq        mm1,mm0			//;  y   |  x
        movq        mm2,mm1			//;  y   |  x
        pfmul       mm0,[edx]		//; y * m_21 | x * m_11
        pfmul       mm1,[edx+12]	//; y * m_22 | x * m_12
        punpckldq   mm3,mm3			//;  z   |  z
        pfmul       mm2,[edx+24]	//; y * m_23 | x * m_13
        punpckldq   mm4,[edx+20]	//; m_32 | m_31
        pfacc       mm0,mm1			//; y * m_22 + x * m_12 | y * m_21 + x * m_11
        movd        mm5,[edx+32]	//;  0   | m_33
        pfacc       mm2,mm2			//;      | y * m_23 + x * m_13
        pfmul       mm4,mm3			//; z * m_32 | z * m_31
        pfmul       mm5,mm3			//;		| z * m_33
        pfadd       mm4,mm0			//; y * m_22 + x * m_12 + z * m_32 | y * m_21 + x * m_11 + z * m_31
        pfadd       mm5,mm2			//; 		|  y * m_23 + x * m_13 + z * m_33
        movq        [eax],mm4		//; r_y   | r_x
        movd        [eax+8],mm5		//;       | r_z
        femms
    }
}



//;******************************************************************************
//; Routine:  void _D3DTrans_a3x3 (float *r, const float *a, const float *m, int n)
//; Input:    r - result vector, 3 floats
//;           a - input  vector, 3 floats
//;           m - matrix (3x3), 9 floats
//;           n - number of vectors in 'a' and 'r'
//; Output:   r[n] = a[n] * m (matrix multiplication rules)
//; Uses:     eax, ecx, edx, mm0 - mm7
//;******************************************************************************
void _D3DTrans_a3x3 (float *r, const float *a, const float *m, int n)
{
    __m64 tmp;
    __asm
    {
	push	esi
        mov         ecx,[n]		    //; number of vectors
        femms
        cmp         ecx,0			//; test for no vectors
        jle         exit			//; if no vectors, leave

        mov         edx,[a]         //; vector
        mov         esi,[m]         //; matrix
        mov         eax,[r]         //; result

        prefetch    [edx]
        movd        mm0,[esi+32]	//;  0   | m_33
        movq        [tmp],mm0		//;  0   | m_33
        movq        mm7,[esi]		//; m_12 | m_11
        movd        mm5,[esi+8]		//;  0   | m_13
        punpckhdq   mm6,mm7			//; m_12 | ?
        punpckldq   mm7,[esi+12]	//; m_21 | m_11
        punpckhdq   mm6,[esi+12]	//; m_22 | m_11
        punpckldq   mm5,[esi+20]	//; m_23 | m_13

        //;;; Unrolled loop does two passes - detect odd number of vectors
        shr         ecx,1
        prefetch    [edx]
        jnc         short do_next
        add         ecx,1
        jmp         short do_next2

        ALIGN       16
do_next:
        prefetch    [edx+12]
        movq        mm0,[edx]		//;  y   |  x
        movd        mm3,[edx+8]		//;  0   |  z
        movq        mm1,mm0			//;  y   |  x
        pfmul       mm0,mm7			//; y * m_21 | x * m_11 
        movq        mm2,mm1			//;  y   |  x
        pfmul       mm1,mm6			//; y * m_22 | x * m_11
        punpckldq   mm3,mm3			//;  z   |  z
        pfmul       mm2,mm5			//; y * m_23 | x * m_13
        movq        mm4,mm3			//;  z   |  z
        pfmul       mm3,[esi+24]	//; z * m_32 | z * m_31
        pfmul       mm4,[tmp]		//;  0   | z * m_33
        pfacc       mm0,mm1			//; y * m_22 + x * m_11 |  y * m_21 + x * m_11 
        pfacc       mm2,mm2			//;      |  y * m_23 + x * m_13
        pfadd       mm3,mm0			//; y * m_22 + x * m_11 +  z * m_32 |  y * m_21 + x * m_11 +  z * m_31
        pfadd       mm4,mm2			//;      |  y * m_23 + x * m_13 + z * m_33
        add         edx,12			//; next vert
        movq        [eax],mm3		//; r_y  | r_x
        movd        [eax+8],mm4		//;      | r_z
        add         eax,12			//; next result

do_next2:
        prefetch    [edx+12]
        movq        mm0,[edx]		//;  y   |  x
        movd        mm3,[edx+8]		//;  0   |  z
        movq        mm1,mm0			//;  y   |  x
        pfmul       mm0,mm7			//; y * m_21 | x * m_11
        movq        mm2,mm1			//;  y   |  x
        pfmul       mm1,mm6			//; y * m_22 | x * m_11
        punpckldq   mm3,mm3			//;  z   |  z
        pfmul       mm2,mm5			//; y * m_23 | x * m_13
        movq        mm4,mm3			//;  z   |  z
        pfmul       mm3,[esi+24]	//; z * m_32 | z * m_31
        pfmul       mm4,[tmp]		//;  0   | z * m_33
        pfacc       mm0,mm1			//; y * m_22 + x * m_11 |  y * m_21 + x * m_11
        pfacc       mm2,mm2			//;      |  y * m_23 + x * m_13
        pfadd       mm3,mm0			//; y * m_22 + x * m_11 +  z * m_32 |  y * m_21 + x * m_11 +  z * m_31
        pfadd       mm4,mm2			//;      |  y * m_23 + x * m_13 + z * m_33
        add         edx,12			//; next vert
        movq        [eax],mm3		//; r_y  | r_x
        movd        [eax+8],mm4		//;      | r_z
        add         eax,12			//; next result

        dec         ecx				//; count down n
        jne         do_next

exit:
        femms
	pop	esi
    }
}



//;******************************************************************************
//; Routine:  void _glTrans_a3x3 (float *r, const float *a, const float *m, int n)
//; Input:    r - array of 'n' vectors (3x1), n*3 floats
//;           a - array of 'n' vectors (3x1), n*3 floats
//;           m - matrix (3x3), 9 floats
//;           n - number of vectors in 'a' and 'r'
//; Output:   r[n] = a[n] * m (matrix multiplication rules)
//; Uses:     eax, ecx, edx, mm0 - mm5
//;******************************************************************************
void _glTrans_a3x3 (float *r, const float *a, const float *m, int n)
{
    __asm
    {
        mov         ecx,[n]
        femms
        cmp         ecx,0
        jle         exit

        mov         eax,[r]
        mov         ebx,[m]
        mov         edx,[a]

        //;;; Unrolled loop does two passes - detect odd number of vectors
        shr         ecx,1
        prefetch    [edx]
        jnc         short do_next
        add         ecx,1
        jmp         short do_next2

        ALIGN       16
do_next:
        prefetch    [edx+12]
        movq        mm0,[edx]
        movd        mm4,[ebx+8]
        movd        mm3,[edx+8]
        movq        mm1,mm0
        movq        mm2,mm1
        pfmul       mm0,[ebx]
        pfmul       mm1,[ebx+12]
        punpckldq   mm3,mm3
        pfmul       mm2,[ebx+24]
        punpckldq   mm4,[ebx+20]
        pfacc       mm0,mm1
        movd        mm5,[ebx+32]
        pfacc       mm2,mm2
        pfmul       mm4,mm3
        pfmul       mm5,mm3
        pfadd       mm4,mm0
        pfadd       mm5,mm2
        add         edx,12
        movq        [eax],mm4
        movd        [eax+8],mm5
        add         eax,12

do_next2:
        prefetch    [edx+12]
        movq        mm0,[edx]
        movd        mm4,[ebx+8]
        movd        mm3,[edx+8]
        movq        mm1,mm0
        movq        mm2,mm1
        pfmul       mm0,[ebx]
        pfmul       mm1,[ebx+12]
        punpckldq   mm3,mm3
        pfmul       mm2,[ebx+24]
        punpckldq   mm4,[ebx+20]
        pfacc       mm0,mm1
        movd        mm5,[ebx+32]
        pfacc       mm2,mm2
        pfmul       mm4,mm3
        pfmul       mm5,mm3
        pfadd       mm4,mm0
        pfadd       mm5,mm2
        add         edx,12
        movq        [eax],mm4
        movd        [eax+8],mm5
        add         eax,12

        dec         ecx
        jne         do_next

exit:
        femms
    }
}



//;******************************************************************************
//; Routine:  void _D3DTrans_a4x4 (D3DTLVERTEX *r, const D3DTLVERTEX *a,
//;                                const D3DMATRIX &m, int n)
//; Input:    r - array of 'n' vectors (1x4)
//;           a - array of 'n' vectors (1x4)
//;           m - matrix (4x4) reference
//;           n - number of vectors in 'a' and 'r'
//; Output:   r[n] = a[n] * m (matrix multiplication rules)
//; Uses:     eax, ecx, edx, mm0 - mm7
//;******************************************************************************
void _D3DTrans_a4x4 (D3DTLVERTEX *r, const D3DTLVERTEX *a, const D3DMATRIX &m, int n)
{
    __asm
    {
        mov         ecx,[n]
        femms
        cmp         ecx,0
        jle         exit

        mov         eax,[r]
        mov         ebx,[m]
        mov         edx,[a]

        //;;; Unrolled loop does two passes - detect odd number of vectors
        shr         ecx,1
        prefetch    [edx]
        prefetchw   [eax]
        jnc         short do_next
        add         ecx,1
        jmp         do_next2

        ALIGN       16
do_next:
        prefetch    [edx+32]
        prefetchw   [eax+32]
        movq        mm0,[edx]
        movq        mm1,[edx+8]
        movq        mm4,[ebx]
        punpckhdq   mm2,mm0
        movq        mm5,[ebx+16]
        punpckldq   mm0,mm0
        movq        mm6,[ebx+32]
        pfmul       mm4,mm0
        movq        mm7,[ebx+48]
        punpckhdq   mm2,mm2
        punpckhdq   mm3,mm1
        pfmul       mm5,mm2
        punpckldq   mm1,mm1
        pfmul       mm0,[ebx+8]
        punpckhdq   mm3,mm3
        pfmul       mm2,[ebx+24]
        pfmul       mm6,mm1
        pfadd       mm5,mm4
        pfmul       mm1,[ebx+40]
        pfadd       mm2,mm0
        pfmul       mm7,mm3
        pfadd       mm6,mm5
        pfmul       mm3,[ebx+56]
        pfadd       mm2,mm1
        pfadd       mm7,mm6
        pfadd       mm3,mm2
        add         edx,8*4
        movq        [eax+0],mm7
        movq        [eax+8],mm3
        add         eax,8*4

        ALIGN       16
do_next2:
        prefetch    [edx+32]
        prefetchw   [eax+32]
        movq        mm0,[edx]
        movq        mm1,[edx+8]
        movq        mm4,[ebx]
        punpckhdq   mm2,mm0
        movq        mm5,[ebx+16]
        punpckldq   mm0,mm0
        movq        mm6,[ebx+32]
        pfmul       mm4,mm0
        movq        mm7,[ebx+48]
        punpckhdq   mm2,mm2
        punpckhdq   mm3,mm1
        pfmul       mm5,mm2
        punpckldq   mm1,mm1
        pfmul       mm0,[ebx+8]
        punpckhdq   mm3,mm3
        pfmul       mm2,[ebx+24]
        pfmul       mm6,mm1
        pfadd       mm5,mm4
        pfmul       mm1,[ebx+40]
        pfadd       mm2,mm0
        pfmul       mm7,mm3
        pfadd       mm6,mm5
        pfmul       mm3,[ebx+56]
        pfadd       mm2,mm1
        pfadd       mm7,mm6
        pfadd       mm3,mm2
        add         edx,8*4
        movq        [eax],mm7
        movq        [eax+8],mm3
        add         eax,8*4

        dec         ecx
        jne         do_next

exit:
        femms
    }
}



//;******************************************************************************
//; Routine:  void _glTrans_a4x4 (float *r, const float *a, const float *m, int n)
//; Input:    r - array of 'n' vectors (4x1), n*4 floats
//;           a - array of 'n' vectors (4x1), n*4 floats
//;           m - matrix (4x4) reference, 16 floats
//;           n - number of vectors in 'a' and 'r'
//; Output:   r[n] = a[n] * m (matrix multiplication rules)
//; Uses:     eax, ecx, edx, mm0 - mm7
//;******************************************************************************
void _glTrans_a4x4 (float *r, const float *a, const float *m, int n)
{
    __asm
    {
        mov         ecx,[n]
        femms
        cmp         ecx,0
        jle         done

        mov         eax,[r]
        mov         ebx,[m]
        mov         edx,[a]

        //;;; Unrolled loop does two passes - detect odd number of vectors
        shr         ecx,1
        prefetch    [edx]
        prefetchw   [eax]
        jnc         short do_next
        add         ecx,1
        jmp         do_next2

        ALIGN       16
do_next:
        prefetch    [edx+8*4]
        prefetchw   [eax+8*4]
        movq        mm0,[edx]
        movq        mm4,[edx+8]
        movq        mm1,mm0
        pfmul       mm0,[ebx]
        movq        mm5,mm4
        pfmul       mm4,[ebx+8]
        movq        mm2,mm1
        pfmul       mm1,[ebx+16]
        movq        mm6,mm5
        pfmul       mm5,[ebx+24]
        movq        mm3,mm2
        pfmul       mm2,[ebx+32]
        movq        mm7,mm6
        pfmul       mm6,[ebx+40]
        pfmul       mm3,[ebx+48]
        pfacc       mm0,mm4
        pfmul       mm7,[ebx+56]
        pfacc       mm1,mm5
        pfacc       mm2,mm6
        pfacc       mm3,mm7
        pfacc       mm0,mm1
        pfacc       mm2,mm3
        add         edx,8*4
        movq        [eax],mm0
        movq        [eax+8],mm2
        add         eax,8*4

do_next2:
        prefetch    [edx+8*4]
        prefetchw   [eax+8*4]
        movq        mm0,[edx]
        movq        mm4,[edx+8]
        movq        mm1,mm0
        pfmul       mm0,[ebx]
        movq        mm5,mm4
        pfmul       mm4,[ebx+8]
        movq        mm2,mm1
        pfmul       mm1,[ebx+16]
        movq        mm6,mm5
        pfmul       mm5,[ebx+24]
        movq        mm3,mm2
        pfmul       mm2,[ebx+32]
        movq        mm7,mm6
        pfmul       mm6,[ebx+40]
        pfmul       mm3,[ebx+48]
        pfacc       mm0,mm4
        pfmul       mm7,[ebx+56]
        pfacc       mm1,mm5
        pfacc       mm2,mm6
        pfacc       mm3,mm7
        pfacc       mm0,mm1
        pfacc       mm2,mm3
        add         edx,8*4
        movq        [eax],mm0
        movq        [eax+8],mm2
        add         eax,8*4

        dec         ecx
        jne         do_next

done:
        femms
    }
}

// eof
