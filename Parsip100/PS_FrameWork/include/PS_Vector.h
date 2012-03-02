//***************************************************************************
// Author: Pourya Shirazian 
// 2,3,4 Dimensional generic vector classes. many necessary operations
// have been implemented in each class.
//***************************************************************************

#pragma once
#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>
#include <float.h>
#include <vector>

#include "mathHelper.h"

namespace PS{
	namespace MATH{

//////////////////////////////////////////////////////////////////////////
template <class Type> 
class Vector2d
{
public:
	Vector2d(void) 
	{
		x = y = 0;
	};  

	Vector2d(const Vector2d &a) // constructor copies existing vector.
	{
		x = a.x;
		y = a.y;
	};

	Vector2d(const float *t)
	{
		x = t[0];
		y = t[1];
	};


	Vector2d(Type a,Type b) // construct with initial point.
	{
		x = a;
		y = b;
	};

	const Type* Ptr() const { return &x; }
	Type* Ptr() { return &x; }

	Vector2d & operator+=(const Vector2d &a) // += operator.
	{
		x+=a.x;
		y+=a.y;
		return *this;
	};

	Vector2d & operator-=(const Vector2d &a)
	{
		x-=a.x;
		y-=a.y;
		return *this;
	};

	Vector2d & operator*=(const Vector2d &a)
	{
		x*=a.x;
		y*=a.y;
		return *this;
	};

	Vector2d & operator/=(const Vector2d &a)
	{
		x/=a.x;
		y/=a.y;
		return *this;
	};

	bool operator==(const Vector2d<Type> &a) const
	{
		return (FLOAT_EQ(a.x, x)&&FLOAT_EQ(a.y, y));
	};

	bool operator!=(const Vector2d &a) const
	{
		return !(FLOAT_EQ(a.x, x)&&FLOAT_EQ(a.y, y));
	};

	Vector2d operator+(Vector2d a) const
	{
		a.x+=x;
		a.y+=y;
		return a;
	};

	Vector2d operator-(Vector2d a) const
	{
		a.x = x-a.x;
		a.y = y-a.y;
		return a;
	};

	Vector2d operator - (void) const
	{
		return negative();
	};

	Vector2d operator*(Vector2d a) const
	{
		a.x*=x;
		a.y*=y;
		return a;
	};

	Vector2d operator*(Type c) const
	{
		Vector2d<Type> a;

		a.x = x * c;
		a.y = y * c;

		return a;
	};

	Vector2d operator/(Vector2d a) const
	{
		a.x = x/a.x;
		a.y = y/a.y;
		return a;
	};


	Type Dot(const Vector2d<Type> &a) const        // computes dot product.
	{
		return (x * a.x + y * a.y );
	};

	Type GetX(void) const { return x; };
	Type GetY(void) const { return y; };

	void SetX(Type t) { x   = t; };
	void SetY(Type t) { y   = t; };

	float sum()
	{
		return x + y;
	}

	void Set(const int *p)
	{
		x = (Type) p[0];
		y = (Type) p[1];
	}

	void Set(const float *p)
	{
		x = (Type) p[0];
		y = (Type) p[1];
	}


	void Set(Type a,Type b)
	{
		x = a;
		y = b;
	};

	void Zero(void)
	{
		x = y = 0;
	};

	Vector2d negative(void) const
	{
		Vector2d result;
		result.x = -x;
		result.y = -y;
		return result;
	}

	Type magnitude(void) const
	{
		return (Type) sqrtf(x * x + y * y );
	}

	Type fastmagnitude(void) const
	{
		return (Type) fast_sqrt(x * x + y * y );
	}

	Type fastermagnitude(void) const
	{
		return (Type) faster_sqrt( x * x + y * y );
	}

	void Reflection(Vector2d &a,Vector2d &b); // compute reflection vector.

	Type Length(void) const          // length of vector.
	{
		return Type(sqrtf( x*x + y*y ));
	};

	Type FastLength(void) const          // length of vector.
	{
		return Type(fast_sqrt( x*x + y*y ));
	};

	Type FasterLength(void) const          // length of vector.
	{
		return Type(faster_sqrt( x*x + y*y ));
	};

	Type Length2(void)        // squared distance, prior to square root.
	{
		return x*x+y*y;
	}

