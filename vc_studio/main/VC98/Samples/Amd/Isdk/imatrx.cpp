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
#include "amath.h"

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

#pragma warning (disable:4730)

#define _m_from_float(f) _m_from_int (*(int *)&(f))
#define _m_from_floats(f) (*(__m64 *)(f))


//;******************************************************************************
//; Routine:  void _glMul_3x3 (float *r, const float *a, const float *b)
//; Input:    r - matrix (3x3) address
//;           a - matrix (3x3) address
//;           b - matrix (3x3) address
//; Output:   r = a * b, using standard matrix multiplication rules
//;******************************************************************************
void _glMul_3x3 (float *r, const float *a, const float *b)
{
    __m64 r0, r1, r2, r3, r4, r5, r6, r7;
    _m_femms();

    r7 = _m_from_float (b[2]);		                //; 0    | b_31
    r6 = _m_from_float (b[8]);	                    //; 0    | b_33
    r7 = _m_punpckldq  (r7,_m_from_floats (b+5));   //; b_31 | b_31
    r0 = _m_from_floats(a+0);                       //; a_21 | a_11
    r3 = _m_from_float (a[2]);                      //; 0    | a_31
    r1 = r0;			                            //; a_21 | a_11
    r0 = _m_pfmul      (r0,r7);		                //; a_21 * b_31 | a_11 * b_31
    r2 = r1;			                            //; a_21 | a_11
    r1 = _m_punpckldq  (r1,r1);		                //; a_11 | a_11
    r1 = _m_pfmul      (r1,_m_from_floats (b+0));	//; a_11 * b_21 | a_11 * b_11 
    r2 = _m_punpckhdq  (r2,r2);		                //; a_21 | a_21
    r2 = _m_pfmul      (r2,_m_from_floats (b+3));	//; a_21 * b_22 | a_21 * b_12 
    r0 = _m_pfacc      (r0,r0);		                //; XXX  | a_21 * b_31 + a_11 * b_31
    r4 = r3;			                            //; 0    | a_31
    r3 = _m_punpckldq  (r3,r3);		                //; a_31 | a_31
    r3 = _m_pfmul      (r3,_m_from_floats (b+6));	//; a_31 * b_23 | a_31 * b_13
    r2 = _m_pfadd      (r2,r1);		                //; a_11 * b_21 + a_21 * b_22 | a_11 * b_11 + a_21 * b_12
    r4 = _m_pfmul      (r4,r6);		                //; 0    | a_31 * b_33
    r5 = _m_from_floats(a+3);                       //; a_22 | a_12
    r2 = _m_pfadd      (r2,r3);		                //; a_31 * b_23 + a_11 * b_21 + a_21 * b_22 | a_31 * b_13 + a_11 * b_11 + a_21 * b_12
    r3 = _m_from_float (a[5]);                      //; 0    | a_32
    r4 = _m_pfadd      (r4,r0);		                //; XXX  | a_31 * b_33 + a_21 * b_31 + a_11 * b_31
    r1 = r5;			                            //; a_22 | a_12
    *(__m64 *)r = r2;                               //; r_21 | r_11
    r5 = _m_pfmul      (r5,r7);		                //; a_22 * b_31 | a_12 * b_31
    r[2] = r4.m64_f32[0];                           //; 0    | r_31
    r2 = r1;			                            //; a_22 | a_12
    r1 = _m_punpckldq  (r1,r1);		                //; a_12 | a_12
    r0 = _m_from_floats(a+6);                       //; a_23 | a_13
    r1 = _m_pfmul      (r1,_m_from_floats (b+0)); 	//; a_12 * b_21 | a_12 * b_11
    r2 = _m_punpckhdq  (r2,r2);		                //; a_22 | a_22
    r2 = _m_pfmul      (r2,_m_from_floats (b+3));   //; a_22 * b_22 | a_22 * b_12
    r5 = _m_pfacc      (r5,r5);		                //; XXX  | a_22 * b_31 + a_12 * b_31
    r4 = r3;			                            //; 0    | a_32
    r3 = _m_punpckldq  (r3,r3);		                //; a_32 | a_32
    r3 = _m_pfmul      (r3,_m_from_floats (b+6));   //; a_32 b_23 | a_32 * b_13
    r2 = _m_pfadd      (r2,r1);		                //; a_22 * b_22 + a_12 * b_21 | a_22 * b_12 + a_12 * b_11
    r4 = _m_pfmul      (r4,r6);		                //; 0    | a_32 * b_33
    r1 = r0;			                            //; a_23 | a_13
    r2 = _m_pfadd      (r2,r3);		                //; a_32 b_23 + a_22 * b_22 + a_12 * b_21 | a_32 * b_13 + a_22 * b_12 + a_12 * b_11
    r3 = _m_from_float (a[8]);                      //; 0    | a_33
    r4 = _m_pfadd      (r4,r5);		                //; XXX  | a_32 * b_33 + a_22 * b_31 + a_12 * b_31
    r0 = _m_pfmul      (r0,r7);		                //; a_23 * b_31 | a_13 * b_31
    *(__m64 *)(r+3) = r2;                           //; r_22 | r_12
    r2 = r1;			                            //; a_23 | a_13
    r[5] = r4.m64_f32[0];                           //; 0    | r_32
    r1 = _m_punpckldq  (r1,r1);		                //; a_13 | a_13
    r1 = _m_pfmul      (r1,_m_from_floats (b+0));   //; a_13 * b_21 | a_13 * b_11 
    r2 = _m_punpckhdq  (r2,r2);		                //; a_23 | a_23
    r2 = _m_pfmul      (r2,_m_from_floats (b+3));   //; a_23 * b_22 | a_23 * b_12
    r0 = _m_pfacc      (r0,r0);		                //; XXX  | a_23 * b_31 + a_13 * b_31
    r6 = _m_pfmul      (r6,r3);		                //;  0   | a_33 * b_33
    r3 = _m_punpckldq  (r3,r3);		                //; a_33 | a_33
    r3 = _m_pfmul      (r3,_m_from_floats (b+6));   //; a_33 * b_23 | a_33 * b_13
    r2 = _m_pfadd      (r2,r1);		                //; a_13 * b_21 + a_23 * b_22 | a_13 * b_11 + a_23 * b_12
    r6 = _m_pfadd      (r6,r0);		                //;  0   | a_33 * b_33 + a_23 * b_31 + a_13 * b_31
    r2 = _m_pfadd      (r2,r3);		                //;  a_33 * b_23 + a_13 * b_21 + a_23 * b_22 | a_33 * b_13 + a_13 * b_11 + a_23 * b_12
    r[8] = r6.m64_f32[0];                           //; 0    | r_33
    *(__m64 *)(r+6) = r2;                           //; r_23 | r_13

    _m_femms();
}


