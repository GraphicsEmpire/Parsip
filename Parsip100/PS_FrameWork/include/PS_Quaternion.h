//***************************************************************************
// Author: Pourya Shirazian 
// Quaternion math for efficient rotations. Use quaternions to avoid 
// gymbal lock problem.
//***************************************************************************
#ifndef PS_QUATERNION_H
#define PS_QUATERNION_H

#include <math.h>

#include "PS_Vector.h"
#include "PS_Matrix.h"

namespace PS{
namespace MATH{

// quaternions are always represent as type float.
// Represents 3d rotations as a quaternion number.
class CQuaternion
{
public:
    CQuaternion()
    {
        identity();
    };

    CQuaternion(float _x, float _y,float _z,float _w)
    {
        set(_x, _y, _z, _w);
    };

    CQuaternion(vec4f input)
    {
        set(input.x, input.y, input.z, input.w);
    }

    CQuaternion(vec3f _q, float _w)
    {
        q = _q;
        w = _w;
    }

    CQuaternion(const float *qt)
    {
        q.x = qt[0];
        q.y = qt[1];
        q.z = qt[2];
        w   = qt[3];
    }


    void set(float x, float y, float z, float _w)
    {
        q.set(x, y, z);
        w = _w;
    };

    void set(const float *quat)
    {
        q.x = quat[0];
        q.y = quat[1];
        q.z = quat[2];
        w   = quat[3];
    };


    void get(float *dest) const
    {
        dest[0] = q.x;
        dest[1] = q.y;
        dest[2] = q.z;
        dest[3] = w;
    };

    vec4f getAsVec4f() const
    {
        vec4f res(q.x, q.y, q.z, w);
        return res;
    }

    vec3f transform(vec3f p)
    {
        CQuaternion input(p, 0.0f);
        CQuaternion inv = inverse();

        CQuaternion res = multiply(input);
        res = res.multiply(inv);
        return res.q;
    }

    vec3f transform(const CQuaternion& inv, vec3f p)
    {
        CQuaternion input(p, 0.0f);
        CQuaternion res = multiply(input);
        res = res.multiply(inv);
        return res.q;
    }

    void setRotationAxis(float x, float y, float z)
    {
        q.set(x,y,z);
    };

    void setRotationAngle(float a)
    {
        w = a;
    };

    void identity(void)
    {
        q.set(0,0,0);
        w = 1;
    }

    bool binaryEqual(const CQuaternion &quat) const
    {
        const int *p1 = (const int *) &quat.q.x;
        const int *p2 = (const int *) &q.x;
        if ( p1[0] == p2[0] && p1[1] == p2[1] && p1[2] == p2[2] && p1[3] == p2[3] ) return true;
        return false;
    }

    //Convert a quaternion to a rotation matrix.
    void toMatrix(CMatrix &t) const;

    //Convert Quaternion to Euler
    void toEuler(float &roll, float &pitch, float &yaw) const;

    //Convert from Euler angles to quaternion.
    void fromEuler(float roll, float pitch, float yaw);

    void fromEuler(const vec3f &v)
    {
        fromEuler(v.x, v.y, v.z);
    };

    // Convert angle/axis into quaternion, and return rotation matrix.
    void fromAngleAxis(float radians,const vec3f &axis)
    {
        float halftheta    = radians*0.5f;
        float sinHalfTheta = (float)sin( halftheta );
        float cosHalfTheta = (float)cos( halftheta );

        q.x = axis.x*sinHalfTheta;
        q.y = axis.y*sinHalfTheta;
        q.z = axis.z*sinHalfTheta;
        w   = cosHalfTheta;
    };

    CQuaternion inverse() const
    {
        float square = q.x * q.x + q.y * q.y + q.z * q.z + w * w;
        if(square == 0.0f)
            square = 1.0f;
        float coeff = 1.0f/square;
        CQuaternion result(q * -1.0f * coeff, w * coeff);
        return result;
    }

