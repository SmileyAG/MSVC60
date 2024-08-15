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

 QUAT_LIB.C

 AMD3D 3D library code: Quaternion math

  These routines provide a portable quaternion math library.  3DNow!
  accelerated versions of these routines can be found in QUAT.ASM.

  Loosly based on the quaternion library presented by Jeff Lander.
  Adapted to 3DNow! implementation and library conventions.

******************************************************************************/

#include <math.h>
#include "amath.h"
#include "aquat.h"

#define M_PI 3.1415927f
#define HALF_PI (M_PI*0.5f)
#define DEG2RAD (M_PI / (360.0f / 2.0f))
#define RAD2DEG ((360.0f / 2.0f) / M_PI)


/******************************************************************************
 Routine:   add_quat
 Input:     r - quaternion address
            a - quaternion address
            b - quaternion address
 Output:    r.i = a.i + b.i
******************************************************************************/
void add_quat (D3DRMQUATERNION *r, const D3DRMQUATERNION *a, const D3DRMQUATERNION *b)
{
    r->s   = a->s   + b->s;
    r->v.x = a->v.x + b->v.x;
    r->v.y = a->v.y + b->v.y;
    r->v.z = a->v.z + b->v.z;
}


/******************************************************************************
 Routine:   sub_quat
 Input:     r - quaternion address
            a - quaternion address
            b - quaternion address
 Output:    r.i = a.i - b.i
******************************************************************************/
void sub_quat (D3DRMQUATERNION *r, const D3DRMQUATERNION *a, const D3DRMQUATERNION *b)
{
    r->s   = a->s   - b->s;
    r->v.x = a->v.x - b->v.x;
    r->v.y = a->v.y - b->v.y;
    r->v.z = a->v.z - b->v.z;
}


/******************************************************************************
 Routine:   mult_quat
 Input:     r - quaternion address
            a - quaternion address
            b - quaternion address
 Output:    r = a * b, using the definition of quaternion multiplication
******************************************************************************/
void mult_quat (D3DRMQUATERNION *r, const D3DRMQUATERNION *a, const D3DRMQUATERNION *b)
{
    // Do this to a temporary variable in case the output aliases an input
    D3DRMQUATERNION tmp;
    tmp.s   = a->s   * b->s - a->v.x * b->v.x - a->v.y * b->v.y - a->v.z * b->v.z;
    tmp.v.x = a->v.x * b->s + b->v.x * a->s   + a->v.y * b->v.z - a->v.z * b->v.y;
    tmp.v.y = a->v.y * b->s + b->v.y * a->s   + a->v.z * b->v.x - a->v.x * b->v.z;
    tmp.v.z = a->v.z * b->s + b->v.z * a->s   + a->v.x * b->v.y - a->v.y * b->v.x;

    *r = tmp;
}


/******************************************************************************
 Routine:   norm_quat
 Input:     dst  - quaternion address
            quat - quaternion address
 Output:    dst  = normalize(quat) such that (x^2 + y^2 + z^2 + w^2 = 1)
******************************************************************************/
void norm_quat (D3DRMQUATERNION *dst, const D3DRMQUATERNION *a)
{
    const float quatx = a->v.x,
                quaty = a->v.y,
                quatz = a->v.z,
                quatw = a->s;
    const float magnitude = 1.0f / (float)sqrt((quatx * quatx) +
                                               (quaty * quaty) +
                                               (quatz * quatz) +
                                               (quatw * quatw));

    dst->v.x = quatx * magnitude;
    dst->v.y = quaty * magnitude;
    dst->v.z = quatz * magnitude;
    dst->s   = quatw * magnitude;
}


