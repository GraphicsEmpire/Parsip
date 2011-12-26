/*
 * GL_ShaderManager.h
 *
 *  Created on: May 25, 2011
 *      Author: pshirazi
 */

#ifndef GL_SHADERMANAGER_H_
#define GL_SHADERMANAGER_H_

#include "PS_ResourceManager.h"
#include "PS_ShaderGLSL.h"
#include "loki/Singleton.h"


#define DEFAULT_FRAGMENT_EXT ".fsh"
#define DEFAULT_VERTEX_EXT ".vsh"

using namespace std;
using namespace Loki;
using namespace PS::CONTAINERS;

namespace PS{
namespace SHADER{


	class CGLShaderManager : public CResourceManager<CGLShaderProgram>
	{
	public:
		CGLShaderManager();
		virtual ~CGLShaderManager();

		//virtual T* loadResource(const DAnsiStr& inStrFilePath, DAnsiStr& inoutStrName) = 0;
		CGLShaderProgram* loadResource(const DAnsiStr& inStrFilePath, DAnsiStr& inoutStrName);

		void setFragmentExt(const DAnsiStr& strFragmentExt) {m_strFragExt = strFragmentExt;}
		void setVertexExt(const DAnsiStr& strVertexExt) {m_strVertexExt = strVertexExt;}
	private:
		DAnsiStr m_strFragExt;
		DAnsiStr m_strVertexExt;
	};

	typedef Loki::SingletonHolder<CGLShaderManager, CreateUsingNew, PhoenixSingleton> CGLShaderManagerSingleton;

}
}

#endif /* GL_SHADERMANAGER_H_ */
