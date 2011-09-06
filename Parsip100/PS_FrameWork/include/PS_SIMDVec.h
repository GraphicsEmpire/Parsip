#pragma once
#ifndef PS_SIMD_VEC
#define PS_SIMD_VEC


//#include "fvec.h"
#include <xmmintrin.h>
#include <pmmintrin.h>
#include <crtdefs.h>

#define PS_SIMD_LINES 4
#define PS_SIMD_ALIGN_SIZE 16
#define USE_SSE3 
#define USE_HW_SIMD

#define FOR_I_N for (int i=0; i<PS_SIMD_LINES; i++)

#ifdef  _MSC_VER
#pragma pack(push,_CRT_PACKING)
#endif  

#pragma pack(push,16) /* Must ensure class & union 16-B aligned */

#define CompareResult4(v)  (_mm_movemask_ps((v)) == 15)
#define CompareResult3(v) ((_mm_movemask_ps((v)) == 7)||(_mm_movemask_ps((v)) == 15))

namespace PS{
	namespace MATH{

		class SVecN
		{
#ifdef USE_HW_SIMD
		public:
			__m128 v;

			SVecN()			{ v = _mm_setzero_ps();	}

			SVecN(const float& x, const float& y, const float& z, const float& w = 0.0f)
			{
				v = _mm_set_ps(w, z, y, x);
			}

			SVecN(const __m128& rhs)	{ v = rhs; }
			SVecN(const float& rhs)		{ v = _mm_set_ps1(rhs); }			
			////////////////////////////////////////////////
			void setzero()				{ v = _mm_setzero_ps();    }
			void set( const float& x, const float& y, const float& z, const float& w = 0.0f)
			{
				v = _mm_set_ps(w, z, y, x);
			}

			void set(const __m128& rhs)	{ v = rhs; }

			void set(const float& rhs)	{ v = _mm_set_ps1(rhs); }

			SVecN dot(const SVecN &a)
			{
				SVecN res = _mm_mul_ps(v, a);
			#ifdef USE_SSE3
				SVecN d = _mm_hadd_ps(res, res);
				res = _mm_hadd_ps(d, d);
			#else
				float f = res[0] + res[1] + res[2] + res[3];
				res.set(f);
			#endif
				return res;
			}

			SVecN vmul(const SVecN &a) 	{ return _mm_mul_ps(v, a);}			
			////////////////////////////////////////////////
			/* Compares: Mask is returned  */
			/* Macros expand to all compare intrinsics.  Example:
			friend F32vec4 cmpeq(const F32vec4 &a, const F32vec4 &b)
			{ return _mm_cmpeq_ps(a,b);} */
			#define SVEC4_COMP(op) \
			friend SVecN cmp##op (const SVecN &a, const SVecN &b) { return _mm_cmp##op##_ps(a,b); }
				SVEC4_COMP(eq)					/* expanded to cmpeq(a,b) */
				SVEC4_COMP(lt)					/* expanded to cmplt(a,b) */
				SVEC4_COMP(le)					/* expanded to cmple(a,b) */
				SVEC4_COMP(gt)					/* expanded to cmpgt(a,b) */
				SVEC4_COMP(ge)					/* expanded to cmpge(a,b) */
				SVEC4_COMP(neq)					/* expanded to cmpneq(a,b) */
				SVEC4_COMP(nlt)					/* expanded to cmpnlt(a,b) */
				SVEC4_COMP(nle)					/* expanded to cmpnle(a,b) */
				SVEC4_COMP(ngt)					/* expanded to cmpngt(a,b) */
				SVEC4_COMP(nge)					/* expanded to cmpnge(a,b) */
			#undef SVEC4_COMP

			friend bool isZero(const SVecN &a) 
			{
				SVecN zero = _mm_setzero_ps();				
				int mask = _mm_movemask_ps(_mm_cmpeq_ps(a, zero));
				return (mask == 15);
			}
			
			//Max and Min
			friend SVecN simd_min(const SVecN &a, const SVecN &b) { return _mm_min_ps(a,b); }
			friend SVecN simd_max(const SVecN &a, const SVecN &b) { return _mm_max_ps(a,b); }
			

			/* Square Root */
			friend SVecN simd_sqrt(const SVecN &a)		{ return _mm_sqrt_ps(a); }
			/* Reciprocal */
			friend SVecN simd_rcp(const SVecN &a)		{ return _mm_rcp_ps(a); }
			/* Reciprocal Square Root */
			friend SVecN simd_rsqrt(const SVecN &a)		{ return _mm_rsqrt_ps(a); }
			//Power
			//friend SVecN rsqrt(const SVecN &a)		{ return _mm_rsqrt_ps(a); }