    CQuaternion& operator-=(const CQuaternion &a)
    {
        q.x-=a.q.x;
        q.y-=a.q.y;
        q.z-=a.q.z;
        w-=a.w;
        Normalize();
        return *this;
    };

    CQuaternion& operator+=(const CQuaternion &a) // += operator.
    {
        q.x+=a.q.x;
        q.y+=a.q.y;
        q.z+=a.q.z;
        w+=a.w;
        return *this;
    };

    // Multiplying two quaternions adds their rotations.
    CQuaternion& operator*=(const CQuaternion &a)
    {
        CQuaternion r;
        r.q.x = w * a.q.x + q.x * a.w   + q.y * a.q.z - q.z * a.q.y;
        r.q.y = w * a.q.y + q.y * a.w   + q.z * a.q.x - q.x * a.q.z;
        r.q.z = w * a.q.z + q.z * a.w   + q.x * a.q.y - q.y * a.q.x;
        r.w   = w * a.w   - q.x * a.q.x - q.y * a.q.y - q.z * a.q.z;
        r.Normalize();

        this->q = r.q;
        this->w = r.w;
        return *this;
    };

    CQuaternion operator*(const CQuaternion &a)
    {
        CQuaternion r;
        r.q.x = w * a.q.x + q.x * a.w   + q.y * a.q.z - q.z * a.q.y;
        r.q.y = w * a.q.y + q.y * a.w   + q.z * a.q.x - q.x * a.q.z;
        r.q.z = w * a.q.z + q.z * a.w   + q.x * a.q.y - q.y * a.q.x;
        r.w   = w * a.w   - q.x * a.q.x - q.y * a.q.y - q.z * a.q.z;
        r.Normalize();
        return r;
    };


    bool operator==(const CQuaternion &rhs) const
    {
        return ( rhs.q.x == q.x && rhs.q.y == q.y && rhs.q.z == q.z && rhs.w == w );
    };

    bool operator!=(const CQuaternion &rhs) const
    {
        return ( rhs.q.x != q.x || rhs.q.y != q.y || rhs.q.z != q.z || rhs.w != w );
    };

    CQuaternion& operator = (const CQuaternion& rhs)          // ASSIGNMENT (=)
    {
        q = rhs.q;
        w = rhs.w;
        return(*this);
    };

    // Taking the reciprocal of a quaternion makes its rotation go the other way
    void  reciprocal( void )
    {
        q.x = -q.x;
        q.y = -q.y;
        q.z = -q.z;
    }

    CQuaternion multiply(const CQuaternion &b) const
    {
        CQuaternion res;
        res.q.x = b.w * q.x + b.q.x * w   + b.q.y * q.z - b.q.z * q.y;
        res.q.y = b.w * q.y + b.q.y * w   + b.q.z * q.x - b.q.x * q.z;
        res.q.z = b.w * q.z + b.q.z * w   + b.q.x * q.y - b.q.y * q.x;
        res.w   = b.w * w   - b.q.x * q.x - b.q.y * q.y - b.q.z * q.z;
        res.Normalize();
        return res;
    }

    void multiply(const CQuaternion &a, const CQuaternion &b)
    {
        q.x = b.w * a.q.x + b.q.x * a.w   + b.q.y * a.q.z - b.q.z * a.q.y;
        q.y = b.w * a.q.y + b.q.y * a.w   + b.q.z * a.q.x - b.q.x * a.q.z;
        q.z = b.w * a.q.z + b.q.z * a.w   + b.q.x * a.q.y - b.q.y * a.q.x;
        w   = b.w * a.w   - b.q.x * a.q.x - b.q.y * a.q.y - b.q.z * a.q.z;
        Normalize();
    }

    void getAxisAngle(vec3f &axis, float &angle) const // returns result in *DEGREES*
    {
        angle = acosf(w) * 2.0f;
        float sa = sqrtf(1.0f - w*w);
        if (sa)
        {
            axis.set(q.x/sa, q.y/sa, q.z/sa);
            angle *= RAD_TO_DEG;
        }
        else
        {
            axis.set(1,0,0);
        }
    }