	Type Distance(const Vector2d &a) const   // distance between two points.
	{
		Type dx = a.x - x;
		Type dy = a.y - y;
		Type d  = dx*dx+dy*dy;
		return sqrtf(d);
	};

	Type FastDistance(const Vector2d &a) const   // distance between two points.
	{
		Type dx = a.x - x;
		Type dy = a.y - y;
		Type d  = dx*dx+dy*dy;
		return fast_sqrt(d);
	};


	Type FasterDistance(const Vector2d &a) const   // distance between two points.
	{
		Type dx = a.x - x;
		Type dy = a.y - y;
		Type d  = dx*dx+dy*dy;
		return faster_sqrt(d);
	};

	Type Distance2(Vector2d &a) // squared distance.
	{
		Type dx = a.x - x;
		Type dy = a.y - y;
		return dx*dx + dy *dy;
	};

	void Lerp(const Vector2d<Type>& from,const Vector2d<Type>& to,float slerp)
	{
		x = ((to.x - from.x)*slerp) + from.x;
		y = ((to.y - from.y)*slerp) + from.y;
	};


	void Cross(const Vector2d<Type> &a,const Vector2d<Type> &b)  // cross two vectors result in this one.
	{
		x = a.y*b.x - a.x*b.y;
		y = a.x*b.x - a.x*b.x;
	};

	Type Normalize(void)       // normalize to a unit vector, returns distance.
	{
		Type l = Length();
		if ( l != 0 )
		{
			l = Type( 1 ) / l;
			x*=l;
			y*=l;
		}
		else
		{
			x = y = 0;
		}
		return l;
	};

	Type FastNormalize(void)       // normalize to a unit vector, returns distance.
	{
		Type l = FastLength();
		if ( l != 0 )
		{
			l = Type( 1 ) / l;
			x*=l;
			y*=l;
		}
		else
		{
			x = y = 0;
		}
		return l;
	};

	Type FasterNormalize(void)       // normalize to a unit vector, returns distance.
	{
		Type l = FasterLength();
		if ( l != 0 )
		{
			l = Type( 1 ) / l;
			x*=l;
			y*=l;
		}
		else
		{
			x = y = 0;
		}
		return l;
	};


	Type x;
	Type y;
};

//////////////////////////////////////////////////////////////////////////
template <class Type> 
class Vector3d
{
public:
	Vector3d(void) 
	{
		x = y = z = 0;
	};  

	Vector3d(const Vector3d &a) // constructor copies existing vector.
	{
		x = a.x;
		y = a.y;
		z = a.z;
	};

	Vector3d(Type a,Type b,Type c) // construct with initial point.
	{
		x = a;
		y = b;
		z = c;		
	};

	Vector3d(const float *t)
	{
		x = t[0];
		y = t[1];
		z = t[2];		
	};

	Vector3d(const double *t)
	{
		x = (float)t[0];
		y = (float)t[1];
		z = (float)t[2];	
	};

	Vector3d(const int *t)
	{
		x = t[0];
		y = t[1];
		z = t[2];
	};

	bool operator==(const Vector3d<Type> &a) const
	{
		//return( a.x == x && a.y == y && a.z == z );
		return (FLOAT_EQ(a.x, x)&&FLOAT_EQ(a.y, y)&&FLOAT_EQ(a.z, z));
	};

	bool operator!=(const Vector3d<Type> &a) const
	{
		//return( a.x != x || a.y != y || a.z != z );
		return !(FLOAT_EQ(a.x, x)&&FLOAT_EQ(a.y, y)&&FLOAT_EQ(a.z, z));
	};

// Operators
		Vector3d& operator = (const Vector3d& A)          // ASSIGNMENT (=)
		{ 
			x=A.x; y=A.y; z=A.z; 			
			return(*this);  
		};

		Vector3d operator + (const Vector3d& A) const     // ADDITION (+)
		{ 
			Vector3d Sum(x+A.x, y+A.y, z+A.z);
			return(Sum); 
		};

		Vector3d operator + (const float s) const       // ADD CONSTANT TO ALL 3 COMPONENTS 
		{ 
			Vector3d Scaled(x+s, y+s, z+s);
			return(Scaled); 
		};


		Vector3d operator - (const Vector3d& A) const     // SUBTRACTION (-)
		{ 
			Vector3d Diff(x-A.x, y-A.y, z-A.z);
			return(Diff); 
		};