			friend SVecN simd_dot(const SVecN &a, const SVecN &b)
			{				
				SVecN res = _mm_mul_ps(a, b);
			#ifdef USE_SSE3
				SVecN d = _mm_hadd_ps(res, res);
				res = _mm_hadd_ps(d, d);
			#else
				float f = res[0] + res[1] + res[2] + res[3];
				res.set(f);
			#endif
				return res;
			}

			friend SVecN simd_dist2(const SVecN &a, const SVecN &b)
			{
				SVecN delta = _mm_sub_ps(a, b);
				SVecN res = _mm_mul_ps(delta, delta);
			#ifdef USE_SSE3
				SVecN d = _mm_hadd_ps(res, res);
				res = _mm_hadd_ps(d, d);
			#else
				float f = res[0] + res[1] + res[2] + res[3];
				res.set(f);
			#endif
				return res;
			}

			friend SVecN simd_len2(const SVecN &a)
			{
				SVecN res = _mm_mul_ps(a, a);
			#ifdef USE_SSE3
				SVecN d = _mm_hadd_ps(res, res);
				res = _mm_hadd_ps(d, d);
			#else
				float f = res[0] + res[1] + res[2] + res[3];
				res.set(f);
			#endif
				return res;
			}

			
			friend SVecN simd_abs(const SVecN &a)
			{	
				static const U32 notSignBitMask = ~0x80000000;
				return _mm_and_ps( a.v, _mm_set_ps1( *(float *)&notSignBitMask ) );
			}
			
			friend SVecN simd_sign(const SVecN &a)
			{
				const __m128	zero		= _mm_setzero_ps();
				const __m128	selectPos	= _mm_cmpgt_ps( a.v, zero );	// > 0
				const __m128	selectNeg	= _mm_cmplt_ps( a.v, zero );	// < 0

				__m128	res =		  _mm_and_ps( selectPos, _mm_set_ps1(  1.0f ) );
				res = _mm_or_ps( res, _mm_and_ps( selectNeg, _mm_set_ps1( -1.0f ) ) );
				return res;
			}


			friend SVecN simd_len(const SVecN &a)
			{
				SVecN res = _mm_mul_ps(a, a);
			#ifdef USE_SSE3
				SVecN d = _mm_hadd_ps(res, res);
				res = _mm_hadd_ps(d, d);
			#else
				float f = res.f[0] + res.f[1] + res.f[2] + res.f[3];
				res.set(f);
			#endif
				int mask = _mm_movemask_ps(_mm_cmple_ps(res, _mm_setzero_ps()));
				if(mask > 1)
					return a;
				else
					return _mm_sqrt_ps(res);
			}

			friend SVecN simd_normalize(const SVecN &a)
			{
				SVecN res = _mm_mul_ps(a, a);
			#ifdef USE_SSE3
				SVecN d = _mm_hadd_ps(res, res);
				res = _mm_hadd_ps(d, d);
			#else
				float f = res.f[0] + res.f[1] + res.f[2] + res.f[3];
				res.set(f);
			#endif
				int mask = _mm_movemask_ps(_mm_cmple_ps(res, _mm_setzero_ps()));
				if(mask > 1)
					return a;
				else
				{
					SVecN invD = _mm_rsqrt_ps(res);
					return _mm_mul_ps(a, invD);		
				}
			}
			////////////////////////////////////////////////
			// Element Access Only, no modifications to elements
			const float& operator[](int i) const
			{
				/* Assert enabled only during debug /DDEBUG */
				assert((0 <= i) && (i <= 3));			/* User should only access elements 0-3 */
				float *fp = (float*)&v;
				return *(fp+i);
			}
			// Element Access and Modification
			float& operator[](int i)
			{
				/* Assert enabled only during debug /DDEBUG */
				assert((0 <= i) && (i <= 3));			/* User should only access elements 0-3 */
				float *fp = (float*)&v;
				return *(fp+i);
			}

			SVecN& operator=(float f){ v = _mm_set_ps1(f); return (*this);}
			SVecN& operator=(double d){ v = _mm_set_ps1((float)d); return (*this);}

			//
			operator __m128() const {return v;}

			//Logical Operators
			friend SVecN operator &(const SVecN &a, const SVecN &b) { return _mm_and_ps(a,b); }
			friend SVecN operator |(const SVecN &a, const SVecN &b) { return _mm_or_ps(a,b);  }
			friend SVecN operator ^(const SVecN &a, const SVecN &b) { return _mm_xor_ps(a,b); }

