//#include "stdafx.h"
#include "DX_ShaderManager.h"
#include "PS_FileDirectory.h"
#include "PS_AppConfig.h"

namespace PS
{
	using namespace FILESTRINGUTILS;

	DWideStr findShaderProgram(const DAnsiStr& strShaderName)
	{
		DAnsiStr strPath;

		//Try using app.inf file
		PS::CAppConfig* cfg = new PS::CAppConfig();
		if(cfg->setForRead())
		{
			strPath = cfg->readString("paths", "computeshaders", strPath);		
			if(strPath.length() > 0)
			{
				strPath += "\\";
				strPath += strShaderName;
			}
			else
				strPath = strShaderName;
		}
		SAFE_DELETE(cfg);
		if(FileExists(strPath))
			return toWideString(strPath);

		
		//Try directories near exe 
		DAnsiStr temp;
		strPath = ExtractFilePath(GetExePath());
		temp = strPath + strShaderName;
				
		if(FileExists(temp.ptr()))
			return toWideString(temp);
		else 
		{
			temp = strPath;
			temp += DAnsiStr("Resources\\DXComputeShaders\\");
			temp += strShaderName;
			return toWideString(temp);
		}		
	}
}