		Vector3d operator - (const float d) const     // SUBTRACTION (-)
		{ 
			Vector3d Diff(x-d, y-d, z-d);
			return(Diff); 
		};


		Vector3d operator * (const Vector3d& A) const
		{
			Vector3d scaled(x * A.x, y * A.y, z * A.z);
			return scaled;
		};


		Vector3d operator * (const float s) const       // MULTIPLY BY SCALAR (*)
		{ 
			Vector3d Scaled(x*s, y*s, z*s);
			return(Scaled); 
		};

		Vector3d operator / (const float s) const       // DIVIDE BY SCALAR (/)
		{
			float r = 1.0f / s;
			Vector3d Scaled(x*r, y*r, z*r);
			return(Scaled);
		};

		void operator /= (Type A)             // ACCUMULATED VECTOR ADDITION (/=)
		{ 
			x/=A; y/=A; z/=A; 
		};

		void operator += (const Vector3d A)             // ACCUMULATED VECTOR ADDITION (+=)
		{ 
			x+=A.x; y+=A.y; z+=A.z; 
		};

		void operator -= (const Vector3d A)             // ACCUMULATED VECTOR SUBTRACTION (+=)
		{ 
			x-=A.x; y-=A.y; z-=A.z; 
		};

		void operator *= (const float s)        // ACCUMULATED SCALAR MULTIPLICATION (*=) (bpc 4/24/2000)
		{
			x*=s; y*=s; z*=s;
		}
		void operator *= (const Vector3d A)
		{
			x *= A.x;
			y *= A.y;
			z *= A.z;
		}

		void operator += (const float A)             // ACCUMULATED VECTOR ADDITION (+=)
			{ x+=A; y+=A; z+=A; };


		Vector3d operator - (void) const                // NEGATION (-)
			{ Vector3d Negated(-x, -y, -z);
				return(Negated); };

		Type operator [] (const int i) const         // ALLOWS VECTOR ACCESS AS AN ARRAY.
			{ return( (i==0)?x:((i==1)?y:z) ); };
		Type & operator [] (const int i)
			{ return( (i==0)?x:((i==1)?y:z) ); };
//

	bool isSame(const Vector3d<float> &v,float epsilon) const
	{
		float dx = fabsf( x - v.x );
		if ( dx > epsilon ) return false;
		float dy = fabsf( y - v.y );
		if ( dy > epsilon ) return false;
		float dz = fabsf( z - v.z );
		if ( dz > epsilon ) return false;
		return true;
	}


	float computeNormal(const Vector3d<float> &A,
										 const Vector3d<float> &B,
										 const Vector3d<float> &C)
	{
		float vx,vy,vz,wx,wy,wz,vw_x,vw_y,vw_z,mag;

		vx = (B.x - C.x);
		vy = (B.y - C.y);
		vz = (B.z - C.z);

		wx = (A.x - B.x);
		wy = (A.y - B.y);
		wz = (A.z - B.z);

		vw_x = vy * wz - vz * wy;
		vw_y = vz * wx - vx * wz;
		vw_z = vx * wy - vy * wx;

		mag = sqrtf((vw_x * vw_x) + (vw_y * vw_y) + (vw_z * vw_z));

		if ( mag < 0.000001f )
		{
			mag = 0;
		}
		else
		{
			mag = 1.0f/mag;
		}

		x = vw_x * mag;
		y = vw_y * mag;
		z = vw_z * mag;

		return mag;
	}

	bool isUnit() const 
	{
                return FLOAT_EQ(1.0f, float(length2()));
	}

	bool isZero() const
	{
                return FLOAT_EQ(0.0f, float(length2()));
	}

	Vector3d<Type> add(Type displacement)
	{
		x += displacement;
		y += displacement;
		z += displacement;
		return *this;
	}

	Vector3d<Type> add(const Vector3d<Type> &other)
	{
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}

	Vector3d<Type> subtract(Type displacement)
	{
		x -= displacement;
		y -= displacement;
		z -= displacement;
		return *this;
	}

	Vector3d<Type> subtract(const Vector3d<Type> &other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}

	Vector3d<Type> multiply(Type m)
	{
		x *= m;
		y *= m;
		z *= m;
		return *this;
	}

	Vector3d<Type> multiply(const Vector3d<Type> &other)
	{
		x *= other.x;
		y *= other.y;
		z *= other.z;
		return *this;
	}

