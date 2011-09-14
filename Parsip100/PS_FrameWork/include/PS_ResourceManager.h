/*
 * CResourceManager.h
 *
 *  Created on: Jun 1, 2011
 *      Author: pourya
 */

#ifndef CRESOURCEMANAGER_H_
#define CRESOURCEMANAGER_H_

#ifdef WIN32
	#include <windows.h>
	#include <tchar.h>
#else
	#include <sys/types.h>
	#include <dirent.h>
	#include <errno.h>
#endif

#include <vector>
#include "PS_String.h"
#include "PS_FileDirectory.h"
#include "PS_ErrorManager.h"
#include "_dataTypes.h"

using namespace std;


namespace PS{
namespace CONTAINERS{

/*!
 * All named resources can be managed by using CResourceManager Base Class
 * TODO: Disk Resources should be handled with a different interface than
 * Memory resources. Right they all use the same base class
 */
template <class T>
class CResourceManager{
public:
	enum {
		PS_NAME_ALREADY_USED = -1,
		PS_INVALID_INDEX 	 = -2,
		PS_INVALID_FILE_PATH = -3,
		PS_INVALID_RESOURCE  = -4,
		PS_INVALID_NAME 	 = -5,
		PS_SUCCESS = 1
	};

	typedef T* RESPOINTER;


	struct NamedResource
	{
		RESPOINTER resource;
		DAnsiStr strName;
		DAnsiStr strPath;
	};

	CResourceManager() {}
	CResourceManager(const DAnsiStr& strRoot, const DAnsiStr& strWildCard)
	{
		m_strRoot = strRoot;
		m_strWildCard = strWildCard;
	}

	virtual ~CResourceManager()
	{
		cleanup();
	}

	void releaseAll() {	cleanup();}

	int countResources() const {return (int)m_storage.size();}
	int countFilesFound() const {return (int)m_vFilePaths.size();}
	bool hasError() const {return m_vFilePaths.size() != m_storage.size();}

	T* getResource(int index);
	void setResource(int index, T* pResource);

	DAnsiStr getResourceFilePath(int index) const;
	DAnsiStr getResourceFilePathByName(const char* chrName);
	DAnsiStr getResourceAliasName(int index);
	virtual T* getResourceByName(const char* chrName);

	virtual int loadFileNamesOnly();

	//Loading file resources from Disk
	virtual int loadAll();
	virtual int loadResource(int idxFilePath);
	virtual int loadResource(const char* chrFilePath, const char*chrAlias);

	//Adding
	virtual int addResourceFromMemory(T* aObject, const char* chrAlias);

	bool isNameUsed(const char* chrName);

	//Access
	DAnsiStr getRootDir() const {return m_strRoot;}
	void setRootDir(const DAnsiStr& root) {m_strRoot =  root;}

	DAnsiStr getWildCard() const {return m_strWildCard;}
	void setWildCard(const DAnsiStr& strWildCard) { m_strWildCard = strWildCard;}

protected:
	DAnsiStr m_strRoot;
	DAnsiStr m_strWildCard;
	std::vector<DAnsiStr> 	   m_vFilePaths;
	std::vector<NamedResource> m_storage;

	NamedResource& getNamedResource(int index) {return m_storage[index];}
	void setNamedResource(int index, const NamedResource& nr);

	//The only function that has to be implemented by each instance of ResourceManager
	virtual T* loadResource(const DAnsiStr& inStrFilePath, DAnsiStr& inoutStrName) = 0;
private:
	bool isIndex(int index) const { return ((index >=0)&&(index < (int)m_storage.size()));}
	void cleanup();
};


///////////////////////////////////////////////////////////////////////////////////
template<class T>
void CResourceManager<T>::cleanup()
{
	for(size_t i=0; i<m_storage.size(); i++)
	{
		T* ptr = m_storage[i].resource;
		SAFE_DELETE(ptr);
	}
	m_storage.resize(0);
	m_vFilePaths.resize(0);
}

template<class T>
DAnsiStr CResourceManager<T>::getResourceFilePath(int index) const
{
	if(isIndex(index))
		return m_storage[index].strPath;
}

template<class T>
DAnsiStr CResourceManager<T>::getResourceFilePathByName(const char* chrName)
{
	DAnsiStr strName = DAnsiStr(chrName);
	for(size_t i=0; i<m_storage.size(); i++)
	{
		if(m_storage[i].strName == strName)
		{
			return m_storage[i].strPath;
		}
	}

	//Write to log
	ReportErrorExt("Resource %s not found!", chrName);
	return strName;
}



template<class T>
DAnsiStr CResourceManager<T>::getResourceAliasName(int index)
{
	if(isIndex(index))
		return m_storage[index].strName;
}

template<class T>
T* CResourceManager<T>::getResource(int index)
{
	if(isIndex(index))
		return m_storage[index].resource;
	else
		return NULL;
}

template<class T>
void CResourceManager<T>::setResource(int index, T* pResource)
{
	if(isIndex(index))
		m_storage[index].resource = pResource;
}


template<class T>
T* CResourceManager<T>::getResourceByName(const char* chrName)
{
	DAnsiStr strName = DAnsiStr(chrName);
	for(size_t i=0; i<m_storage.size(); i++)
	{
		if(m_storage[i].strName == strName)
		{
			return m_storage[i].resource;
		}
	}

	//Write to log
	ReportErrorExt("Resource %s not found!", chrName);

	return NULL;
}

template<class T>
void CResourceManager<T>::setNamedResource(int index, const NamedResource& nr)
{
	if(!isIndex(index)) return;
	m_storage[index].resource = nr.resource;
	m_storage[index].strName  = nr.strName;
	m_storage[index].strPath  = nr.strPath;
}

template<class T>
int CResourceManager<T>::loadFileNamesOnly()
{
	cleanup();

#ifdef WIN32
	WIN32_FIND_DATA ffd;
	LARGE_INTEGER filesize;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	//DWORD dwError = 0;

	DWideStr strQuery = toWideString(m_strRoot + m_strWildCard);
	hFind = FindFirstFile(strQuery.ptr(), &ffd);
	if(hFind == INVALID_HANDLE_VALUE)
		return 0;

	do
	{
	    if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
	    	filesize.LowPart = ffd.nFileSizeLow;
	        filesize.HighPart = ffd.nFileSizeHigh;
	        //_tprintf(TEXT("  %s   %ld bytes\n"), ffd.cFileName, filesize.QuadPart);

	        //Instead of adding unloaded files to storage. Just load names first
	        m_vFilePaths.push_back(m_strRoot + DAnsiStr(ffd.cFileName));
	        /*
	        NamedResource nr;
	        nr.strName = DAnsiStr(ffd.cFileName);
	        nr.strPath = m_strRoot + DAnsiStr(ffd.cFileName);
	        nr.resource = NULL;
	        m_storage.push_back(nr);
	        */
        }
	}while (FindNextFile(hFind, &ffd) != 0);
   FindClose(hFind);

#else
   DIR *dp = NULL;
   struct dirent *dirp;

