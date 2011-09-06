#include "GL_Shader.h"
#include "PS_ErrorManager.h"
#include "PS_FileDirectory.h"

#include <iostream>
#include <fstream>
#include <GL/glew.h>

using namespace PS::FILESTRINGUTILS;

namespace PS{



bool CGLShaderProgram::init()
{
	m_bIsRunning = false;
	m_bCompiled = false;	

	m_vertexShader = 0;
	m_fragmentShader = 0;
	m_program = 0;

	GLenum err = glewInit();
	if(err != GLEW_OK)
	{
		DAnsiStr strError = printToAStr("GLEW init error: %s", glewGetErrorString(err));	
		ReportError(strError.ptr());
		return false;
	}		

	return true;
}

CGLShaderProgram::~CGLShaderProgram()
{
}

UINT CGLShaderProgram::getUniformLocation(const GLchar* name)
{
	//GL_INVALID_OPERATION
	//GL_INVALID_VALUE
	return glGetUniformLocation(m_program, name);
}

int	 CGLShaderProgram::compile(const char* vShaderFile, const char* fShaderFile)
{
	if((vShaderFile == NULL)||(fShaderFile == NULL))
		return ERR_SHADER_FILE_NOT_FOUND;

	m_strVertexShaderFile = DAnsiStr(vShaderFile);
	m_strFragmentShaderFile = DAnsiStr(fShaderFile);
	
	//Read both files
	bool bres = readShaderCode(m_strVertexShaderFile, m_strVertexShaderCode);
	if(!bres) return ERR_SHADER_FILE_NOT_FOUND;
	bres = readShaderCode(m_strFragmentShaderFile, m_strFragmentShaderCode);
	if(!bres) return ERR_SHADER_FILE_NOT_FOUND;

	//Compile and Link
	GLint status = 0;
	//buffer for error messages 
	GLchar *ebuffer; 
	//length of error messages
	GLsizei elength;  
	DAnsiStr strError;

	m_program		 = glCreateProgram();
	m_vertexShader   = glCreateShader(GL_VERTEX_SHADER);
	m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glAttachShader(m_program, m_vertexShader);
	glAttachShader(m_program, m_fragmentShader);

	//Get the pointer to both shader codes
	const GLchar * cvSource= m_strVertexShaderCode.ptr();
	const GLchar * cfSource= m_strFragmentShaderCode.ptr();

	glShaderSource(m_vertexShader, 1, &cvSource, NULL);
	glShaderSource(m_fragmentShader, 1, &cfSource, NULL);

	//Compile Vertex Shader
	glCompileShader(m_vertexShader);

	//Check for compile errors
	glGetShaderiv(m_vertexShader, GL_COMPILE_STATUS, &status);
	if(status==GL_FALSE)
	{	
		//Get Error code from GL shader log
		glGetShaderiv(m_vertexShader, GL_INFO_LOG_LENGTH, &elength);
		ebuffer = new GLchar[elength];
		glGetShaderInfoLog(m_vertexShader, elength, NULL, ebuffer);

		strError = printToAStr("VertexShader [%s] Compile Error: %s", extractFileName(vShaderFile), ebuffer);
		OutputDebugStringA(ebuffer);
		SAFE_DELETE(ebuffer);

		ReportError(strError.ptr());		
		return ERR_SHADER_COMPILE_ERROR;
	}

	//Compile Fragment Shader
	glCompileShader(m_fragmentShader);

	//Check for compile errors
	glGetShaderiv(m_fragmentShader, GL_COMPILE_STATUS, &status);
	if(status==GL_FALSE)
	{
		//Get Error code from GL shader log
		glGetShaderiv(m_fragmentShader, GL_INFO_LOG_LENGTH, &elength);
		ebuffer = new GLchar[elength];
		glGetShaderInfoLog(m_fragmentShader, elength, NULL, ebuffer);

		strError = printToAStr("FragmentShader [%s] Compile Error: %s", extractFileName(fShaderFile), ebuffer);
		OutputDebugStringA(ebuffer);

		SAFE_DELETE(ebuffer);
		ReportError(strError.ptr());		
		return ERR_SHADER_COMPILE_ERROR;
	}

	//Link Shader
	glLinkProgram(m_program);

	//Check for Linking Errors
	glGetShaderiv(m_program, GL_LINK_STATUS, &status);
	if(status==GL_FALSE)
	{		
		glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &elength);

		ebuffer = new GLchar[elength];
		glGetProgramInfoLog(m_program, elength, &elength, ebuffer);
		strError = printToAStr("Link Error: %s", ebuffer);

		SAFE_DELETE(ebuffer);
		ReportError(strError.ptr());
		return ERR_SHADER_LINK_ERROR;
	}

	m_bCompiled = true;

	return GL_SUCCESS;
}

bool CGLShaderProgram::run()
{
	if(!isReadyToRun())
		return false;

	//Run program
	glUseProgram(m_program);

	m_bIsRunning = true;
	return true;
}

bool CGLShaderProgram::readShaderCode(const DAnsiStr& strFilePath, DAnsiStr& strCode)
{
	std::ifstream fp;
	fp.open(strFilePath.ptr(), std::ios::binary);	
	if(!fp.is_open())
		return false;
	
	size_t size;
	fp.seekg(0, std::ios::end);
	size = fp.tellg();
	fp.seekg(0, std::ios::beg);

	char * buf = new char[size+1];
	//Read file content
	fp.read(buf, size);		
	buf[size] = '\0';

	strCode = DAnsiStr(buf);
	SAFE_DELETE(buf);
	fp.close();		

	return true;
}

bool CGLShaderProgram::isReadyToRun()
{
	GLboolean bValid = glIsProgram(m_program);		
	return m_bCompiled && (bValid == GL_TRUE);
}

void CGLShaderProgram::setUniform1f( int location, float v0 )
{
	glUniform1f(location, v0);
}

}