    void  fromMatrix(const CMatrix &t); // convert rotation matrix to quaternion.
    void  Slerp(const CQuaternion &from,const CQuaternion &to,float t); // smooth interpolation.
    void  Lerp(const CQuaternion &from,const CQuaternion &to,float t);  // fast interpolation, not as smooth.

    void QuaternionInterpolate(float c0,float c1,const CQuaternion &quat)
    {
        // Find the cosine of the angle between the quaternions by
        // taking the inner product

        float CosineAngle = q.x * quat.q.x +
                q.y * quat.q.y +
                q.z * quat.q.z +
                w   * quat.w ;

        float Sign = 1.0f;

        if(CosineAngle < 0.0f)
        {
            CosineAngle = -CosineAngle;
            Sign = -Sign;
        }

        // TODO: Pick this constant
        float const SphericalLinearInterpolationThreshold = 0.0001f;

        if ( (1.0f - CosineAngle) > SphericalLinearInterpolationThreshold )
        {
            // TODO: Change this to use ATan2.
            // Fit for spherical interpolation
            float const Angle = acosf(CosineAngle);
            float const OneOverSinAngle = 1.0f / sinf(Angle);
            c0 = sinf(c0 * Angle) * Sign * OneOverSinAngle;
            c1 = sinf(c1 * Angle) * OneOverSinAngle;
        }
        else
        {
            c0 *= Sign;
        }
        q.x = q.x*c0 + quat.q.x * c1;
        q.y = q.y*c0 + quat.q.y * c1;
        q.z = q.z*c0 + quat.q.z * c1;
        w =   w*c0 + quat.w   * c1;

    }

    void NormalizeCloseToOne(void)
    {
        float sum = q.x * q.x + q.y * q.y + q.z * q.z + w * w;
        sum = (3.0f - sum ) * 0.5f; // this is an approximation
        q.x*=sum;
        q.y*=sum;
        q.z*=sum;
        w*=sum;
    }

    void  Normalize(void)  // normalize quaternion.
    {
        float	dist, square;

        square = q.x * q.x + q.y * q.y + q.z * q.z + w * w;
        if (square > 0.0f)
            dist = (float)(1.0f / sqrt(square));
        else
            dist = 1;

        q.x *= dist;
        q.y *= dist;
        q.z *= dist;
        w *= dist;
    };

    const float * GetFloat(void) const
    {
        return &q.x;
    };

    float * ptr(void)
    {
        return &q.x;
    };

    void RotationArc(const vec3f& v0, const vec3f& v1)
    {
        vec3f _v0 = v0;
        vec3f _v1 = v1;
        _v0.normalize();
        _v1.normalize();
        float s = sqrtf((1.0f + (v0.dot(v1))) * 2.0f);
        q = (_v0.cross(_v1)) / s;
        w = s * 0.5f;
    }

    void RandomRotation(bool x,bool y,bool z);

