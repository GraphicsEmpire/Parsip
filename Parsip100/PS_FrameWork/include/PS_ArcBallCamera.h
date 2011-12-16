#pragma once
#ifndef PS_CAMERA_H
#define PS_CAMERA_H

#include "mathHelper.h"
#include "PS_Vector.h"

const float zoomMin = 0.01f;
const float zoomMax = 80.0f;
const float zoom = 30.0f;
const float verticalAngleMin = 0.01f;
const float verticalAngleMax = Pi - 0.01f;
const float horizontalAngle = -1.0f * PiOver2;
const float verticalAngle = PiOver2;

//A basic ArcBall Camera class to be used with openGL or directX
namespace PS{

using namespace MATH;

class CArcBallCamera
{
private:

	//Omega and Phi are horizontal and vertical angles of spherical coordinates respectively
	//rho is the CCamera distance from scene (Zoom)
	float m_omega, m_phi, m_rho;
	vec3f m_origin;

	//Center point in scene to lookAt
	vec3f  m_center;
	vec2i m_lastPos;	
public:
	//Default Constructor
	CArcBallCamera();

	//Constructor with valid values
	CArcBallCamera(float o, float p, float r);

	typedef enum MOUSEBUTTONSTATE {mbLeft, mbRight, mbMiddle, mbNone};

	//Access to member variables of this class
	const float getHorizontalAngle() const {return m_omega;}
	const float getVerticalAngle() const {return m_phi;}
	const float getCurrentZoom() const {return m_rho;}
	vec3f getOrigin() {return m_origin;}
	vec3f getCenter() {return m_center;}

	//Set our horizontal angle can be any value (Omega)
	void setHorizontalAngle(float o);

	//Set our vertical angle. This is clamped between 0 and 180
	void setVerticalAngle(float p);

	//Zoom or CCamera distance from scene is clamped.
	void setZoom(float r);

	//Set Origin
	void setOrigin(vec3f org);

	//Set Center point inside scene
	void setCenter(vec3f c);

	//convert spherical coordinates to Eulerian values
	vec3f getCoordinates();

	//Return Current CCamera Direction
	vec3f getDirection();

	//Calculate an Up vector
	vec3f getUp();

	vec3f getStrafe();

	//Set the last position
	void setLastPos(const vec2i& lastPos)
	{
		m_lastPos = lastPos;
	}

	vec2i getLastPos() const {return m_lastPos;}

	void goHome();
};

}
#endif