/******************************************************************************
 Routine:   D3DMat2quat
 Input:     m    - matrix (4x4) address
            quat - quaternion address
 Output:    quat = quaternion from rotation matrix 'm'
******************************************************************************/
void D3DMat2quat (D3DRMQUATERNION *quat, const D3DMATRIX *m)
{
    float   *mm = (float *)&m;
    float   tr, s;

    tr = m->_11 + m->_22 + m->_33;

    // check the diagonal
    if (tr > 0.0)
    {
        s = (float)sqrt (tr + 1.0f);
        quat->s = s * 0.5f;
        s = 0.5f / s;
        quat->v.x = (m->_32 - m->_23) * s;
        quat->v.y = (m->_13 - m->_31) * s;
        quat->v.z = (m->_21 - m->_12) * s;
    }
    else
    {
        if (m->_22 > m->_11 && m->_33 <= m->_22)
        {
            s = (float)sqrt ((m->_22 - (m->_33 + m->_11)) + 1.0f);

            quat->v.x = s * 0.5f;

            if (s != 0.0)
                s = 0.5f / s;

            quat->v.z = (m->_13 - m->_31) * s;
            quat->v.y = (m->_32 + m->_23) * s;
            quat->s   = (m->_12 + m->_21) * s;
        }
        else if ((m->_22 <= m->_11  &&  m->_33 > m->_11)  ||  (m->_33 > m->_22))
        {
            s = (float)sqrt ((m->_33 - (m->_11 + m->_22)) + 1.0f);

            quat->v.y = s * 0.5f;

            if (s != 0.0)
                s = 0.5f / s;

            quat->v.z = (m->_21 - m->_12) * s;
            quat->s   = (m->_13 + m->_31) * s;
            quat->v.x = (m->_23 + m->_32) * s;
        }
        else
        {
            s = (float)sqrt ((m->_11 - (m->_22 + m->_33)) + 1.0f);

            quat->s = s * 0.5f;

            if (s != 0.0)
                s = 0.5f / s;

            quat->v.z = (m->_32 - m->_23) * s;
            quat->v.x = (m->_21 + m->_12) * s;
            quat->v.y = (m->_31 + m->_13) * s;
        }

#if 0
        // diagonal is negative
        i = 0;
        if (m->_22 > m->_11) i = 1;
        if (m->_33 > mm[i*4+i]) i = 2;
        j = nxt[i];
        k = nxt[j];

        s = (float)sqrt ((mm[i*4+i] - (mm[j*4+j] + mm[k*4+k])) + 1.0f);

        q[i] = s * 0.5f;

        if (s != 0.0)
            s = 0.5f / s;

        q[3] = (mm[j*4+k] - mm[k*4+j]) * s;
        q[j] = (mm[i*4+j] + mm[j*4+i]) * s;
        q[k] = (mm[i*4+k] + mm[k*4+i]) * s;

        *quat = q;
#endif
    }
}


/******************************************************************************
 Routine:   glMat2quat
 Input:     m    - matrix (4x4) address
            quat - quaternion address
 Output:    quat = quaternion from rotation matrix 'm'
******************************************************************************/
void glMat2quat (D3DRMQUATERNION *quat, const float *m)
{
    float   *mm = (float *)&m;
    float   tr, s;

    tr = m[0] + m[5] + m[10];

    // check the diagonal
    if (tr > 0.0)
    {
        s = (float)sqrt (tr + 1.0f);
        quat->s = s * 0.5f;
        s = 0.5f / s;
        quat->v.x = (m[6] - m[9]) * s;
        quat->v.y = (m[8] - m[2]) * s;
        quat->v.z = (m[1] - m[4]) * s;
    }
    else
    {
        if (m[5] > m[0] && m[10] <= m[5])
        {
            s = (float)sqrt ((m[5] - (m[10] + m[0])) + 1.0f);

            quat->v.x = s * 0.5f;

            if (s != 0.0)
                s = 0.5f / s;

            quat->v.z = (m[8] - m[2]) * s;
            quat->v.y = (m[6] + m[9]) * s;
            quat->s   = (m[4] + m[1]) * s;
        }
        else if ((m[5] <= m[0]  &&  m[10] > m[0])  ||  (m[10] > m[5]))
        {
            s = (float)sqrt ((m[10] - (m[0] + m[5])) + 1.0f);

            quat->v.y = s * 0.5f;

            if (s != 0.0)
                s = 0.5f / s;

            quat->v.z = (m[1] - m[4]) * s;
            quat->s   = (m[8] + m[2]) * s;
            quat->v.x = (m[9] + m[6]) * s;
        }
        else
        {
            s = (float)sqrt ((m[0] - (m[5] + m[10])) + 1.0f);

            quat->s = s * 0.5f;

            if (s != 0.0)
                s = 0.5f / s;

            quat->v.z = (m[6] - m[9]) * s;
            quat->v.x = (m[1] + m[4]) * s;
            quat->v.y = (m[2] + m[8]) * s;
        }
    }
}