	float getAngleDeg(const Vector3d<float> &other) const
	{		 
		return RADTODEG(getAngleRad(other));
	}

	float getAngleRad(const Vector3d<float> &other) const
	{
		float dd = dot(other);
		Clampf(dd, -1.0f, 1.0f);

		float rad_angle = float(acos(dd));

		return rad_angle;
	}

	void ScaleSumScale(float c0,float c1,const Vector3d<float> &pos)
	{
		x = (x*c0) + (pos.x*c1);
		y = (y*c0) + (pos.y*c1);
		z = (z*c0) + (pos.z*c1);
	}

	void SwapYZ(void)
	{
		float t = y;
		y = z;
		z = t;
	};

	void get(Type *v) const
	{
		v[0] = x;
		v[1] = y;
		v[2] = z;
	};

	void set(const int *p)
	{
		x = (Type) p[0];
		y = (Type) p[1];
		z = (Type) p[2];
	}

	void set(const float *p)
	{
		x = (Type) p[0];
		y = (Type) p[1];
		z = (Type) p[2];
	}


	void set(Type a,Type b,Type c)
	{
		x = a;
		y = b;
		z = c;
	};

	float sum()
	{
		return x + y + z;
	}

	void zero(void)
	{
		x = y = z = 0;
	};
	

	const Type* ptr() const { return &x; }
	Type* ptr() { return &x; }

	Vector3d<Type> vectorAbs()
	{
		Vector3d result;
		result.x = abs(x);
		result.y = abs(y);
		result.z = abs(z);
		return result;		
	}
	
        Vector3d<Type> vectorMin(Vector3d<Type> a) const
	{
		Vector3d<Type> c(min(a.x, x), min(a.y, y), min(a.z, z));
		return c;
	}

        Vector3d<Type> vectorMin(Vector3d<Type> a, Vector3d<Type> b) const
	{
		Vector3d<Type> c(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z));
		return c;
	}

        Vector3d vectorMax(const Vector3d<Type> a) const
	{
		Vector3d c(max(a.x, x), max(a.y, y), max(a.z, z));
		return c;
	}

        Vector3d vectorMax(const Vector3d<Type> a, const Vector3d<Type> b) const
	{
		Vector3d c(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z));
		return c;
	}

        Vector3d findArbitaryNormal()
        {
            Vector3d absDir = this->vectorAbs();
            Vector3d n	= Vector3d(1.0f, 0.0f, 0.0f);
            float fMin = absDir.x;
            if(absDir.y < fMin)
            {
                fMin = absDir.y;
                n	 = Vector3d(0.0f, 1.0f, 0.0f);
            }
            if(absDir.z < fMin )
            {
                fMin = absDir.z;
                n	 = Vector3d(0.0f, 0.0f, 1.0f);
            }
            n = this->cross(n);
            n.normalize();

            return n;
        }

        Vector3d negative() const
	{
		Vector3d result;
		result.x = -x;
		result.y = -y;
		result.z = -z;
		return result;
	}

	Type magnitude(void) const
	{
		return Type(sqrt(x * x + y * y + z * z));
	};

	Type fastMagnitude(void) const
	{
		return Type(fast_sqrt(x * x + y * y + z * z));
	};

	Type fasterMagnitude(void) const
	{
		return Type(faster_sqrt(x * x + y * y + z * z));
	};

	void lerp(const Vector3d<Type>& from,const Vector3d<Type>& to,float slerp)
	{
		x = ((to.x - from.x) * slerp) + from.x;
		y = ((to.y - from.y) * slerp) + from.y;
		z = ((to.z - from.z) * slerp) + from.z;
	};

	// Highly specialized interpolate routine.  Will compute the interpolated position
	// shifted forward or backwards along the ray defined between (from) and (to).
	// Reason for existance is so that when a bullet collides with a wall, for
	// example, you can generate a graphic effect slightly *before* it hit the
	// wall so that the effect doesn't sort into the wall itself.
	void interpolate(const Vector3d<float> &from,const Vector3d<float> &to,float offset)
	{
		x = to.x-from.x;
		y = to.y-from.y;
		z = to.z-from.z;
		float d = sqrtf( x*x + y*y + z*z );
		float recip = 1.0f / d;
		x*=recip;
		y*=recip;
		z*=recip; // normalize vector
		d+=offset; // shift along ray
		x = x*d + from.x;
		y = y*d + from.y;
		z = z*d + from.z;
	};

