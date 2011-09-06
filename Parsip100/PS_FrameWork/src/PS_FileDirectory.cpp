//#include "stdafx.h"
#include "PS_FileDirectory.h"
#include "Windows.h"
#include <io.h>

#include <fstream>

namespace PS
{

namespace FILESTRINGUTILS
{
size_t chrBufferLen(char* buf, size_t maxLen)
{
	size_t i = 0;
	while (i < maxLen)
	{
		if(buf[i] == '\0')
			return i;
		i++;
	}
	return maxLen;
}

void chrZeroMemory(char* buf, size_t len)
{
	memset((void*)buf, 0, len);
}

int	chrFromString(char* dstBuf, size_t bufLen, string str)
{
	chrZeroMemory(dstBuf, bufLen);
	if((size_t)str.length() < bufLen)	
	{
		strcpy_s(dstBuf, bufLen, str.c_str());
		return bufLen;
	}
	else
	{
		string temp = str.substr(0, bufLen-1);
		strcpy_s(dstBuf, bufLen, temp.c_str());
		return bufLen-1;
	}
}


long getFileSize(const char* chrFilePath)
{
	ifstream ifs(chrFilePath, ios::in | ios::binary );
	if(!ifs.is_open())
		return -1;
		
	long begin, end;
	begin = ifs.tellg();
	ifs.seekg(0, ios::end);
	end = ifs.tellg();	
	ifs.close();

	return end - begin;
}


//==================================================================
bool fileExists( const char *pFileName )
{
	FILE* pFile;
	errno_t err = fopen_s( &pFile, pFileName, "rb" );
	
	if (( pFile )&&( err == 0 ))
	{
		fclose( pFile );
		return true;
	}
	else
	{
		return false;
	}	
}

//==================================================================
bool fileExistsW( const wchar_t *pwcFileName)
{
	char chrFileName[2048];
	if(convertWideCharToMultiByte(2048, chrFileName, const_cast<wchar_t *>(pwcFileName)))
		return fileExists(chrFileName);
	else
		return false;
}

//==================================================================
bool convertMultiByteToWideChar(wchar_t *pwcDst, const char *pSrc)
{
	int len = strlen(pSrc);

	size_t ctConverted;
	errno_t err = mbstowcs_s(&ctConverted, pwcDst, len+1, pSrc, _TRUNCATE);
	//MultiByteToWideChar(CP_ACP, 0, input, len + 1, output, sizeof(output)/sizeof(output[0]));	
	//output[len] = 0;
	return (err == 0);
}

//==================================================================
VOID WINAPI DebugOutput( LPCWSTR strMsg, ... )
{
	WCHAR strBuffer[512];

	va_list args;
	va_start(args, strMsg);
	vswprintf_s( strBuffer, 512, strMsg, args );
	strBuffer[511] = L'\0';
	va_end(args);

	OutputDebugString( strBuffer );
}

//==================================================================
//==================================================================
bool convertWideCharToMultiByte(int szBuffer, const char *pDst, wchar_t *pwcSrc)
{
	//int len = wcslen(pwcSrc);

	int res = WideCharToMultiByte(CP_ACP, 0, pwcSrc, -1, (LPSTR)pDst, szBuffer, NULL, NULL);
	return (res > 0);
}

//==================================================================
DWideStr getExePathWStr()
{
	WCHAR wszExeName[MAX_PATH];

	ZeroMemory(wszExeName, sizeof(WCHAR) * MAX_PATH);	
	GetModuleFileName(NULL, wszExeName, MAX_PATH);

	DWideStr str(wszExeName);
	return str;	
}
//==================================================================
DAnsiStr getExePath()
{
	char buff[1024];
	getExePath(buff, 1024);		
	DAnsiStr strOut = DAnsiStr(buff);
	return strOut;	
}
//==================================================================
void getExePath(const char *exePath, int szBuffer)
{
	WCHAR wszExeName[MAX_PATH + 1];
	wszExeName[MAX_PATH] = 0;
	GetModuleFileName(NULL, wszExeName, sizeof(wszExeName) - 1);
	
	WideCharToMultiByte(CP_ACP, 0, wszExeName, -1, (LPSTR)exePath, szBuffer, NULL, NULL);
}

//==================================================================
DAnsiStr extractFileName(DAnsiStr strPathFileName)
{
	size_t npos;

	DAnsiStr strOutput;
	if(strPathFileName.rfind(L'/', npos) || strPathFileName.rfind(L'\\', npos))
	{
		strOutput = strPathFileName.substr(npos+1);
	}

	return strOutput;
}
//==================================================================
string	extractFileName( string strPathFileName )
{
	string str;
	int len = strPathFileName.length();

	for (int i=len-1; i >= 0; --i)
		if ( strPathFileName[i] == '/' || strPathFileName[i] == '\\' )
		{
			str = strPathFileName.substr(i+1);
			return str;
		}

	str = strPathFileName.substr(len);
	return str;
}

//==================================================================
const char *extractFileName( const char *pPathFileName )
{	
	int	len = (int)strlen( pPathFileName );
	
	for (int i=len-1; i >= 0; --i)
		if ( pPathFileName[i] == '/' || pPathFileName[i] == '\\' )
			return pPathFileName + i + 1;
			
	return pPathFileName + len;
}
//==================================================================
char* changeFileExtChr_s(char* pPathFileName, size_t nElements, const char* pExtWithDot)
{
	char buff[1024];
	char* p = pPathFileName;
	int	len = (int)strlen( p );
	
	for (int i=len-1; i >= 0; --i)
	{
		if ( p[i] == '.' )
		{		
			p[i] = '\0';
			strcpy_s(buff, 1024, p);
			strcat_s(buff, 1024, pExtWithDot);
			strcpy_s(p, nElements, buff);
			return p;
		}

	}
	strcpy_s(buff, 1024, p);
	strcat_s(buff, 1024, pExtWithDot);
	strcpy_s(p, nElements, buff);
	return p;
}
//==================================================================
string changeFileExt(string strFilePath, string strExt)
{
	//size_t len = strFilePath.length() + strExt.length() + 1;
	
	//char chrFilePath = new char[len];
	char chrFilePath[1024];
	strcpy_s(chrFilePath, strFilePath.c_str());

	char chrExt[10];
	strcpy_s(chrExt, strExt.c_str());

	changeFileExtChr_s(chrFilePath, 1024, chrExt);
	string strOutput = string(chrFilePath);

	return strOutput;
}
//==================================================================
DAnsiStr changeFileExtA(const DAnsiStr& strFilePath, const DAnsiStr& strExtWithDot)
{
	DAnsiStr strOut;
	size_t npos;
	if(strFilePath.rfind('.', npos))
	{
		strOut = strFilePath.substr(0, npos);
		strOut += strExtWithDot;
	}
	else
		strOut = strFilePath + strExtWithDot;

	return strOut;
}
//==================================================================
DAnsiStr createNewFileAtRoot(const char* pExtWithDot)
{
	WCHAR wszExeName[MAX_PATH];

	GetModuleFileName(NULL, wszExeName, MAX_PATH);

	DAnsiStr strOutput(wszExeName);	
	size_t posDot;
	if(strOutput.rfind(L'.', posDot))
	{
		DAnsiStr temp = strOutput.substr(0, posDot);
		temp.appendFromT(pExtWithDot);
		return temp;
	}
	else
		return strOutput;	
}

//==================================================================
DAnsiStr extractFilePath(const DAnsiStr& fileName)
{
	size_t npos;
	DAnsiStr strOutput;
	if(fileName.rfind(L'/', npos) || fileName.rfind(L'\\', npos))	
	{
		strOutput = fileName.substr(0, npos+1);
	}
	return strOutput;	
}
//==================================================================
string extractFilePath(const string &fileName)
{
	return fileName.substr(0, fileName.rfind("\\")+1);
}
//==================================================================
const char*	extractFileExt( const char *pPathFileName )
{
	int	len = (int)strlen( pPathFileName );
	
	for (int i=len-1; i >= 0; --i)
		if ( pPathFileName[i] == '.' )
			return pPathFileName + i + 1;
			
	return pPathFileName + len;
}

//==================================================================
char*	extractFileExt( char *pPathFileName )
{
	return (char *)extractFileExt( (const char *)pPathFileName );
}
//==================================================================
DAnsiStr extractFileExt(const DAnsiStr& strPathFileName)
{
	size_t npos;
	DAnsiStr strOut;
	if(strPathFileName.rfind('.', npos))
	{
		strOut = strPathFileName.substr(npos+1);
	}
	return strOut;	
}
//==================================================================
string	extractDirName( const char *pPathFileName)
{
	const char *pInFPathnameEnd = pPathFileName + strlen( pPathFileName );

	const char *pFNamePtr = extractFileName( pPathFileName );

	if ( pFNamePtr >= pInFPathnameEnd )
		return NULL;
	else
	{
		string str(pPathFileName);		
		size_t	len = pFNamePtr - pPathFileName - 1;

		str.resize(len);
		return str;
	}
}

//==================================================================
std::string strToUpper(std::string& s)
{
	std::string::iterator i = s.begin();
	std::string::iterator end = s.end();
	while(i != end)
	{
		*i = toupper((unsigned char)*i);
		++i;
	}

	return s;
}
//==================================================================
std::string strToLower(std::string& s)
{
	std::string::iterator i = s.begin();
	std::string::iterator end = s.end();
	while(i != end)
	{
		*i = tolower((unsigned char)*i);
		++i;
	}

	return s;
}
//==================================================================
int listFilesInDirS(DVec<DAnsiStr>& lstFiles, const DAnsiStr strDir, const DAnsiStr strExtensions, bool storeWithPath)
{

	DAnsiStr strResolvedDir;
	
	lstFiles.resize(0);
	if(strDir.length() == 0)	
		strResolvedDir = extractFilePath(getExePath());	
	else
		strResolvedDir = strDir;

	char chrDir[1024];
	if(strExtensions.length() != 0)
		sprintf_s(chrDir, "%s/*.%s", strResolvedDir.c_str(), strExtensions.c_str());
	else
		sprintf_s(chrDir, "%s/*.*", strResolvedDir.c_str());


	//WCHAR   wszFileName[512];
	_finddatai64_t	findData;

	DAnsiStr temp;
	intptr_t	handle = _findfirst64( chrDir, &findData );
	if ( handle != -1 )
	{
		int	ret = 0;
		do
		{
			if (storeWithPath)			
			{
				temp = strResolvedDir + "\\";
				temp += findData.name;
				lstFiles.push_back(temp);
			}
			else
			{
				temp = findData.name;
				lstFiles.push_back(temp);
			}
		
			//MultiByteToWideChar(CP_ACP, 0, findData.name, strlen(findData.name) + 1, wszFileName, sizeof(wszFileName) / sizeof(wszFileName[0]));
			//DUT::SSPrintFS( "%s / %s", pDirName, findData.name ).c_str()
			//g_hudRibRender.GetListBox( IDC_LISTBOXSAMPLEFILES )->AddItem(wszFileName, NULL);
			ret = _findnext64( handle, &findData );
		} while ( ret == 0 );

		_findclose( handle );
	}

	return (int)lstFiles.size();
}

//==================================================================
int listFilesInDir(DVec<DAnsiStr>& lstFiles, const char* pDir, const char* pExtensions, bool storeWithPath)
{
	char chrDir[1024];
	DAnsiStr strResolvedDir;

	lstFiles.resize(0);
	if(pDir == NULL)	
		strResolvedDir = extractFilePath(getExePath());		
	else
		strResolvedDir = DAnsiStr(pDir);
		

	if(pExtensions != NULL)
		sprintf_s(chrDir, "%s/*.%s", strResolvedDir.c_str(), pExtensions);
	else
		sprintf_s(chrDir, "%s/*.*", strResolvedDir.c_str());


	//WCHAR   wszFileName[512];
	_finddatai64_t	findData;
	DAnsiStr temp;

	intptr_t	handle = _findfirst64( chrDir, &findData );
	if ( handle != -1 )
	{
		int	ret = 0;
		do
		{
			if (storeWithPath)			
			{
				temp = strResolvedDir + "\\";
				temp += findData.name;
				lstFiles.push_back(temp);
			}
			else
			{
				temp = findData.name;
				lstFiles.push_back(temp);
			}
		
			//MultiByteToWideChar(CP_ACP, 0, findData.name, strlen(findData.name) + 1, wszFileName, sizeof(wszFileName) / sizeof(wszFileName[0]));
			//DUT::SSPrintFS( "%s / %s", pDirName, findData.name ).c_str()
			//g_hudRibRender.GetListBox( IDC_LISTBOXSAMPLEFILES )->AddItem(wszFileName, NULL);
			ret = _findnext64( handle, &findData );
		} while ( ret == 0 );

		_findclose( handle );
	}

	return (int)lstFiles.size();
}

//==================================================================
DAnsiStr GetDirNameFromFPathName( const char *pInFPathname )
{
	DAnsiStr strOut;
	const char *pInFPathnameEnd = pInFPathname + strlen( pInFPathname );

	const char *pFNamePtr = extractFileName( pInFPathname );

	if ( pFNamePtr >= pInFPathnameEnd )
		return strOut;
	else
	{
		size_t	len = pFNamePtr - pInFPathname - 1;

		strOut.copyFromT(pInFPathname);		
		strOut = strOut.substr(0, len);		
		return strOut;
	}
}

//==================================================================
// Not really needed for now by RibTools
DAnsiStr GetFullDirNameFromFPathName( const char *pInFPathname )
{
#if 0
	return GetDirNameFromFPathName( pInFPathname );

#else
	DAnsiStr	tmp = GetDirNameFromFPathName( pInFPathname );

#if defined(WIN32)
	char	buff[2048];
	GetFullPathNameA( tmp.c_str(), _countof(buff), buff, NULL );

#elif defined(__linux__)
	DASSERT( 0 );	// TODO

#endif

	return buff;
#endif

}

//===============================================================
DAnsiStr ASPrintFS( const char *pFmt, ... )
{
	va_list	vl;
	va_start( vl, pFmt );

	char	buff[1024];
	vsnprintf_s( buff, _countof(buff)-1, _TRUNCATE, pFmt, vl );

	va_end( vl );

	DAnsiStr strOut = buff;
	return strOut;
}

bool WriteTextFile( DAnsiStr strFN, const DVec<DAnsiStr>& content )
{
	ofstream ofs(strFN.ptr(), ios::out | ios::trunc);
	if(!ofs.is_open())
		return false;

	DAnsiStr strLine;
	for(size_t i=0; i < content.size(); i++)
	{
		strLine = content[i];
		if(strLine.length() > 0)
		{
			ofs << strLine << '\0' << '\n';
		}
	}
	ofs.close();

	return true;
}

bool ReadTextFile( DAnsiStr strFN, DVec<DAnsiStr>& content )
{
	ifstream ifs(strFN.ptr(), ios::in);
	if(!ifs.is_open())
		return false;

	DAnsiStr strLine;
	char buffer[2048];

	while( !ifs.eof())
	{
		ifs.getline(buffer, 2048);
		//ifs >> strLine;
		strLine.copyFromT(buffer);
		strLine.trim();
		content.push_back(strLine);
	}	
	ifs.close();

	return true;
}
}
}