/******************************************************************************
 Routine:   quat2D3DMat
 Input:     quat - quaternion to convert
            m    - matrix (4x4) to fill
 Output:    m    = rotation matrix from quat
******************************************************************************/
void quat2D3DMat (D3DMATRIX *m, const D3DRMQUATERNION *quat)
{
    float wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

    // calculate coefficients
    x2 = quat->v.x + quat->v.x;
    y2 = quat->v.y + quat->v.y; 
    z2 = quat->v.z + quat->v.z;

    xx = quat->v.x * x2;   xy = quat->v.x * y2;   xz = quat->v.x * z2;
    yy = quat->v.y * y2;   yz = quat->v.y * z2;   zz = quat->v.z * z2;
    wx = quat->s   * x2;   wy = quat->s   * y2;   wz = quat->s   * z2;

    m->_11 = 1.0f - yy - zz;
    m->_12 = xy - wz;
    m->_13 = xz + wy;
    m->_21 = xy + wz;
    m->_22 = 1.0f - xx - zz;
    m->_23 = yz - wx;
    m->_31 = xz - wy;
    m->_32 = yz + wx;
    m->_33 = 1.0f - xx - yy;

    m->_14 =
    m->_24 =
    m->_34 =
    m->_41 =
    m->_42 =
    m->_43 = 0.0f;
    m->_44 = 1.0f;
}


/******************************************************************************
 Routine:   quat2glMat
 Input:     quat - quaternion to convert
            m    - matrix (4x4) to fill
 Output:    m    = rotation matrix from quat
******************************************************************************/
void quat2glMat (float *m, const D3DRMQUATERNION *quat)
{
    float wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

    // calculate coefficients
    x2 = quat->v.x + quat->v.x;
    y2 = quat->v.y + quat->v.y; 
    z2 = quat->v.z + quat->v.z;

    xx = quat->v.x * x2;   xy = quat->v.x * y2;   xz = quat->v.x * z2;
    yy = quat->v.y * y2;   yz = quat->v.y * z2;   zz = quat->v.z * z2;
    wx = quat->s   * x2;   wy = quat->s   * y2;   wz = quat->s   * z2;

    m[0] = 1.0f - yy - zz;
    m[1] = xy + wz;
    m[2] = xz - wy;

    m[4] = xy - wz;
    m[5] = 1.0f - xx - zz;
    m[6] = yz + wx;

    m[8] = xz + wy;
    m[9] = yz - wx;
    m[10] = 1.0f - xx - yy;

    m[3] =
    m[7] =
    m[11] =
    m[12] =
    m[13] =
    m[14] = 0.0f;
    m[15] = 1.0f;
}