	bool binaryEqual(const Vector3d<float> &p) const
	{
		const int *source = (const int *) &x;
		const int *dest   = (const int *) &p.x;

		if ( source[0] == dest[0] &&
				 source[1] == dest[1] &&
				 source[2] == dest[2] ) return true;

		return false;
	};

	bool binaryEqual(const Vector3d<int> &p) const
	{
		if ( x == p.x && y == p.y && z == p.z ) return true;
		return false;
	}


	/** Computes the reflection vector between two vectors.*/
	void reflect(const Vector3d<Type> &a,const Vector3d<Type> &b)// compute reflection vector.
	{
		Vector3d<float> c;
		Vector3d<float> d;

		float dot = a.dot(b) * 2.0f;

		c = b * dot;

		d = c - a;

		x = -d.x;
		y = -d.y;
		z = -d.z;
	};

	void angleAxis(Type angle,const Vector3d<Type>& axis)
	{
		x = axis.x*angle;
		y = axis.y*angle;
		z = axis.z*angle;
	};

	Type length(void) const          // length of vector.
	{
		return Type(sqrt( x*x + y*y + z*z ));
	};


	float computePlane(const Vector3d<float> &A,
										 const Vector3d<float> &B,
										 const Vector3d<float> &C)
	{
		float vx,vy,vz,wx,wy,wz,vw_x,vw_y,vw_z,mag;

		vx = (B.x - C.x);
		vy = (B.y - C.y);
		vz = (B.z - C.z);

		wx = (A.x - B.x);
		wy = (A.y - B.y);
		wz = (A.z - B.z);

		vw_x = vy * wz - vz * wy;
		vw_y = vz * wx - vx * wz;
		vw_z = vx * wy - vy * wx;

		mag = sqrtf((vw_x * vw_x) + (vw_y * vw_y) + (vw_z * vw_z));

		if ( mag < 0.000001f )
		{
			mag = 0;
		}
		else
		{
			mag = 1.0f/mag;
		}

		x = vw_x * mag;
		y = vw_y * mag;
		z = vw_z * mag;


		float D = 0.0f - ((x*A.x)+(y*A.y)+(z*A.z));

		return D;
	}


	Type fastLength(void) const          // length of vector.
	{
		return Type(fast_sqrt( x*x + y*y + z*z ));
	};
	

	Type fasterLength(void) const          // length of vector.
	{
		return Type(faster_sqrt( x*x + y*y + z*z ));
	};

	Type length2(void) const         // squared distance, prior to square root.
	{
		return x*x+y*y+z*z;		
	};

	Type distance(const Vector3d<Type> &a) const   // distance between two points.
	{
		Vector3d<Type> d(a.x-x,a.y-y,a.z-z);
		return d.length();
	}

	Type fastDistance(const Vector3d<Type> &a) const   // distance between two points.
	{
		Vector3d<Type> d(a.x-x,a.y-y,a.z-z);
		return d.fastLength();
	}
	
	Type fasterDistance(const Vector3d<Type> &a) const   // distance between two points.
	{
		Vector3d<Type> d(a.x-x,a.y-y,a.z-z);
		return d.fasterLength();
	}


	Type distXY(const Vector3d<Type> &a) const
	{
            float dx = a.x - x;
            float dy = a.y - y;
            float dist = dx*dx + dy*dy;
            return dist;
	}

	Type dist2(const Vector3d<Type> &a) const  // squared distance.
	{
		float dx = a.x - x;
		float dy = a.y - y;
		float dz = a.z - z;
		return dx*dx + dy*dy + dz*dz;
	};

	Type partial(const Vector3d<Type> &p) const
	{
		return (x*p.y) - (p.x*y);
	}

	Type area(const Vector3d<Type> &p1,const Vector3d<Type> &p2) const
	{
		Type A = partial(p1);
		A+= p1.partial(p2);
		A+= p2.partial(*this);
		return A*0.5f;
	}

	// normalize to a unit vector, returns distance.
	inline float normalize(void)       
	{
		float d = sqrtf( static_cast< float >( x*x + y*y + z*z ) );
		if ( d > 0 )
		{
			float r = 1.0f / d;
			x *= r;
			y *= r;
			z *= r;
		}
		else
		{
			x = y = z = 1;
		}
		return d;
	};

