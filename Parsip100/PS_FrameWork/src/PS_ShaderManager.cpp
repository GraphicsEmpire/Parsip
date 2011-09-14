/*
 * GL_ShaderManager.cpp
 *
 *  Created on: May 25, 2011
 *      Author: pshirazi
 */
#include "PS_ShaderManager.h"
#include "PS_FileDirectory.h"
#include "_dataTypes.h"

namespace PS{
namespace SHADER{

	CGLShaderManager::CGLShaderManager()
	{
		m_strFragExt = DAnsiStr(DEFAULT_FRAGMENT_EXT);
		m_strVertexExt = DAnsiStr(DEFAULT_VERTEX_EXT);
		setWildCard(m_strVertexExt);
	}

	CGLShaderManager::~CGLShaderManager()
	{
	}

	//CGLShaderProgram* loadResource(const DAnsiStr& inStrFilePath, DAnsiStr& inoutStrName);
	CGLShaderProgram* CGLShaderManager::loadResource(const DAnsiStr& inStrFilePath, DAnsiStr& inoutStrName)
	{
		if(PS::FILESTRINGUTILS::FileExists(inStrFilePath) == false) return NULL;

		//Set Alias Name for this shader
		//The shader can be fetched by this name with no extensions
		inoutStrName = PS::FILESTRINGUTILS::ExtractFileTitleOnly(inStrFilePath);

		DAnsiStr strVertexShaderFP = inStrFilePath;
		DAnsiStr strFragShaderFP = PS::FILESTRINGUTILS::ChangeFileExt(strVertexShaderFP, m_strFragExt);


		CGLShaderProgram* prg = new CGLShaderProgram();
		bool bres = (prg->compileFromFile(strVertexShaderFP.ptr(), strFragShaderFP.ptr()) == GL_SUCCESS);
		if(!bres)
			SAFE_DELETE(prg);

		return prg;
	}
}
}