			//Arithmetic Operators
			friend SVecN operator +(const SVecN &a, const SVecN &b) { return _mm_add_ps(a,b); }
			friend SVecN operator -(const SVecN &a, const SVecN &b) { return _mm_sub_ps(a,b); }
			friend SVecN operator *(const SVecN &a, const SVecN &b) { return _mm_mul_ps(a,b); }
			friend SVecN operator /(const SVecN &a, const SVecN &b) { return _mm_div_ps(a,b); }

			SVecN& operator =(const SVecN &a) { v = a.v; return *this; }
			SVecN& operator =(const __m128 &avec) { v = avec; return *this; }
			SVecN& operator +=(SVecN &a) { return *this = _mm_add_ps(v,a); }
			SVecN& operator -=(SVecN &a) { return *this = _mm_sub_ps(v,a); }
			SVecN& operator *=(SVecN &a) { return *this = _mm_mul_ps(v,a); }
			SVecN& operator /=(SVecN &a) { return *this = _mm_div_ps(v,a); }
			SVecN& operator &=(SVecN &a) { return *this = _mm_and_ps(v,a); }
			SVecN& operator |=(SVecN &a) { return *this = _mm_or_ps(v,a); }
			SVecN& operator ^=(SVecN &a) { return *this = _mm_xor_ps(v,a); }
#else
//==================================================================
/// No hardware SIMD
//==================================================================
public:		
			float v[PS_SIMD_LINES];

			SVecN()			{ setzero();	}

			SVecN(const float& x, const float& y, const float& z, const float& w = 0.0f)
			{
				v[0] = x; v[1] = y; v[2] = z; v[3] = w;
			}
			
			SVecN(const float* input)
			{
				FOR_I_N v[i] = input[i];
			}
			SVecN(const float& rhs)		
			{ 
				v[0] = v[1] = v[2] = v[3] = rhs;	
			}
			////////////////////////////////////////////////
			void setzero()				{ FOR_I_N v[i] = 0.0f;    }
			void set( const float& x, const float& y, const float& z, const float& w = 0.0f)
			{
				v[0] = x; v[1] = y; v[2] = z; v[3] = w;
			}

			void set(const float& rhs)	{ v[0] = v[1] = v[2] = v[3] = rhs;	 }

			friend bool isZero(const SVecN &a) 
			{
				FOR_I_N if(a[i] != 0.0f) return false; 
				return true;				
			}
			
			//Max and Min
			friend SVecN simd_min(const SVecN &a, const SVecN &b) 
			{ 
				SVecN res;
				FOR_I_N res.v[i] = MATHMIN(a.v[i], b.v[i]);				
				return res; 
			}
			
			friend SVecN simd_max(const SVecN &a, const SVecN &b) 
			{
				SVecN res;
				FOR_I_N res.v[i] = MATHMAX(a.v[i], b.v[i]);				
				return res; 
			}			

			/* Square Root */
			friend SVecN simd_sqrt(const SVecN &a)		
			{ 
				SVecN res;
				FOR_I_N res.v[i] = sqrtf(a.v[i]);				
				return res; 
			}

			/* Reciprocal */
			friend SVecN simd_rcp(const SVecN &a)		
			{ 
				SVecN res;
				FOR_I_N res.v[i] = 1.0f / a.v[i];								
				return res; 
			}

			/* Reciprocal Square Root */
			friend SVecN simd_rsqrt(const SVecN &a)		
			{ 
				SVecN res;
				FOR_I_N res.v[i] = 1.0f / sqrtf(a.v[i]);								
				return res; 
			}
			//Power
			//friend SVecN rsqrt(const SVecN &a)		{ return _mm_rsqrt_ps(a); }

			friend SVecN simd_dot(const SVecN &a, const SVecN &b)
			{				
				SVecN res = a * b;
				float f = res[0] + res[1] + res[2] + res[3];
				res.set(f);
				return res;
			}

			friend SVecN simd_dist2(const SVecN &a, const SVecN &b)
			{
				SVecN delta = a - b;
				SVecN res = delta * delta;
				float f = res[0] + res[1] + res[2] + res[3];
				res.set(f);
				return res;
			}

			friend SVecN simd_len2(const SVecN &a)
			{
				SVecN res = a * a;
				float f = res[0] + res[1] + res[2] + res[3];
				res.set(f);
				return res;
			}
			