        /*

	inline float FastNormalize(void)       // normalize to a unit vector, returns distance.
	{
		float d = fast_sqrt( static_cast< float >( x*x + y*y + z*z ) );
		if ( d > 0 )
		{
			float r = 1.0f / d;
			x *= r;
			y *= r;
			z *= r;
		}
		else
		{
			x = y = z = 1;
		}
		return d;
	};

	inline float FasterNormalize(void)       // normalize to a unit vector, returns distance.
	{
		float d = faster_sqrt( static_cast< float >( x*x + y*y + z*z ) );
		if ( d > 0 )
		{
			float r = 1.0f / d;
			x *= r;
			y *= r;
			z *= r;
		}
		else
		{
			x = y = z = 1;
		}
		return d;
	};

        */


	Type dot(const Vector3d<Type> &a) const        // computes dot product.
	{
		return (x * a.x + y * a.y + z * a.z );
	};


	Vector3d<Type> cross( const Vector3d<Type>& other ) const
	{
 		Vector3d<Type> result( y*other.z - z*other.y,  z*other.x - x*other.z,  x*other.y - y*other.x );

		return result;
	}

	void cross(const Vector3d<Type> &a,const Vector3d<Type> &b)  // cross two vectors result in this one.
	{
		x = a.y*b.z - a.z*b.y;
		y = a.z*b.x - a.x*b.z;
		z = a.x*b.y - a.y*b.x;
	};



//private:

	Type x;
	Type y;
	Type z;
};


//////////////////////////////////////////////////////////////////////////
template <class Type>
class Vector4d
{
public:
	Type x,y,z,w;

public:
	Vector4d() 
	{
		x = y = z = w = 0;
	};

	Vector4d(const Type& t)
	{
		x = y = z = w = t;
	}

	Vector4d(const Vector4d &a)
	{
		x = a.x;
		y = a.y;
		z = a.z;
		w = a.w;
	}

        Vector4d(const Vector3d<Type>& a3, Type w)
        {
            x = a3.x;
            y = a3.y;
            z = a3.z;
            w = w;
        }

	Vector4d(const Type* t)
	{
		x = t[0];
		y = t[1];
		z = t[2];
		w = t[3];
	}

	Vector4d(Type vx, Type vy, Type vz, Type vw = 0.0)
	{
		x = vx;
		y = vy;
		z = vz;
		w = vw;
	}

	void set(const float f)
	{
		x = y = z = w = f;
	}

	void set(const float *p)
	{
		x = (Type) p[0];
		y = (Type) p[1];
		z = (Type) p[2];
		w = (Type) p[3];
	}

	void set(float a, float b, float c, float d = 0.0f)
	{
		x = a;
		y = b;
		z = c;
		w = d;
	}

	void zero()
	{
		x = y = z = w = 0;
	}

	inline float normalizeXYZ()
	{
		float d = sqrtf( static_cast< float >( x*x + y*y + z*z ) );
		if ( d > 0 )
		{
			float r = 1.0f / d;
			x *= r;
			y *= r;
			z *= r;
		}
		else
		{
			x = y = z = 1;
		}
		return d;
	}

	// computes dot product.
	Type dot(const Vector4d<Type> &a) const        
	{
		return (x * a.x + y * a.y + z * a.z + w * a.w);
        }

	Type dist2(const Vector4d<Type> &a) const
	{
		Vector4d d;
		d.x = a.x - x;
		d.y = a.y - y;
		d.z = a.z - z;
		d.w = a.w - w;
		return (d.x*d.x + d.y*d.y + d.z*d.z + d.w*d.w);
        }

	Vector4d& operator = (const Vector4d& A)          // ASSIGNMENT (=)
	{ 
		x=A.x; y=A.y; z=A.z; w=A.w; 			
		return(*this);  
        }

	Type* ptr() {return &x;}
	Type & operator [] (const int i)
	{
		if(i==0) return x;
		else if(i==1) return y;
		else if(i==2) return z;
		else return w;
	}

	bool operator==(const Vector4d& a)
	{
		return (FLOAT_EQ(a.x, x)&&FLOAT_EQ(a.y, y)&&FLOAT_EQ(a.z, z)&&FLOAT_EQ(a.w, w));		
	}