   DAnsiStr str = m_strRoot + ".";
   //DAnsiStr str = m_strRoot + m_strWildCard;
   dp  = opendir(str.c_str());
   if(dp == NULL)
   {
		return 0;
   }

   DAnsiStr strWantedExt = m_strWildCard.toUpper();
   size_t pos;
   if(strWantedExt.lfind('.', pos))
	   strWantedExt = strWantedExt.substr(pos+1);

   while ((dirp = readdir(dp)) != NULL)
   {
	   str = DAnsiStr(dirp->d_name);
	   str.trim();
	   str.removeStartEndSpaces();

	   DAnsiStr strExt = PS::FILESTRINGUTILS::ExtractFileExt(str);
	   if(strExt.toUpper() == strWantedExt)
	   {
		   //Instead of adding unloaded files to storage. Just load names first
	       m_vFilePaths.push_back(m_strRoot + str);
	        /*
		   NamedResource nr;
		   nr.strName = str;
		   nr.strPath = m_strRoot + str;
		   nr.resource = NULL;
		   m_storage.push_back(nr);
	        */

	   }
   }
   closedir(dp);
#endif
   return m_vFilePaths.size();
}

template <class T>
bool CResourceManager<T>::isNameUsed(const char* chrName)
{
	for(size_t i=0; i<m_storage.size(); i++)
	{
		if(m_storage[i].strName == DAnsiStr(chrName))
		{
			return true;
		}
	}
	return false;
}

template <class T>
int CResourceManager<T>::loadAll()
{
	loadFileNamesOnly();
	for(size_t i=0;i<m_vFilePaths.size(); i++)
	{
		loadResource(i);
	}

	return (int)m_storage.size();
}

template <class T>
int CResourceManager<T>::loadResource(int idxFilePath)
{
	if(((size_t)idxFilePath < 0) || ((size_t)idxFilePath >= m_vFilePaths.size()))
		return PS_INVALID_INDEX;

	DAnsiStr strFP = m_vFilePaths[idxFilePath];
	NamedResource nr;
	nr.strName  = PS::FILESTRINGUTILS::ExtractFileTitleOnly(strFP);
	nr.strPath  = strFP;
	nr.resource = loadResource(nr.strPath, nr.strName);

	if(nr.resource == NULL)
	{
		ReportErrorExt("Unable to load resource %s.", nr.strName.ptr());
		return PS_INVALID_RESOURCE;
	}

	m_storage.push_back(nr);
	return PS_SUCCESS;
}

template <class T>
int CResourceManager<T>::loadResource(const char* chrFilePath, const char* chrAlias)
{
	if(chrFilePath == NULL)
		return PS_INVALID_FILE_PATH;

	NamedResource nr;
	nr.strPath = DAnsiStr(chrFilePath);

	//Set Name
	if(chrAlias == NULL)
		nr.strName = PS::FILESTRINGUTILS::ExtractFileTitleOnly(nr.strPath);
	else
		nr.strName = DAnsiStr(chrAlias);
	nr.resource = loadResource(nr.strPath, nr.strName);

	//If loaded and the name is not a duplicate then add it
	if(nr.resource == NULL)
		return PS_INVALID_FILE_PATH;

	if(isNameUsed(nr.strName.ptr()))
		return PS_INVALID_NAME;

	m_storage.push_back(nr);
	return PS_SUCCESS;
}

template<class T>
int CResourceManager<T>::addResourceFromMemory(T* aObject, const char* chrAlias)
{
	if(aObject == NULL)
		return PS_INVALID_RESOURCE;
	if(chrAlias == NULL)
		return PS_INVALID_NAME;
	if(isNameUsed(chrAlias))
		return PS_NAME_ALREADY_USED;

	NamedResource nr;
	nr.strPath = DAnsiStr("MEM");
	nr.strName = DAnsiStr(chrAlias);
	nr.resource = aObject;
	m_storage.push_back(nr);

	return PS_SUCCESS;
}



}
}

#endif /* CRESOURCEMANAGER_H_ */