			friend SVecN simd_abs(const SVecN &a)
			{	
				//static const U32 notSignBitMask = ~0x80000000;
				//return a.v & SVecN( *(float *)&notSignBitMask );
				SVecN res;
				FOR_I_N 
				{
					if(a.v[i] < 0.0f) 
						res.v[i] = a.v[i] * -1.0f;
					else
						res.v[i] = a.v[i];
				}
				return res;
			}
			
			friend SVecN simd_sign(const SVecN &a)
			{
				SVecN res;
				FOR_I_N 
				{
					if(a.v[i] >= 0.0f)
						res.v[i] = 1.0f;
					else
						res.v[i] = -1.0f;
				}
				return res;
			}


			friend SVecN simd_len(const SVecN &a)
			{
				SVecN res = a * a;
				float f = res.v[0] + res.v[1] + res.v[2] + res.v[3];
				res.set(sqrtf(f));
				return res;
			}

			friend SVecN simd_normalize(const SVecN &a)
			{
				SVecN res = a * a;
				float f = res.v[0] + res.v[1] + res.v[2] + res.v[3];
				res.set(1.0f / sqrtf(f));
				return a*res;
			}
			////////////////////////////////////////////////
			// Element Access Only, no modifications to elements
			const float& operator[](int i) const
			{
				/* Assert enabled only during debug /DDEBUG */
				assert((0 <= i) && (i <= 3));			/* User should only access elements 0-3 */
				float *fp = (float*)&v;
				return *(fp+i);
			}
			// Element Access and Modification
			float& operator[](int i)
			{
				/* Assert enabled only during debug /DDEBUG */
				assert((0 <= i) && (i <= 3));			/* User should only access elements 0-3 */
				float *fp = (float*)&v;
				return *(fp+i);
			}

			SVecN& operator=(float f){ this->set(f); return (*this);}
			SVecN& operator=(double d){ this->set((float)d); return (*this);}

			//Logical Operators
			//friend SVecN operator &(const SVecN &a, const SVecN &b) { SVecN res; FOR_I_N res.v[i] = (*(int *))&a.v[i] & (*(int *))&b.v[i]; return res; }
			//friend SVecN operator |(const SVecN &a, const SVecN &b) { SVecN res; FOR_I_N res.v[i] = a.v[i] | b.v[i]; return res; }
			//friend SVecN operator ^(const SVecN &a, const SVecN &b) { SVecN res; FOR_I_N res.v[i] = a.v[i] ^ b.v[i]; return res; }

			//Arithmetic Operators
			friend SVecN operator +(const SVecN &a, const SVecN &b) { SVecN res; FOR_I_N res.v[i] = a.v[i] + b.v[i]; return res; }
			friend SVecN operator -(const SVecN &a, const SVecN &b) { SVecN res; FOR_I_N res.v[i] = a.v[i] - b.v[i]; return res; }
			friend SVecN operator *(const SVecN &a, const SVecN &b) { SVecN res; FOR_I_N res.v[i] = a.v[i] * b.v[i]; return res; }
			friend SVecN operator /(const SVecN &a, const SVecN &b) { SVecN res; FOR_I_N res.v[i] = a.v[i] / b.v[i]; return res; }

			SVecN& operator =(const SVecN &a) { FOR_I_N v[i] = a.v[i]; return *this; }
			
			SVecN& operator +=(SVecN &a) { FOR_I_N v[i] += a.v[i]; return *this; }
			SVecN& operator -=(SVecN &a) { FOR_I_N v[i] -= a.v[i]; return *this; }
			SVecN& operator *=(SVecN &a) { FOR_I_N v[i] *= a.v[i]; return *this; }
			SVecN& operator /=(SVecN &a) { FOR_I_N v[i] /= a.v[i]; return *this; }
			//SVecN& operator &=(SVecN &a) { FOR_I_N v[i] &= a.v[i]; return *this; }
			//SVecN& operator |=(SVecN &a) { FOR_I_N v[i] |= a.v[i]; return *this; }
			//SVecN& operator ^=(SVecN &a) { FOR_I_N v[i] ^= a.v[i]; return *this; }
#endif



		};

		//////////////////////////////////////////////////////////////////////////
		//SIMD Type definitions
		typedef __declspec(align(PS_SIMD_ALIGN_SIZE)) SVecN		svec4f;
	}
}

#pragma pack(pop) /* 16-B aligned */

#ifdef  _MSC_VER
#pragma pack(pop)
#endif  /* _MSC_VER */

#endif