	bool operator!=(const Vector4d& a)
	{
		return !(FLOAT_EQ(a.x, x)&&FLOAT_EQ(a.y, y)&&FLOAT_EQ(a.z, z)&&FLOAT_EQ(a.w, w));
	}

	Vector4d operator + (const Vector4d& A) const
	{
		Vector4d added(x + A.x, y + A.y, z + A.z, w + A.w);
		return added;
	};

	Vector4d operator + (const float s) const
	{
		Vector4d added(x + s, y + s, z + s, w + s);
		return added;
	};

	void operator += (const Vector4d& A)  
	{
		x += A.x;
		y += A.y;
		z += A.z;
		w += A.w;
	}

	void operator += (const float s)  
	{
		x += s;
		y += s;
		z += s;
		w += s;
	}

	Vector4d operator - (const Vector4d& A) const
	{
		Vector4d added(x - A.x, y - A.y, z - A.z, w - A.w);
		return added;
	};

	Vector4d operator - (const float s) const
	{
		Vector4d added(x - s, y - s, z - s, w - s);
		return added;
	};

	void operator -= (const Vector4d& A)  
	{
		x -= A.x;
		y -= A.y;
		z -= A.z;
		w -= A.w;
	}

	void operator -= (const float s)  
	{
		x -= s;
		y -= s;
		z -= s;
		w -= s;
	}


	Vector4d operator * (const Vector4d& A) const
	{
		Vector4d scaled(x * A.x, y * A.y, z * A.z, w * A.w);
		return scaled;
	};


	Vector4d operator * (const float s) const       // MULTIPLY BY SCALAR (*)
	{ 
		Vector4d Scaled(x*s, y*s, z*s, w*s);
		return(Scaled); 
	};

	void operator *= (const float s)        
	{
		x*=s; y*=s; z*=s; w*=s;
	}
	void operator *= (const Vector4d A)
	{
		x *= A.x;
		y *= A.y;
		z *= A.z;
		w *= A.w;
	}

	// DIVIDE BY SCALAR (/)
	Vector4d operator / (const float s) const    
	{
		float r = 1.0f / s;
		Vector4d Scaled(x*r, y*r, z*r, w*r);
		return(Scaled);
	};

	void operator /= (Type A)             
	{ 
		Type invA = 1.0f / A;
		x*=A; y*=A; z*=A; w*=A;
	};


	Vector3d<Type> xyz() const
	{
		Vector3d<Type> ret(x, y, z);
		return ret;
	}

	// ALLOWS VECTOR ACCESS AS AN ARRAY.
	/*
	Type operator [] (const int i) const         
	{ 		
		return( (i==0)?x:((i==1)?y:z) ); 
	}
	
	Type & operator [] (const int i)
	{ 
		return( (i==0)?x:((i==1)?y:z) ); 
	}
*/


};


typedef std::vector< Vector3d<float> > vec3fArray;
typedef std::vector< Vector2d<float> > vec2fArray;

//float vector
typedef Vector4d<float> vec4;
typedef Vector3d<float> vec3f;
typedef Vector2d<float> vec2;

//float vector
typedef Vector4d<float> vec4f;
typedef Vector3d<float> vec3f;
typedef Vector2d<float> vec2f;

//Double Precision 
typedef Vector4d<double> vec4d;
typedef Vector3d<double> vec3d;
typedef Vector2d<double> vec2d;

//boolean vertices
typedef Vector4d<bool> vec4b;
typedef Vector3d<bool> vec3b;
typedef Vector2d<bool> vec2b;

//integer vertices
typedef Vector4d<int> vec4i;
typedef Vector3d<int> vec3i;
typedef Vector2d<int> vec2i;

typedef Vector4d<unsigned int> vec4ui;
typedef Vector3d<unsigned int> vec3ui;
typedef Vector2d<unsigned int> vec2ui;


template <class Type> Vector3d<Type> operator * (Type s, const Vector3d<Type> &v )
			{ Vector3d <Type> Scaled(v.x*s, v.y*s, v.z*s);
				return(Scaled); };

template <class Type> Vector2d<Type> operator * (Type s, const Vector2d<Type> &v )
			{ Vector2d <Type> Scaled(v.x*s, v.y*s);
				return(Scaled); };

}
}

#endif
