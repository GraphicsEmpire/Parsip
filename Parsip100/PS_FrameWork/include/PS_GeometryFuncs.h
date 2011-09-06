#pragma once

#ifndef PS_GEOMTERYFUNCS_H
#define PS_GEOMTERYFUNCS_H

#include "PS_Vector.h"

namespace PS{
	namespace MATH{
		//Check if next edge (b to c) turns inward		
		//Edge from a to b is already in face
		//Edge from b to c is being considered for addition to face		
		bool Concave(const vec3f& a,const vec3f& b, const vec3f& c);
				
		// test to see if this point is inside the triangle specified by
		// these three points on the X/Y plane.
		bool PointInTriXY(const vec3f& v, const vec3f &p1, const vec3f &p2, const vec3f &p3);

		// test to see if this point is inside the triangle specified by
		// these three points on the Y/Z plane.
		bool PointInTriYZ(const vec3f& v, const vec3f &p1, const vec3f &p2,	const vec3f &p3);

		// test to see if this point is inside the triangle specified by
		// these three points on the X/Z plane.
		bool PointInTriXZ(const vec3f& v, const vec3f &p1, const vec3f &p2,	const vec3f &p3);

		// Given a point and a line (defined by two points), compute the closest point
		// in the line.  (The line is treated as infinitely long.)
		vec3f NearestPointInLine(const vec3f &point,	const vec3f &line0,	const vec3f &line1);

		// Given a point and a line segment (defined by two points), compute the closest point
		// in the line. Cap the point at the endpoints of the line segment.
		vec3f NearestPointInLineSegment(const vec3f &point, const vec3f &line0, const vec3f &line1);

		// Given a point and a plane (defined by three points), compute the closest point
		// in the plane.  (The plane is unbounded.)
		vec3f NearestPointInPlane(const vec3f &point,
			  		 			  const vec3f &triangle0,
								  const vec3f &triangle1,
								  const vec3f &triangle2);

		// Given a point and a plane (defined by a coplanar point and a normal), compute the closest point
		// in the plane.  (The plane is unbounded.)
		vec3f NearestPointInPlane(const vec3f &point,
								 const vec3f &planePoint,
								 const vec3f &planeNormal);

		vec3f NearestPointInTriangle(const vec3f &point,
									const vec3f &triangle0,
									const vec3f &triangle1,
									const vec3f &triangle2);
	}
}
#endif