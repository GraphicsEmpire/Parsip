#pragma once
#include <string>
#include <vector>
#include "PS_String.h"
#include "DSystem/include/DContainers.h"

using namespace std;

namespace PS{
	namespace FILESTRINGUTILS{

		size_t chrBufferLen(char* buf, size_t maxLen);
		void  chrZeroMemory(char* buf, size_t len);
		int	  chrFromString(char* dstBuf, size_t bufLen, string str);


		long getFileSize(const char* chrFilePath);

		bool fileExists( const char *pFileName);
		bool fileExistsW( const wchar_t *pwcFileName);

		bool convertMultiByteToWideChar(wchar_t *pwcDst, const char *pSrc);
		bool convertWideCharToMultiByte(int szBuffer, const char *pDst, wchar_t *pwcSrc);


		DAnsiStr extractFilePath(const DAnsiStr &fileName);
		string extractFilePath(const string &fileName);

		DWideStr getExePathWStr();
		DAnsiStr getExePath();	
		void getExePath(const char *exePath, int szBuffer);

		DAnsiStr createNewFileAtRoot(const char* pExtWithDot);	


		DAnsiStr extractFileName(DAnsiStr strPathFileName);
		const char*	extractFileName( const char *pPathFileName );
		string	extractFileName( string strPathFileName  );


		const char*	extractFileExt( const char *pPathFileName );
		char*	extractFileExt( char *pPathFileName );	
		DAnsiStr extractFileExt(const DAnsiStr& strPathFileName);

		string	extractDirName( const char *pPathFileName);

		char* changeFileExtChr_s(char* pPathFileName, size_t nElements, const char* pExtWithDot);	
		string changeFileExt(string strFilePath, string strExt);
		DAnsiStr changeFileExtA(const DAnsiStr& strFilePath, const DAnsiStr& strExtWithDot);

		//int listFilesInDir(std::vector<string>& lstFiles, const char* pDir = NULL, const char* pExtensions = NULL);	
		int listFilesInDir(DVec<DAnsiStr>& lstFiles, const char* pDir, const char* pExtensions, bool storeWithPath);	
		int listFilesInDirS(DVec<DAnsiStr>& lstFiles, const DAnsiStr strDir, const DAnsiStr strExtensions, bool storeWithPath);	

		//Some string manipulation 
		std::string strToUpper(std::string& s);
		std::string strToLower(std::string& s);

		void DebugOutput( LPCWSTR strMsg, ... );
		
		DAnsiStr GetDirNameFromFPathName( const char *pInFPathname );
		DAnsiStr GetFullDirNameFromFPathName( const char *pInFPathname );
		DAnsiStr ASPrintFS( const char *pFmt, ... );

		bool WriteTextFile(DAnsiStr strFN, const DVec<DAnsiStr>& content);
		bool ReadTextFile(DAnsiStr strFN, DVec<DAnsiStr>& content);

	}
}