//;******************************************************************************
//; Routine:  void _glMul_4x4 (const float *r, const float *a, const float *b)
//; Input:    r - matrix (4x4) address
//;           a - matrix (4x4) address
//;           b - matrix (4x4) address
//; Output:   r = a * b, using standard matrix multiplication rules
//;******************************************************************************
void _glMul_4x4 (float *r, const float *a, const float *b)
{
    __m64 r0, r1, r2, r3, r4, r5, r6, r7;
    _m_femms();

    r0 = _m_from_floats(a);		                    //; a_21	| a_11
    r1 = _m_from_floats(a+2);                       //; a_41	| a_31
    r4 = _m_from_floats(b);                         //; b_21	| b_11
    r2 = _m_punpckhdq  (r0,r0);                     //; a_21	| 
    r5 = _m_from_floats(b+4);                       //; b_22	| b_12
    r3 = _m_punpckhdq  (r1,r1);                     //; a_41	| 
    r6 = _m_from_floats(b+8);                       //; b_23	| b_13
    r0 = _m_punpckldq  (r0,r0);                     //; a_11	| a_11
    r1 = _m_punpckldq  (r1,r1);                     //; a_31	| a_31
    r4 = _m_pfmul      (r4,r0);                     //; a_11 * b_21 | a_11 * b_11
    r2 = _m_punpckhdq  (r2,r2);                     //; a_21	| a_21
    r0 = _m_pfmul      (r0,_m_from_floats (b+2));   //; a_11 * b_41 | a_11 * b_31
    r7 = _m_from_floats(b+12);                      //; b_24	| b_14
    r5 = _m_pfmul      (r5,r2);                     //; a_21 * b_22 | a_21 * b_12
    r3 = _m_punpckhdq  (r3,r3);                     //; a_41	| a_41
    r2 = _m_pfmul      (r2,_m_from_floats (b+6));   //; a_21 * b_42 | a_21 * b_32
    r6 = _m_pfmul      (r6,r1);                     //; a_31 * b_23 | a_31 * b_13 
    r5 = _m_pfadd      (r5,r4);                     //; a_21 * b_22 + a_11 * b_21 | a_21 * b_12 + a_11 * b_11
    r1 = _m_pfmul      (r1,_m_from_floats (b+10));	//; a_31 * b_43 | a_31 * b_33
    r2 = _m_pfadd      (r2,r0);                     //; a_21 * b_42 + a_11 * b_41 | a_21 * b_32 + a_11 * b_31
    r7 = _m_pfmul      (r7,r3);                     //; a_41 * b_24 | a_41 * b_14 
    r6 = _m_pfadd      (r6,r5);                     //; a_21 * b_22 + a_11 * b_21 + a_31 * b_23 | a_21 * b_12 + a_11 * b_11 + a_31 * b_13
    r3 = _m_pfmul      (r3,_m_from_floats (b+14));  //; a_41 * b_44 | a_41 * b_34
    r2 = _m_pfadd      (r2,r1);                     //; a_21 * b_42 + a_11 * b_41 + a_31 * b_43 | a_21 * b_32 + a_11 * b_31 + a_31 * b_33 
    r7 = _m_pfadd      (r7,r6);                     //; a_41 * b_24 + a_21 * b_22 + a_11 * b_21 + a_31 * b_23 |  a_41 * b_14 + a_21 * b_12 + a_11 * b_11 + a_31 * b_13
    r0 = _m_from_floats(a+4);                       //; a_22	| a_12
    r3 = _m_pfadd      (r3,r2);                     //; a_41 * b_44 + a_21 * b_42 + a_11 * b_41 + a_31 * b_43 | a_41 * b_34 + a_21 * b_32 + a_11 * b_31 + a_31 * b_33 
    r1 = _m_from_floats(a+6);                       //; a_42	| a_32
    ((__m64 *)r)[0] = r7;                           //; r_21	| r_11 
    r4 = _m_from_floats(b);                         //; b_21	| b_11
    ((__m64 *)r)[1] = r3;                           //; r_41	| r_31

    r2 = _m_punpckhdq  (r2,r0);			            //; a_22	| XXX
    r5 = _m_from_floats(b+4);                       //; b_22	| b_12
    r3 = _m_punpckhdq  (r3,r1);                     //; a_42	| XXX
    r6 = _m_from_floats(b+8);                       //; b_23	| b_13
    r0 = _m_punpckldq  (r0,r0);                     //; a_12	| a_12
    r1 = _m_punpckldq  (r1,r1);                     //; a_32	| a_32
    r4 = _m_pfmul      (r4,r0);                     //; a_12 * b_21 | a_12 * b_11
    r2 = _m_punpckhdq  (r2,r2);                     //; a_22	| a_22
    r0 = _m_pfmul      (r0,_m_from_floats (b+2));   //; a_12 * b_41 | a_12 * b_31
    r7 = _m_from_floats(b+12);                      //; b_24	| b_14
    r5 = _m_pfmul      (r5,r2);			            //; a_22 * b_22 | a_22 * b12
    r3 = _m_punpckhdq  (r3,r3);			            //; a_42	| a_42
    r2 = _m_pfmul      (r2,_m_from_floats (b+6));	    //; a_22 * b_42 | a_22 * b_32
    r6 = _m_pfmul      (r6,r1);			            //; a_32 * b_23 | a_32 * b_13
    r5 = _m_pfadd      (r5,r4);			            //; a_12 * b_21 + a_22 * b_22 | a_12 * b_11 + a_22 * b12
    r1 = _m_pfmul      (r1,_m_from_floats (b+10));	//; a_32 * b_43 | a_32 * b_33
    r2 = _m_pfadd      (r2,r0);			            //; a_12 * b_41 + a_22 * b_42 | a_12 * b_11 + a_22 * b_32
    r7 = _m_pfmul      (r7,r3);			            //; a_42 * b_24 | a_42 * b_14
    r6 = _m_pfadd      (r6,r5);			            //; a_32 * b_23 + a_12 * b_21 + a_22 * b_22 | a_32 * b_13 + a_12 * b_11 + a_22 * b12
    r3 = _m_pfmul      (r3,_m_from_floats (b+14));	//; a_42 * b_44 | a_42 * b_34
    r2 = _m_pfadd      (r2,r1);			            //; a_32 * b_43 + a_12 * b_41 + a_22 * b_42 | a_32 * b_33 + a_12 * b_11 + a_22 * b_32
    r7 = _m_pfadd      (r7,r6);			            //; a_42 * b_24 + a_32 * b_23 + a_12 * b_21 + a_22 * b_22 | a_42 * b_14 + a_32 * b_13 + a_12 * b_11 + a_22 * b12
    r0 = _m_from_floats(a+8);                       //; a_23 | a_13
    r3 = _m_pfadd      (r3,r2);			            //; a_42 * b_44 + a_32 * b_43 + a_12 * b_41 + a_22 * b_42 | a_42 * b_34 + a_32 * b_33 + a_12 * b_11 + a_22 * b_32
    r1 = _m_from_floats(a+10);                      //; a_43 | a_33
    ((__m64 *)r)[2] = r7;                           //; r_22 | r_12
    r4 = _m_from_floats(b);                         //; b_21	| b_11
    ((__m64 *)r)[3] = r3;                           //; r_42 | r_32

    r2 = _m_punpckhdq  (r2,r0);                     //; a_23 | XXX
    r5 = _m_from_floats(b+4);                       //; b_22 | b_12
    r3 = _m_punpckhdq  (r3,r1);                     //; a_43 | XXX
    r6 = _m_from_floats(b+8);                       //; b_23 | b_13
    r0 = _m_punpckldq  (r0,r0);                     //; a_13 | a_13
    r1 = _m_punpckldq  (r1,r1);                     //; a_33 | a_33
    r4 = _m_pfmul      (r4,r0);                     //; a_13 * b_21 | a_13 * b_11
    r2 = _m_punpckhdq  (r2,r2);                     //; a_23 | a_23
    r0 = _m_pfmul      (r0,_m_from_floats (b+2));   //; a_13 * b_41 | a_13 * b_31
    r7 = _m_from_floats(b+12);                      //; b_24 | b_14
    r5 = _m_pfmul      (r5,r2);                     //; a_23 * b_22 | a_23 * b_12
    r3 = _m_punpckhdq  (r3,r3);                     //; a_43 | a_43
    r2 = _m_pfmul      (r2,_m_from_floats (b+6));   //; a_23 * b_42 | a_23 * b_32
    r6 = _m_pfmul      (r6,r1);                     //; a_33 * b_23 | a_33 * b_13
    r5 = _m_pfadd      (r5,r4);                     //; a_23 * b_22 + a_13 * b_21 | a_23 * b_12 + a_13 * b_11
    r1 = _m_pfmul      (r1,_m_from_floats (b+10));  //; a_33 * b_43 | a_33 * b_33 
    r2 = _m_pfadd      (r2,r0);                     //; a_13 * b_41 + a_23 * b_42 | a_13 * b_31 + a_23 * b_32
    r7 = _m_pfmul      (r7,r3);                     //; a_43 * b_24 | a_43 * b_14
    r6 = _m_pfadd      (r6,r5);                     //; a_33 * b_23 + a_23 * b_22 + a_13 * b_21 | a_33 * b_13 + a_23 * b_12 + a_13 * b_11
    r3 = _m_pfmul      (r3,_m_from_floats (b+14));  //; a_43 * b_44 | a_43 * b_34
    r2 = _m_pfadd      (r2,r1);                     //; a_33 * b_43 * a_13 * b_41 + a_23 * b_42 | a_33 * b_33 + a_13 * b_31 + a_23 * b_32
    r7 = _m_pfadd      (r7,r6);                     //; a_43 * b_24 + a_33 * b_23 + a_23 * b_22 + a_13 * b_21 | a_43 * b_14 + a_33 * b_13 + a_23 * b_12 + a_13 * b_11
    r0 = _m_from_floats(a+12);                      //; a_24 | a_14
    r3 = _m_pfadd      (r3,r2);                     //; a_43 * b_44 + a_33 * b_43 * a_13 * b_41 + a_23 * b_42 | a_43 * b_34 + a_33 * b_33 + a_13 * b_31 + a_23 * b_32
    r1 = _m_from_floats(a+14);                      //; a_44 | a_34
    ((__m64 *)r)[4] = r7;                           //; r_23 | r_13
    r4 = _m_from_floats(b);                         //; b_21 | b_11
    ((__m64 *)r)[5] = r3;                           //; r_43 | r_33

    r2 = _m_punpckhdq  (r2,r0);                     //; a_24 | XXX
    r5 = _m_from_floats(b+4);                       //; b_22 | b_12
    r3 = _m_punpckhdq  (r3,r1);                     //; a_44 | XXX
    r6 = _m_from_floats(b+8);                       //; b_23 | b_13
    r0 = _m_punpckldq  (r0,r0);                     //; a_14 | a_14
    r1 = _m_punpckldq  (r1,r1);                     //; a_34 | a_34
    r4 = _m_pfmul      (r4,r0);                     //; a_14 * b_21 | a_14 * b_11
    r2 = _m_punpckhdq  (r2,r2);                     //; a_24 | a_24
    r0 = _m_pfmul      (r0,_m_from_floats (b+2));   //; a_14 * b_41 | a_14 * b_31
    r7 = _m_from_floats(b+12);                      //; b_24 | b_14
    r5 = _m_pfmul      (r5,r2);                     //; a_24 * b_22 | a_24 * b_12
    r3 = _m_punpckhdq  (r3,r3);                     //; a_44 | a_44
    r2 = _m_pfmul      (r2,_m_from_floats (b+6));   //; a_24 * b_ 42 | a_24 * b_32
    r6 = _m_pfmul      (r6,r1);                     //; a_34 * b_23 | a_34 * b_13
    r5 = _m_pfadd      (r5,r4);                     //; a_14 * b_21 + a_24 * b_22 | a_14 * b_11 + a_24 * b_12
    r1 = _m_pfmul      (r1,_m_from_floats (b+10));	//; a_34 * b_43 | a_34 * b_33
    r2 = _m_pfadd      (r2,r0);                     //; a_14 * b_41 + a_24 * b_ 42 | a_14 * b_31 + a_24 * b_32
    r7 = _m_pfmul      (r7,r3);                     //; a_44 * b_24 | a_44 * b_14
    r6 = _m_pfadd      (r6,r5);                     //; a_34 * b_23 + a_14 * b_21 + a_24 * b_22 | a_34 * b_13 + a_14 * b_11 + a_24 * b_12
    r3 = _m_pfmul      (r3,_m_from_floats (b+14));	//; a_44 * b_44 | a_44 * b_34
    r2 = _m_pfadd      (r2,r1);                     //; a_34 * b_43 + a_14 * b_41 + a_24 * b_ 42 | a_34 * b_33 + a_14 * b_31 + a_24 * b_32
    r7 = _m_pfadd      (r7,r6);                     //; a_44 * b_24 + a_14 * b_23 + a_24 * b_ 42 | a_44 * b_14 + a_14 * b_31 + a_24 * b_32
    r3 = _m_pfadd      (r3,r2);                     //; a_44 * b_44 + a_34 * b_43 + a_14 * b_41 + a_24 * b_42 | a_44 * b_34 + a_34 * b_33 + a_14 * b_31 + a_24 * b_32
    ((__m64 *)r)[6] = r7;                           //; r_24 | r_14
    ((__m64 *)r)[7] = r3;                           //; r_44 | r_34
    _m_femms();
}


