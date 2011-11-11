#include "PS_GeometryFuncs.h"

namespace PS{
        namespace MATH{
                //Check if next edge (b to c) turns inward
                // Edge from a to b is already in face
                // Edge from b to c is being considered for addition to face
                bool Concave(const vec3f& a,const vec3f& b, const vec3f& c)
                {
                        float vx,vy,vz,wx,wy,wz,vw_x,vw_y,vw_z,mag,nx,ny,nz,mag_a,mag_b;

                        wx = b.x - a.x;
                        wy = b.y - a.y;
                        wz = b.z - a.z;

                        mag_a = (float) sqrtf((wx * wx) + (wy * wy) + (wz * wz));

                        vx = c.x - b.x;
                        vy = c.y - b.y;
                        vz = c.z - b.z;

                        mag_b = (float) sqrtf((vx * vx) + (vy * vy) + (vz * vz));

                        vw_x = (vy * wz) - (vz * wy);
                        vw_y = (vz * wx) - (vx * wz);
                        vw_z = (vx * wy) - (vy * wx);

                        mag = (float) sqrtf((vw_x * vw_x) + (vw_y * vw_y) + (vw_z * vw_z));

                        // Check magnitude of cross product, which is a sine function
                        // i.e., mag (a x b) = mag (a) * mag (b) * sin (theta);
                        // If sin (theta) small, then angle between edges is very close to
                        // 180, which we may want to call a concavity.	Setting the
                        // CONCAVITY_TOLERANCE value greater than about 0.01 MAY cause
                        // face consolidation to get stuck on particular face.	Most meshes
                        // convert properly with a value of 0.0

                        if (mag/(mag_a*mag_b) <= 0.0f )	return true;

                        mag = 1.0f / mag;

                        nx = vw_x * mag;
                        ny = vw_y * mag;
                        nz = vw_z * mag;

                        // Dot product of tri normal with cross product result will
                        // yield positive number if edges are convex (+1.0 if two tris
                        // are coplanar), negative number if edges are concave (-1.0 if
                        // two tris are coplanar.)

                        mag = ( c.x * nx) + ( c.y * ny) + ( c.z * nz);

                        if (mag > 0.0f ) return false;

                        return(true);
                };

                // test to see if this point is inside the triangle specified by
                // these three points on the X/Y plane.
                bool PointInTriXY(const vec3f& v, const vec3f &p1,
                                                  const vec3f &p2, const vec3f &p3)
                {
                        float ax  = p3.x - p2.x;
                        float ay  = p3.y - p2.y;
                        float bx  = p1.x - p3.x;
                        float by  = p1.y - p3.y;
                        float cx  = p2.x - p1.x;
                        float cy  = p2.y - p1.y;
                        float apx = v.x - p1.x;
                        float apy = v.y - p1.y;
                        float bpx = v.x - p2.x;
                        float bpy = v.y - p2.y;
                        float cpx = v.x - p3.x;
                        float cpy = v.y - p3.y;

                        float aCROSSbp = ax*bpy - ay*bpx;
                        float cCROSSap = cx*apy - cy*apx;
                        float bCROSScp = bx*cpy - by*cpx;

                        return ((aCROSSbp >= 0.0f) && (bCROSScp >= 0.0f) && (cCROSSap >= 0.0f));
                };

                // test to see if this point is inside the triangle specified by
                // these three points on the X/Y plane.
                bool PointInTriYZ(const vec3f& v, const vec3f &p1,
                                                  const vec3f &p2,	const vec3f &p3)
                {
                        float ay  = p3.y - p2.y;
                        float az  = p3.z - p2.z;
                        float by  = p1.y - p3.y;
                        float bz  = p1.z - p3.z;
                        float cy  = p2.y - p1.y;
                        float cz  = p2.z - p1.z;
                        float apy = v.y - p1.y;
                        float apz = v.z - p1.z;
                        float bpy = v.y - p2.y;
                        float bpz = v.z - p2.z;
                        float cpy = v.y - p3.y;
                        float cpz = v.z - p3.z;

                        float aCROSSbp = ay*bpz - az*bpy;
                        float cCROSSap = cy*apz - cz*apy;
                        float bCROSScp = by*cpz - bz*cpy;

                        return ((aCROSSbp >= 0.0f) && (bCROSScp >= 0.0f) && (cCROSSap >= 0.0f));
                };