/******************************************************************************
 Routine:   euler2quat
 Input:     rot  - float address, <pitch, yaw, roll> angles
            quat - quaternion address
 Output:    quat = rotation specified by 'rot'
******************************************************************************/
void euler2quat (D3DRMQUATERNION *quat, const float *rot)
{
    float csx[2],csy[2],csz[2],cc,cs,sc,ss, cy, sy;

    // Convert angles to radians/2, construct the quat axes
    if (rot[0] == 0.0f)
    {
        csx[0] = 1.0f;
        csx[1] = 0.0f;
    }
    else
    {
        const float deg = rot[0] * (DEG2RAD * 0.5f);
        csx[0] = (float)cos (deg);
        csx[1] = (float)sin (deg);
    }

    if (rot[2] == 0.0f)
    {
        cc = csx[0];
        ss = 0.0f;
        cs = 0.0f;
        sc = csx[1];
    }
    else
    {
        const float deg = rot[2] * (DEG2RAD * 0.5f);
        csz[0] = (float)cos (deg);
        csz[1] = (float)sin (deg);
        cc = csx[0] * csz[0];
        ss = csx[1] * csz[1];
        cs = csx[0] * csz[1];
        sc = csx[1] * csz[0];
    }

    if (rot[1] == 0.0f)
    {
        quat->v.x = sc;
        quat->v.y = ss;
        quat->v.z = cs;
        quat->s   = cc;
    }
    else
    {
        const float deg = rot[1] * (DEG2RAD * 0.5f);
        cy = csy[0] = (float)cos (deg);
        sy = csy[1] = (float)sin (deg);
        quat->v.x = (cy * sc) - (sy * cs);
        quat->v.y = (cy * ss) + (sy * cc);
        quat->v.z = (cy * cs) - (sy * sc);
        quat->s   = (cy * cc) + (sy * ss);
    }

    // should be normal, if sin and cos are accurate enough
}


/******************************************************************************
 Routine:   euler2quat2
 Input:     rot  - float address, <pitch, yaw, roll> angles
            quat - quaternion address
 Output:    quat = rotation specified by 'rot'
 Comment:   construct a quaternion by applying the given euler angles
            in X-Y-Z order (pitch-yaw-roll). Less efficient than euler2quat(),
            but more easily modified for different rotation orders.
            Note that euler2quat() was derived by manually inlining the
            mult_quat calls, factoring out zero terms, and working some
            basic (but extensive) algebra on the results.  If you find that
            you need a converter that does not use XYZ order, it might be
            worth taking the time to craft a custom version using a similar
            technique.
******************************************************************************/
void euler2quat2 (D3DRMQUATERNION *quat, const float *rot)
{
    D3DRMQUATERNION qx, qy, qz, qf;
    float   deg;

    // Convert angles to radians (and half-angles), and compute partial quats
    deg = rot[0] * 0.5f * DEG2RAD;
    qx.v.x = (float)sin (deg);
    qx.v.y = 0.0f; 
    qx.v.z = 0.0f; 
    qx.s   = (float)cos (deg);

    deg = rot[1] * 0.5f * DEG2RAD;
    qy.v.x = 0.0f; 
    qy.v.y = (float)sin (deg);
    qy.v.z = 0.0f;
    qy.s   = (float)cos (deg);

    deg = rot[2] * 0.5f * DEG2RAD;
    qz.v.x = 0.0f;
    qz.v.y = 0.0f;
    qz.v.z = (float)sin (deg);
    qz.s   = (float)cos (deg);

    mult_quat (&qf, &qy, &qx);
    mult_quat (quat, &qz, &qf);

    // should be normal, if sin/cos and quat_mult are accurate enough
}


/******************************************************************************
 Routine:   quat2axis_angle
 Input:     quat      - quaternion address
            axisAngle - float[4] <x,y,z,angle>
 Output:    axisAngle representation of 'quat'
******************************************************************************/
void quat2axis_angle (float *axisAngle, const D3DRMQUATERNION *quat)
{
    const float tw = (float)acos (quat->s);
    const float scale = (float)(1.0 / sin (tw));

    axisAngle[3] = tw * 2.0f * RAD2DEG;
    axisAngle[0] = quat->v.x * scale;
    axisAngle[1] = quat->v.y * scale;
    axisAngle[2] = quat->v.z * scale;
}


/******************************************************************************
 Routine:   _axis_angle2quat
 Input:     axisAngle - axis address (1x4 of <x,y,z,degrees>)
            quat      - quaternion address
 Output:    quat      = rotation around given axis
******************************************************************************/
void axis_angle2quat (D3DRMQUATERNION *quat, const float *axisAngle)
{
    const float deg = axisAngle[3] / (2.0f * RAD2DEG);
    const float cs = (float)cos (deg);
    quat->s = (float)sin (deg);
    quat->v.x = cs * axisAngle[0];
    quat->v.y = cs * axisAngle[1];
    quat->v.z = cs * axisAngle[2];
}