//;*********************************************************************
//; VOID _D3DMul_4x4 (D3DMATRIX &q, D3DMATRIX& a, D3DMATRIX& b)
//; REPLACES D3DMath_MatrixMultiply IN DIRECTX 6.0 D3DFRAME.LIB
//; MULTIPLY 2 - 4 x 4 MATRICES
//; LOCAL STACK USED FOR TEMPS TO PREVENT OVERWRITE IN CASE
//; WHERE SOURCE AND DEST WERE SAME MATRIX
//;*********************************************************************
void _D3DMul_4x4 (D3DMATRIX &r, const D3DMATRIX& a, const D3DMATRIX& b)
{
    __m64 r0, r1, r2, r3, r4, r5, r6, r7;
    D3DMATRIX tmp;
    _m_femms();

    r0 = _m_from_float (a._21);                     //;       | a_21
    r1 = _m_from_float (a._11);                     //;       | a_11
    r6 = _m_from_float (a._12);                     //;       | a_12
    r1 = _m_punpckldq  (r1,r0);                     //; a_21  | a_11  
    r5 = _m_from_float (a._22);                     //;       | a_22
    r1 = _m_pfmul      (r1,_m_from_floats (&b._11));//; a_21 * b_12 | a_11 * b_11     
    r6 = _m_punpckldq  (r6,r5);                     //; a_22  | a_12      
    r7 = _m_from_float (a._32);                     //;       | a_32
    r5 = _m_from_float (a._42);                     //;       | a_42
    r6 = _m_pfmul      (r6,_m_from_floats (&b._11));//; a_22 * b_12 | a_12 * b_11     
    r2 = _m_from_float (a._31);                     //;       | a_31
    r7 = _m_punpckldq  (r7,r5);                     //; a_42  | a_32
    r0 = _m_from_float (a._41);                     //;       | a_41
    r7 = _m_pfmul      (r7,_m_from_floats (&b._14));//; a_42 * b_14 | a_32 * b13
    r2 = _m_punpckldq  (r2,r0);                     //; a_41  | a_31
    r6 = _m_pfadd      (r6,r7);				        //; a_42 * b_14 + a_22 * b_12 | a_32 * b13 + a_12 * b_11
    r2 = _m_pfmul      (r2,_m_from_floats (&b._14));//; a_41 * b_14 | a_31 * b13
    r6 = _m_pfacc      (r6,r6);				        //;		| a_12 * b_11 + a_22 * b_12 + a_32 * b_13 + a_42 * b_14  
    r1 = _m_pfadd      (r1,r2);				        //; a_21 * b_12 + a_41 * b_14 | a_11 * b_11 + a_31 * b13
    tmp._12 = r6.m64_f32[0];                        //; T_12   
    r1 = _m_pfacc      (r1,r1);				        //;       |  a_21 * b_12 + a_41 * b_14 + a_11 * b_11 + a_31 * b13
    tmp._11 = r1.m64_f32[0];                        //; T_11

    r0 = _m_from_float (a._23);                     //;       | a_23
    r1 = _m_from_float (a._13);                     //;       | a_13
    r6 = _m_from_float (a._14);                     //;       | a_14
    r1 = _m_punpckldq  (r1,r0);                     //; a_23  | a_13  
    r5 = _m_from_float (a._24);                     //;       | a_24
    r1 = _m_pfmul      (r1,_m_from_floats (&b._11));//; a_23 * b_12 | a_13 * b_11     
    r6 = _m_punpckldq  (r6,r5);                     //; a_24  | a_14      
    r7 = _m_from_float (a._34);                     //;       | a_34
    r5 = _m_from_float (a._44);                     //;       | a_44
    r6 = _m_pfmul      (r6,_m_from_floats (&b._11));//; a_24 * b_12 | a_14 * b_11     
    r2 = _m_from_float (a._33);                     //;       | a_33
    r7 = _m_punpckldq  (r7,r5);                     //; a_44  | a_34
    r0 = _m_from_float (a._43);                     //;       | a_43
    r7 = _m_pfmul      (r7,_m_from_floats (&b._14));//; a_44 * b_14 | a_34 * b_13
    r2 = _m_punpckldq  (r2,r0);                     //; a_43  | a_33
    r6 = _m_pfadd      (r6,r7);				        //; a_44 * b_14 + a_24 * b_12 | a_34 * b_13 + a_14 * b_11
    r2 = _m_pfmul      (r2,_m_from_floats (&b._14));//; a_43 * b_12 | a_33 * b11
    r6 = _m_pfacc      (r6,r6);				        //;		| a_44 * b_14 + a_24 * b_12 + a_34 * b_13 + a_14 * b_11
    r1 = _m_pfadd      (r1,r2);				        //; a_43 * b_12 + a_23 * b_12 | a_33 * b11 + a_13 * b_11
    tmp._14 = r6.m64_f32[0];
    tmp._13 = _m_pfacc (r1,r1).m64_f32[0];	        //;		| a_43 * b_12 + a_23 * b_12 + a_33 * b11 + a_13 * b_11

    r0 = _m_from_float (a._21);                     //;       | a_21
    r1 = _m_from_float (a._11);                     //;       | a_11
    r6 = _m_from_float (a._12);                     //;       | a_12
    r1 = _m_punpckldq  (r1,r0);                     //; a_21  | a_11  
    r5 = _m_from_float (a._22);                     //;       | a_22
    r1 = _m_pfmul      (r1,_m_from_floats (&b._21));//; a_21 * b_22 | a_11 * b_21     
    r6 = _m_punpckldq  (r6,r5);                     //; a_22  | a_12      
    r7 = _m_from_float (a._32);                     //;       | a_32
    r5 = _m_from_float (a._42);                     //;       | a_42
    r6 = _m_pfmul      (r6,_m_from_floats (&b._21));//; a_22 * b_22 | a_12 * b_21     
    r2 = _m_from_float (a._31);                     //;       | a_31
    r7 = _m_punpckldq  (r7,r5);                     //; a_42  | a_32
    r0 = _m_from_float (a._41);                     //;       | a_41
    r7 = _m_pfmul      (r7,_m_from_floats (&b._23));//; a_42 * b_24 | a_32 * b_23
	r2 = _m_punpckldq  (r2,r0);                     //; a_41  | a_31
    r6 = _m_pfadd      (r6,r7);				        //; a_42 * b_24 + a_22 * b_22 | a_32 * b_23 + a_12 * b_21
    r2 = _m_pfmul      (r2,_m_from_floats (&b._23));//; a_41 * b_24 | a_31 * b_23
    r6 = _m_pfacc      (r6,r6);				        //;       | a_42 * b_24 + a_22 * b_22 + a_32 * b_23 + a_12 * b_21
    r1 = _m_pfadd      (r1,r2);				        //; a_41 * b_24 + a_21 * b_22 | a_31 * b_23 + a_11 * b_21
    tmp._22 = r6.m64_f32[0];
    tmp._21 = _m_pfacc (r1,r1).m64_f32[0];	        //;		|a_41 * b_24 + a_21 * b_22 + a_31 * b_23 + a_11 * b_21

    r0 = _m_from_float (a._23);                     //;       | a_23
    r1 = _m_from_float (a._13);                     //;       | a_13
    r6 = _m_from_float (a._14);                     //;       | a_14
    r1 = _m_punpckldq  (r1,r0);                     //; a_23  | a_13  
    r5 = _m_from_float (a._24);                     //;       | a_24
    r1 = _m_pfmul      (r1,_m_from_floats (&b._21));//; a_23 * b_22 | a_13 * b_21 
    r6 = _m_punpckldq  (r6,r5);                     //; a_24  | a_14      
    r7 = _m_from_float (a._34);                     //;       | a_34
    r5 = _m_from_float (a._44);                     //;       | a_44
    r6 = _m_pfmul      (r6,_m_from_floats (&b._21));//; a_24 * b_22 | a_14 * b_21     
    r2 = _m_from_float (a._33);                     //;       | a_33
    r7 = _m_punpckldq  (r7,r5);                     //; a_44  | a_34
    r0 = _m_from_float (a._43);                     //;       | a_43
    r7 = _m_pfmul      (r7,_m_from_floats (&b._23));//; a_44 * b_24 | a_34 * b_23
    r2 = _m_punpckldq  (r2,r0);                     //; a_43  | a_33
    r6 = _m_pfadd      (r6,r7);				        //; a_24 * b_22 + a_44 * b_24 | a_14 * b_21 + a_34 * b_23
    r2 = _m_pfmul      (r2,_m_from_floats (&b._23));//; a_43 * b_24 | a_33 * b_23
    r6 = _m_pfacc      (r6,r6);				        //;		|a_24 * b_22 + a_44 * b_24 + a_14 * b_21 + a_34 * b_23
    r1 = _m_pfadd      (r1,r2);				        //; a_43 * b_24 + a_23 * b_22 | a_33 * b_23 + a_14 * b_21
    tmp._24 = r6.m64_f32[0];
    tmp._23 = _m_pfacc (r1,r1).m64_f32[0];	        //;		| a_43 * b_24 + a_23 * b_22 + a_33 * b_23 + a_14 * b_21

    r0 = _m_from_float (a._21);                     //;       | a_21
    r1 = _m_from_float (a._11);                     //;       | a_11
    r6 = _m_from_float (a._12);                     //;       | a_12
    r1 = _m_punpckldq  (r1,r0);                     //; a_21  | a_11  
    r5 = _m_from_float (a._22);                     //;       | a_22
    r1 = _m_pfmul      (r1,_m_from_floats (&b._31));//; a_21 * b_32 | a_11 * b_31     
    r6 = _m_punpckldq  (r6,r5);                     //; a_22  | a_12      
    r7 = _m_from_float (a._32);                     //;       | a_32
    r5 = _m_from_float (a._42);                     //;       | a_42
    r6 = _m_pfmul      (r6,_m_from_floats (&b._31));//; a_22 * b_32 | a_12 * b_31 
    r2 = _m_from_float (a._31);                     //;       | a_31
    r7 = _m_punpckldq  (r7,r5);                     //; a_42  | a_32
    r0 = _m_from_float (a._41);                     //;       | a_41
    r7 = _m_pfmul      (r7,_m_from_floats (&b._33));//; a_42 * b_34 | a_32 * b33
    r2 = _m_punpckldq  (r2,r0);                     //; a_41  | a_31
    r6 = _m_pfadd      (r6,r7);				        //; a_42 * b_34 + a_22 * b_32 | a_32 * b33 + a_12 * b_31 
    r2 = _m_pfmul      (r2,_m_from_floats (&b._33));//; a_41 * b_34 | a_31 * b33
    r6 = _m_pfacc      (r6,r6);				        //;		|a_42 * b_34 + a_22 * b_32 + a_32 * b33 + a_12 * b_31 
    r1 = _m_pfadd      (r1,r2);				        //; a_41 * b_34 + a_21 * b_32 | a_31 * b33 + a_11 * b_31
    tmp._32 = r6.m64_f32[0];
    tmp._31 = _m_pfacc (r1,r1).m64_f32[0]; 	        //;		|a_41 * b_34 + a_21 * b_32 + a_31 * b33 + a_11 * b_31

    r0 = _m_from_float (a._23);                     //;       | a_23
    r1 = _m_from_float (a._13);                     //;       | a_13
    r6 = _m_from_float (a._14);                     //;       | a_14
    r1 = _m_punpckldq  (r1,r0);                     //; a_23  | a_13  
    r5 = _m_from_float (a._24);                     //;       | a_24
    r1 = _m_pfmul      (r1,_m_from_floats (&b._31));//; a_21 * b_32 | a_11 * b_31     
    r6 = _m_punpckldq  (r6,r5);                     //; a_22  | a_12      
    r7 = _m_from_float (a._34);                     //;       | a_34
    r5 = _m_from_float (a._44);                     //;       | a_44
    r6 = _m_pfmul      (r6,_m_from_floats (&b._31));//; a_22 * b_32 | a_12 * b_31     
    r2 = _m_from_float (a._33);                     //;       | a_33
    r7 = _m_punpckldq  (r7,r5);                     //; a_42  | a_32
    r0 = _m_from_float (a._43);                     //;       | a_43
    r7 = _m_pfmul      (r7,_m_from_floats (&b._33));//; a_42 * b_34 | a_32 * b_33
    r2 = _m_punpckldq  (r2,r0);                     //; a_43  | a_33
    r6 = _m_pfadd      (r6,r7);				        //; a_42 * b_34 + a_22 * b_32 | a_32 * b_33 + a_12 * b_31
    r2 = _m_pfmul      (r2,_m_from_floats (&b._33));//; a_41 * b_34 | a_31 * b_33
    r6 = _m_pfacc      (r6,r6);				        //;		|a_42 * b_34 + a_22 * b_32 + a_32 * b_33 + a_12 * b_31
    r1 = _m_pfadd      (r1,r2);				        //; a_41 * b_34 + a_21 * b_32 | a_31 * b_33 + a_11 * b_31
    tmp._34 = r6.m64_f32[0];
    tmp._33 = _m_pfacc (r1,r1).m64_f32[0];	        //;		|a_41 * b_34 + a_21 * b_32 + a_31 * b_33 + a_11 * b_31

    r0 = _m_from_float (a._21);                     //;       | a_21
    r1 = _m_from_float (a._11);                     //;       | a_11
    r6 = _m_from_float (a._12);                     //;       | a_12
    r1 = _m_punpckldq  (r1,r0);                     //; a_21  | a_11  
    r5 = _m_from_float (a._22);                     //;       | a_22
    r1 = _m_pfmul      (r1,_m_from_floats (&b._41));//; a_21 * b_42 | a_11 * b_41     
    r6 = _m_punpckldq  (r6,r5);                     //; a_22  | a_12      
    r7 = _m_from_float (a._32);                     //;       | a_32
    r5 = _m_from_float (a._42);                     //;       | a_42
    r6 = _m_pfmul      (r6,_m_from_floats (&b._41));//; a_22 * b_42 | a_12 * b_41     
    r2 = _m_from_float (a._31);                     //;       | a_31
    r7 = _m_punpckldq  (r7,r5);                     //; a_42  | a_32
    r0 = _m_from_float (a._41);                     //;       | a_41
    r7 = _m_pfmul      (r7,_m_from_floats (&b._43));//; a_42 * b_44 | a_32 * b_43
    r2 = _m_punpckldq  (r2,r0);                     //; a_41  | a_31
    r6 = _m_pfadd      (r6,r7);				        //; a_42 * b_44 + a_22 * b_42 | a_32 * b_43 + a_12 * b_41
    r2 = _m_pfmul      (r2,_m_from_floats (&b._43));//; a_41 * b_44 | a_31 * b_43
    r6 = _m_pfacc      (r6,r6);				        //;		|a_42 * b_44 + a_22 * b_42 + a_32 * b_43 + a_12 * b_41
    r1 = _m_pfadd      (r1,r2);				        //; a_41 * b_44 + a_21 * b_42 | a_31 * b_43 + a_11 * b_41
    tmp._42 = r6.m64_f32[0];
    tmp._41 = _m_pfacc (r1,r1).m64_f32[0]; 	        //;		| a_41 * b_44 + a_21 * b_42 + a_31 * b_43 + a_11 * b_41


    r0 = _m_from_float (a._23);                     //;       | a_23
    r1 = _m_from_float (a._13);                     //;       | a_13
    r6 = _m_from_float (a._14);                     //;       | a_14
    r1 = _m_punpckldq  (r1,r0);                     //; a_23  | a_13  
    r5 = _m_from_float (a._24);                     //;       | a_24
    r1 = _m_pfmul      (r1,_m_from_floats (&b._41));//; a_21 * b_42 | a_11 * b_41     
    r6 = _m_punpckldq  (r6,r5);                     //; a_22  | a_12      
    r7 = _m_from_float (a._34);                     //;       | a_34
    r5 = _m_from_float (a._44);                     //;       | a_44
    r6 = _m_pfmul      (r6,_m_from_floats (&b._41));//; a_22 * b_42 | a_12 * b_41     
    r2 = _m_from_float (a._33);                     //;       | a_33
    r7 = _m_punpckldq  (r7,r5);                     //; a_42  | a_32
    r0 = _m_from_float (a._43);                     //;       | a_43
    r7 = _m_pfmul      (r7,_m_from_floats (&b._43));//; a_42 * b_44 | a_32 * b_43
    r2 = _m_punpckldq  (r2,r0);                     //; a_43  | a_33
    r6 = _m_pfadd      (r6,r7);				        //; a_42 * b_44 + a_22 * b_42 | a_32 * b_43 + a_12 * b_41
    r2 = _m_pfmul      (r2,_m_from_floats (&b._43));//; a_41 * b_44 | a_31 * b_43
    r6 = _m_pfacc      (r6,r6);				        //;		|a_42 * b_44 + a_22 * b_42 + a_32 * b_43 + a_12 * b_41
    r1 = _m_pfadd      (r1,r2);				        //; a_41 * b_44 + a_21 * b_42 | a_31 * b_43 + a_11 * b_41 
    tmp._44 = r6.m64_f32[0];
    tmp._43 = _m_pfacc  (r1,r1).m64_f32[0];		    //; a_41 * b_44 + a_21 * b_42 + a_31 * b_43 + a_11 * b_41 

    // Copy the results over
    __m64 *pres = (__m64 *)&r._11;
    pres[0] = _m_from_floats (&tmp._11);
    pres[1] = _m_from_floats (&tmp._13);
    pres[2] = _m_from_floats (&tmp._21);
    pres[3] = _m_from_floats (&tmp._23);
    pres[4] = _m_from_floats (&tmp._31);
    pres[5] = _m_from_floats (&tmp._33);
    pres[6] = _m_from_floats (&tmp._41);
    pres[7] = _m_from_floats (&tmp._43);

    _m_femms();
}

// eof