                // test to see if this point is inside the triangle specified by
                // these three points on the X/Y plane.
                bool PointInTriXZ(const vec3f& v, const vec3f &p1,
                                                  const vec3f &p2, const vec3f &p3)
                {
                        float az  = p3.z - p2.z;
                        float ax  = p3.x - p2.x;
                        float bz  = p1.z - p3.z;
                        float bx  = p1.x - p3.x;
                        float cz  = p2.z - p1.z;
                        float cx  = p2.x - p1.x;
                        float apz = v.z - p1.z;
                        float apx = v.x - p1.x;
                        float bpz = v.z - p2.z;
                        float bpx = v.x - p2.x;
                        float cpz = v.z - p3.z;
                        float cpx = v.x - p3.x;

                        float aCROSSbp = az*bpx - ax*bpz;
                        float cCROSSap = cz*apx - cx*apz;
                        float bCROSScp = bz*cpx - bx*cpz;

                        return ((aCROSSbp >= 0.0f) && (bCROSScp >= 0.0f) && (cCROSSap >= 0.0f));
                };

                // Given a point and a line (defined by two points), compute the closest point
                // in the line.  (The line is treated as infinitely long.)
                vec3f NearestPointInLine(const vec3f &point,	const vec3f &line0,	const vec3f &line1)
                {
                        vec3f nearestPoint;
                        vec3f lineDelta     = line1 - line0;

                        // Handle degenerate lines
                        if ( lineDelta == vec3f(0, 0, 0) )
                        {
                                nearestPoint = line0;
                        }
                        else
                        {
                                float delta = (point-line0).dot(lineDelta) / (lineDelta).dot(lineDelta);
                                nearestPoint = line0 + delta*lineDelta;
                        }
                        return nearestPoint;
                }

                // Given a point and a line segment (defined by two points), compute the closest point
                // in the line.  Cap the point at the endpoints of the line segment.
                vec3f NearestPointInLineSegment(const vec3f &point, const vec3f &line0, const vec3f &line1)
                {
                        vec3f nearestPoint;
                        vec3f lineDelta     = line1 - line0;

                        // Handle degenerate lines
                        if ( lineDelta == vec3f(0, 0, 0) )
                        {
                                nearestPoint = line0;
                        }
                        else
                        {
                                float delta = (point-line0).dot(lineDelta) / (lineDelta).dot(lineDelta);

                                // Clamp the point to conform to the segment's endpoints
                                if ( delta < 0 )
                                        delta = 0;
                                else if ( delta > 1 )
                                        delta = 1;

                                nearestPoint = line0 + delta*lineDelta;
                        }
                        return nearestPoint;
                }

                // Given a point and a plane (defined by three points), compute the closest point
                // in the plane.  (The plane is unbounded.)
                vec3f NearestPointInPlane(const vec3f &point, const vec3f &triangle0,
                                                                  const vec3f &triangle1, const vec3f &triangle2)
                {
                        vec3f nearestPoint;
                        vec3f lineDelta0    = triangle1 - triangle0;
                        vec3f lineDelta1    = triangle2 - triangle0;
                        vec3f pointDelta    = point - triangle0;
                        vec3f normal;

                        // Get the normal of the polygon (doesn't have to be a unit vector)
                        normal.cross(lineDelta0, lineDelta1);

                        float delta = normal.dot(pointDelta) / normal.dot(normal);
                        nearestPoint = point - delta*normal;
                        return nearestPoint;
                }

