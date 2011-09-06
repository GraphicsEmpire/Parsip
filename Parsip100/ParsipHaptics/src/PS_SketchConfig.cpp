#include "PS_SketchConfig.h"

namespace PS{
	CSketchConfig::CSketchConfig()
	{
		m_strFileName = "MEM";
		m_fmode = fmMemoryStream;
	}

	CSketchConfig::CSketchConfig( const std::vector<DAnsiStr>& inputContent )
	{
		m_strFileName = "MEM";
		m_fmode = fmMemoryStream;
		setContentBuffer(inputContent);
	}

	bool CSketchConfig::readIntArray(DAnsiStr section, DAnsiStr variable, int ctExpected, std::vector<int>& arrayInt)
	{
		DAnsiStr strVal;
		if(readValue(section, variable, strVal))
		{				
			size_t pos;				
			int iComp = 0;
			DAnsiStr strTemp;			
			if(strVal.firstChar() == '(')
				strVal = strVal.substr(1);
			else
				return false;
			while(strVal.lfind(',', pos))
			{
				strTemp = strVal.substr(0, pos);
				strVal = strVal.substr(pos + 1);					
				strVal.removeStartEndSpaces();
				arrayInt.push_back(atoi(strTemp.ptr()));
				iComp++;
			}				

			if(strVal.length() >= 1)
			{
				if(strVal.lastChar() == ')')
				{
					strTemp = strVal.substr(0, strVal.length() - 1);
					strTemp.removeStartEndSpaces();
					arrayInt.push_back(atoi(strTemp.ptr()));
				}					
			}
		}
		return (arrayInt.size() == ctExpected);
	}

	int CSketchConfig::writeIntArray(DAnsiStr section, DAnsiStr variable, const std::vector<int>& arrayInt)
	{
		char chrVariable[128];
		DAnsiStr strValue;
		if(arrayInt.size() > 1)
		{				
			for(size_t i=0; i<arrayInt.size(); i++)
			{
				if(i == 0)
					sprintf_s(chrVariable, 128, "(%d, ", arrayInt[i]);
				else if(i == arrayInt.size() - 1)
					sprintf_s(chrVariable, 128, "%d)", arrayInt[i]);
				else 
					sprintf_s(chrVariable, 128, "%d, ", arrayInt[i]);

				strValue += DAnsiStr(chrVariable);
			}
			writeValue(section, variable, strValue);
		}
		else if(arrayInt.size() == 1)			
		{
			sprintf_s(chrVariable, 128, "(%d)", arrayInt[0]);
			writeValue(section, variable, DAnsiStr(chrVariable));
		}			
		else
			writeValue(section, variable, DAnsiStr("()"));
		return arrayInt.size();
	}

	vec3f CSketchConfig::readVec3f(DAnsiStr section, DAnsiStr variable)
	{
		DAnsiStr strVal;
		vec3f res;
		if(readValue(section, variable, strVal))
		{
			float f[4];
			size_t pos;				
			int iComp = 0;
			DAnsiStr strTemp;		

			if(strVal.firstChar() == '(')
				strVal = strVal.substr(1);
			else
				return res;
			while(strVal.lfind(',', pos))
			{
				strTemp = strVal.substr(0, pos);
				strVal = strVal.substr(pos + 1);					
				strVal.removeStartEndSpaces();
				f[iComp] = static_cast<float>(atof(strTemp.ptr()));
				iComp++;
			}				

			if(strVal.length() >= 1 && iComp < 3)
			{
				if(strVal.lastChar() == ')')
				{
					strTemp = strVal.substr(0, strVal.length() - 1);
					strTemp.removeStartEndSpaces();
					f[iComp] = static_cast<float>(atof(strTemp.ptr()));					
				}					
			}

			res.set(f);
		}
		return res;
	}

	void CSketchConfig::writeVec3f(DAnsiStr section, DAnsiStr variable, vec3f val)
	{
		char chrVariable[128];
		sprintf_s(chrVariable, 128, "(%f, %f, %f)", val.x, val.y, val.z);
		writeValue(section, variable, DAnsiStr(chrVariable));
	}

	vec4f CSketchConfig::readVec4f(DAnsiStr section, DAnsiStr variable)
	{
		DAnsiStr strVal;
		vec4f res;
		if(readValue(section, variable, strVal))
		{
			float f[4];
			size_t pos;				
			int iComp = 0;
			DAnsiStr strTemp;		

			if(strVal.firstChar() == '(')
				strVal = strVal.substr(1);
			else
				return res;
			while(strVal.lfind(',', pos))
			{
				strTemp = strVal.substr(0, pos);
				strVal = strVal.substr(pos + 1);					
				strVal.removeStartEndSpaces();
				f[iComp] = static_cast<float>(atof(strTemp.ptr()));
				iComp++;
			}				

			if(strVal.length() >= 1 && iComp < 4)
			{
				if(strVal.lastChar() == ')')
				{
					strTemp = strVal.substr(0, strVal.length() - 1);
					strTemp.removeStartEndSpaces();
					f[iComp] = static_cast<float>(atof(strTemp.ptr()));					
				}					
			}

			res.set(f);
		}
		return res;
	}

	void CSketchConfig::writeVec4f(DAnsiStr section, DAnsiStr variable, vec4f val)
	{
		char chrVariable[128];
		sprintf_s(chrVariable, 128, "(%f, %f, %f, %f)", val.x, val.y, val.z, val.w);
		writeValue(section, variable, DAnsiStr(chrVariable));
	}

	DAnsiStr CSketchConfig::readString( DAnsiStr section, DAnsiStr variable )
	{
		DAnsiStr strVal;
		readValue(section, variable, strVal);			
		return strVal;
	}

}