#define DELTA   0.0001      // DIFFERENCE AT WHICH TO LERP INSTEAD OF SLERP
/******************************************************************************
 Routine:   _slerp_quat
 Input:     quat1  - quaternion address
            quat2  - quaternion address
            slerp  - float interp factor (0.0 = quat1, 1.0 = quat2)
            result - quaternion address
 Output:    result = quaternion spherically interpolate between q1 and q2
******************************************************************************/
void slerp_quat (D3DRMQUATERNION *result,
                 const D3DRMQUATERNION *quat1, const D3DRMQUATERNION *quat2,
                 float slerp)
{
    double omega, cosom, isinom;
    float scale0, scale1;
    float q2x, q2y, q2z, q2w;

    // DOT the quats to get the cosine of the angle between them
    cosom = quat1->v.x * quat2->v.x +
            quat1->v.y * quat2->v.y +
            quat1->v.z * quat2->v.z +
            quat1->s   * quat2->s;

    // Two special cases:
    // Quats are exactly opposite, within DELTA?
    if (cosom > DELTA - 1.0)
    {
        // make sure they are different enough to avoid a divide by 0
        if (cosom < 1.0 - DELTA)
        {
            // SLERP away
            omega = acos (cosom);
            isinom = 1.0 / sin (omega);
            scale0 = (float)(sin ((1.0 - slerp) * omega) * isinom);
            scale1 = (float)(sin (slerp * omega) * isinom);
        }
        else
        {
            // LERP is good enough at this distance
            scale0 = 1.0f - slerp;
            scale1 = slerp;
        }

        q2x = quat2->v.x * scale1;
        q2y = quat2->v.y * scale1;
        q2z = quat2->v.z * scale1;
        q2w = quat2->s   * scale1;
    }
    else
    {
        // SLERP towards a perpendicular quat
        // Set slerp parameters
        scale0 = (float)sin ((1.0f - slerp) * HALF_PI);
        scale1 = (float)sin (slerp * HALF_PI);

        q2x = -quat2->v.y * scale1;
        q2y =  quat2->v.x * scale1;
        q2z = -quat2->s   * scale1;
        q2w =  quat2->v.z * scale1;
    }

    // Compute the result
    result->v.x = scale0 * quat1->v.x + q2x;
    result->v.y = scale0 * quat1->v.y + q2y;
    result->v.z = scale0 * quat1->v.z + q2z;
    result->s   = scale0 * quat1->s   + q2w;
}


/******************************************************************************
 Routine:   _trans_quat
 Input:     result - vector 3x1 address
            q      - quaternion address
            v      - vector 3x1 address
 Output:    d      = 'a' rotated with 'b'
 Comments:  Note that this is equivalent to using quat2mat to make a rotation
            matrix, and then multiplying the vector by the matrix.  This form
            is more compact, and equally efficient when only transforming a
            single vector.  For other cases, it is advisable to construct
            a rotation matrix.
******************************************************************************/
void trans_quat (D3DVECTOR *result, const D3DVECTOR *v, const D3DRMQUATERNION *q)
{
    // result = av + bq + c(q.v CROSS v)
    // where
    //  a = q.w^2 - (q.v DOT q.v)
    //  b = 2 * (q.v DOT v)
    //  c = 2q.w
    float   w = q->s;   // just a convenience name
    float   a = w * w - (q->v.x * q->v.x + q->v.y * q->v.y + q->v.z * q->v.z);
    float   b = 2.0f  * (q->v.x * v->x   + q->v.y * v->y   + q->v.z * v->z);
    float   c = 2.0f  * w;

    // Must store this, because result may alias v
    float cross[3]; // q.v CROSS v
    cross[0] = q->v.y * v->z - q->v.z * v->y;
    cross[1] = q->v.z * v->x - q->v.x * v->z;
    cross[2] = q->v.x * v->y - q->v.y * v->x;

    result->x = a * v->x + b * q->v.x + c * cross[0];
    result->y = a * v->y + b * q->v.y + c * cross[1];
    result->z = a * v->z + b * q->v.z + c * cross[2];
}


// eof - quat_lib.c