                // Given a point and a plane (defined by a coplanar point and a normal), compute the closest point
                // in the plane.  (The plane is unbounded.)
                vec3f NearestPointInPlane(const vec3f &point, const vec3f &planePoint, const vec3f &planeNormal)
                {
                        vec3f nearestPoint;
                        vec3f pointDelta    = point - planePoint;

                        float delta = planeNormal.dot(pointDelta) / planeNormal.dot(planeNormal);
                        nearestPoint = point - delta*planeNormal;
                        return nearestPoint;
                }

                // Given a point and a triangle (defined by three points), compute the closest point
                // in the triangle.  Clamp the point so it's confined to the area of the triangle.
                vec3f NearestPointInTriangle(const vec3f &point, const vec3f &triangle0,
                                                                         const vec3f &triangle1,	const vec3f &triangle2)
                {
                        vec3f zeroVector(0, 0, 0);
                        vec3f nearestPoint;

                        vec3f lineDelta0 = triangle1 - triangle0;
                        vec3f lineDelta1 = triangle2 - triangle0;

                        // Handle degenerate triangles
                        if ( (lineDelta0 == zeroVector) || (lineDelta1 == zeroVector) )
                        {
                                nearestPoint = NearestPointInLineSegment(point, triangle1, triangle2);
                        }
                        else if ( lineDelta0 == lineDelta1 )
                        {
                                nearestPoint = NearestPointInLineSegment(point, triangle0, triangle1);
                        }
                        else
                        {
                                static vec3f axis[3];
                                axis[0] = NearestPointInLine(triangle0, triangle1, triangle2);
                                axis[1] = NearestPointInLine(triangle1, triangle0, triangle2);
                                axis[2] = NearestPointInLine(triangle2, triangle0, triangle1);

                                float axisDot[3];
                                axisDot[0] = (triangle0-axis[0]).dot(point-axis[0]);
                                axisDot[1] = (triangle1-axis[1]).dot(point-axis[1]);
                                axisDot[2] = (triangle2-axis[2]).dot(point-axis[2]);

                                bool bForce = true;
                                float bestMagnitude2 = 0;
                                float closeMagnitude2;
                                vec3f closePoint;

                                if ( axisDot[0] < 0 )
                                {
                                        closePoint = NearestPointInLineSegment(point, triangle1, triangle2);
                                        closeMagnitude2 = point.dist2(closePoint);
                                        if ( bForce || (bestMagnitude2 > closeMagnitude2) )
                                        {
                                                bForce         = false;
                                                bestMagnitude2 = closeMagnitude2;
                                                nearestPoint   = closePoint;
                                        }
                                }
                                if ( axisDot[1] < 0 )
                                {
                                        closePoint = NearestPointInLineSegment(point, triangle0, triangle2);
                                        closeMagnitude2 = point.dist2(closePoint);
                                        if ( bForce || (bestMagnitude2 > closeMagnitude2) )
                                        {
                                                bForce         = false;
                                                bestMagnitude2 = closeMagnitude2;
                                                nearestPoint   = closePoint;
                                        }
                                }
                                if ( axisDot[2] < 0 )
                                {
                                        closePoint = NearestPointInLineSegment(point, triangle0, triangle1);
                                        closeMagnitude2 = point.dist2(closePoint);
                                        if ( bForce || (bestMagnitude2 > closeMagnitude2) )
                                        {
                                                bForce         = false;
                                                bestMagnitude2 = closeMagnitude2;
                                                nearestPoint   = closePoint;
                                        }
                                }

                                // If bForce is true at this point, it means the nearest point lies
                                // inside the triangle; use the nearest-point-on-a-plane equation
                                if ( bForce )
                                {
                                        vec3f normal;

                                        // Get the normal of the polygon (doesn't have to be a unit vector)
                                        normal.cross(lineDelta0, lineDelta1);

                                        vec3f pointDelta = point - triangle0;
                                        float delta = normal.dot(pointDelta) / normal.dot(normal);

                                        nearestPoint = point - delta*normal;
                                }
                        }
                        return nearestPoint;
                }
        }
}
