#pragma once
#ifndef CGLSHADER_H
#define CGLSHADER_H

#include "PS_String.h"

#define GL_SUCCESS				   1
#define ERR_SHADER_FILE_NOT_FOUND -1
#define ERR_SHADER_COMPILE_ERROR  -2
#define ERR_SHADER_LINK_ERROR	  -3

namespace PS{

//An OpenGL shader program
class CGLShaderProgram
{
private:
	DAnsiStr m_strVertexShaderFile;
	DAnsiStr m_strFragmentShaderFile;
	
	DAnsiStr m_strVertexShaderCode;
	DAnsiStr m_strFragmentShaderCode;
	UINT m_program;
	UINT m_vertexShader;
	UINT m_fragmentShader;
	bool   m_bIsRunning;
	bool   m_bCompiled;
	
	bool readShaderCode(const DAnsiStr& strFilePath, DAnsiStr& strCode);	
public:
	typedef enum ShaderType {stVertex = 0, stFragment = 1};


	CGLShaderProgram() 
	{}

	~CGLShaderProgram();

	bool init();

	void setUniform1f(int location, float v0);

	UINT getShaderProgram() const { return m_program;}
	UINT getVertexShader() const { return m_vertexShader;}
	UINT getFragmentShader() const { return m_fragmentShader;}
	UINT getUniformLocation(const char* name);
	
	bool isReadyToRun();

	int	 compile(const char* vShaderFile, const char* fShaderFile);	
	bool run();	
};

}
#endif


