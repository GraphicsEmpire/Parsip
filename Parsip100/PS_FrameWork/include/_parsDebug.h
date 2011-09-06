#ifndef PARS_DEBUG_H
#define PARS_DEBUG_H

#ifdef _WIN32
#include "windows.h"
#pragma warning(disable:4505)
#endif

#include <string>
using namespace std;

#include <stdio.h>
#include <stdarg.h>

#ifndef ASSERT
#ifdef _DEBUG
#define ASSERT(x)  { if(! (x) ) abort(); }
#else
#define ASSERT(x) 
#endif
#endif

static void parsDebugInfo(char * str, ...)
{	
	static char buf[1024];
	va_list args;

	va_start(args, str);
	vsprintf_s(buf, str, args);
#ifdef _WIN32
	OutputDebugStringA(buf);
#else
	fprintf(stderr, "%s", buf);
#endif
	va_end(args);
}

static string parsDebugInfoString(char * str, ...)
{	
	static char buf[1024];
	va_list args;

	va_start(args, str);
	vsprintf_s(buf, str, args);
	va_end(args);
	return string(buf);
}


/*
 *
 * OpenGL debug utility functions
 *
 */

#define PARS_DEBUG_WANT_GL


#ifdef PARS_DEBUG_WANT_GL


#include <GL/gl.h>


static char PARS_DEBUG_GLErrorStrings[][32] = {  "GL_NO_ERROR",
												"GL_INVALID_ENUM", 
												"GL_INVALID_VALUE", 
												"GL_INVALID_OPERATION", 
												"GL_STACK_OVERFLOW",
												"GL_STACK_UNDERFLOW",
												"GL_OUT_OF_MEMORY",
												"Unknown GL Error"};

static char * glErrorString(GLenum error) 
{
	switch(error) {
		case GL_NO_ERROR:	   return PARS_DEBUG_GLErrorStrings[0];
		case GL_INVALID_ENUM:  return PARS_DEBUG_GLErrorStrings[1];
		case GL_INVALID_VALUE: return PARS_DEBUG_GLErrorStrings[2];
		case GL_INVALID_OPERATION: return PARS_DEBUG_GLErrorStrings[3];
		case GL_STACK_OVERFLOW:	   return PARS_DEBUG_GLErrorStrings[4];
		case GL_STACK_UNDERFLOW:   return PARS_DEBUG_GLErrorStrings[5];
		case GL_OUT_OF_MEMORY:     return PARS_DEBUG_GLErrorStrings[6];
		default:				   return PARS_DEBUG_GLErrorStrings[7];
	}
}

static GLenum _PARSGLError = GL_NO_ERROR;

#define _PARS_HaveGLError() (_PARSGLError!= GL_NO_ERROR)

static bool glError()
{
	_PARSGLError = glGetError();
	int nextErr = _PARSGLError;
	while (nextErr != GL_NO_ERROR)
		nextErr = glGetError();
	return _PARSGLError != GL_NO_ERROR;
}



#endif // PARS_DEBUG_WANT_GL

#endif   