    // x/y/z components of quaternion.
    vec3f q;
    // w component of quaternion.
    float w;
};


// quaternion must be normalized and matrix t in column-major format
inline void CQuaternion::toMatrix(CMatrix &t) const
{
    float xx = q.x*q.x;
    float yy = q.y*q.y;
    float zz = q.z*q.z;
    float xy = q.x*q.y;
    float xz = q.x*q.z;
    float yz = q.y*q.z;
    float wx = w*q.x;
    float wy = w*q.y;
    float wz = w*q.z;

    t.mElement[0][0] = 1 - 2 * ( yy + zz );
    t.mElement[1][0] =     2 * ( xy - wz );
    t.mElement[2][0] =     2 * ( xz + wy );

    t.mElement[0][1] =     2 * ( xy + wz );
    t.mElement[1][1] = 1 - 2 * ( xx + zz );
    t.mElement[2][1] =     2 * ( yz - wx );

    t.mElement[0][2] =     2 * ( xz - wy );
    t.mElement[1][2] =     2 * ( yz + wx );
    t.mElement[2][2] = 1 - 2 * ( xx + yy );

    t.mElement[3][0] = t.mElement[3][1] = t.mElement[3][2] = 0.0f;
    t.mElement[0][3] = t.mElement[1][3] = t.mElement[2][3] = 0.0f;
    t.mElement[3][3] = 1.0f;
}




void  inline CQuaternion::fromMatrix(const CMatrix &t)
{
    float tr = t.mElement[0][0] + t.mElement[1][1] + t.mElement[2][2];
    // check the diagonal

    if (tr > 0.0f )
    {
        float s = (float) sqrt ( (double) (tr + 1.0f) );
        w = s * 0.5f;
        s = 0.5f / s;
        q.x = (t.mElement[1][2] - t.mElement[2][1]) * s;
        q.y = (t.mElement[2][0] - t.mElement[0][2]) * s;
        q.z = (t.mElement[0][1] - t.mElement[1][0]) * s;

    }
    else
    {
        // diagonal is negative
        int nxt[3] = {1, 2, 0};
        float  qa[4];

        int i = 0;

        if (t.mElement[1][1] > t.mElement[0][0]) i = 1;
        if (t.mElement[2][2] > t.mElement[i][i]) i = 2;

        int j = nxt[i];
        int k = nxt[j];

        float s = sqrtf ( ((t.mElement[i][i] - (t.mElement[j][j] + t.mElement[k][k])) + 1.0f) );

        qa[i] = s * 0.5f;

        if (s != 0.0f ) s = 0.5f / s;

        qa[3] = (t.mElement[j][k] - t.mElement[k][j]) * s;
        qa[j] = (t.mElement[i][j] + t.mElement[j][i]) * s;
        qa[k] = (t.mElement[i][k] + t.mElement[k][i]) * s;

        q.x = qa[0];
        q.y = qa[1];
        q.z = qa[2];
        w   = qa[3];
    }
};

// build quaternion based on euler angles
inline void CQuaternion::fromEuler(float roll,float pitch,float yaw)
{
    roll  *= 0.5f;
    pitch *= 0.5f;
    yaw   *= 0.5f;

    float cr = (float)cos(roll);
    float cp = (float)cos(pitch);
    float cy = (float)cos(yaw);

    float sr = (float)sin(roll);
    float sp = (float)sin(pitch);
    float sy = (float)sin(yaw);

    float cpcy = cp * cy;
    float spsy = sp * sy;
    float spcy = sp * cy;
    float cpsy = cp * sy;

    w   = cr * cpcy + sr * spsy;
    q.x = sr * cpcy - cr * spsy;
    q.y = cr * spcy + sr * cpsy;
    q.z = cr * cpsy - sr * spcy;
};

inline void CQuaternion::toEuler( float &roll, float &pitch, float &yaw ) const
{
    float sint		= (2.0f * w * q.y) - (2.0f * q.x * q.z);
    float cost_temp = 1.0f - (sint * sint);
    float cost		= 0;

    if ( (float)fabs(cost_temp) > 0.001f )
    {
        cost = sqrtf( cost_temp );
    }

    float sinv, cosv, sinf, cosf;
    if ( (float)fabs(cost) > 0.001f )
    {
        sinv = ((2.0f * q.y * q.z) + (2.0f * w * q.x)) / cost;
        cosv = (1.0f - (2.0f * q.x * q.x) - (2.0f * q.y * q.y)) / cost;
        sinf = ((2.0f * q.x * q.y) + (2.0f * w * q.z)) / cost;
        cosf = (1.0f - (2.0f * q.y * q.y) - (2.0f * q.z * q.z)) / cost;
    }
    else
    {
        sinv = (2.0f * w * q.x) - (2.0f * q.y * q.z);
        cosv = 1.0f - (2.0f * q.x * q.x) - (2.0f * q.z * q.z);
        sinf = 0;
        cosf = 1.0f;
    }

    // compute output rotations
    roll	= atan2( sinv, cosv );
    pitch	= atan2( sint, cost );
    yaw		= atan2( sinf, cosf );
}

typedef CQuaternion quat;
}
}
#endif
