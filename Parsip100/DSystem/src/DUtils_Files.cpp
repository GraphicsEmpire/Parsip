//==================================================================
/// DUtils_Files.cpp
///
/// Created by Davide Pasca - 2009/7/23
/// See the file "license.txt" that comes with this project for
/// copyright info. 
//==================================================================

#if defined(WIN32)
#include <Windows.h>

#elif defined(__linux__)

#endif

#include <direct.h>
#include "DUtils_Files.h"

//==================================================================
namespace DUT
{

//==================================================================
bool GrabFile( const char *pFileName, DVec<U8> &out_data )
{
	out_data.clear();
	FILE	*pFile;
	errno_t err = fopen_s(&pFile, pFileName, "rb" );
	if(err != 0)
	{
		return false;
	}
	
	fseek( pFile, 0, SEEK_END );
	out_data.resize( ftell( pFile ) );
	fseek( pFile, 0, SEEK_SET );
	
	fread( &out_data[0], 1, out_data.size(), pFile );
	
	fclose( pFile );
	
	return true;
}

//==================================================================
FILE *BeginGrabFile( const char *pFileName, size_t &out_fileSize )
{
	out_fileSize = 0;

	FILE	*pFile;
	errno_t err = fopen_s(&pFile, pFileName, "rb" );
	if(err != 0)
	{
		return NULL;
	}
	
	fseek( pFile, 0, SEEK_END );
	out_fileSize = ftell( pFile );
	fseek( pFile, 0, SEEK_SET );

	return pFile;
}

//==================================================================
bool EndGrabFile( FILE *pFile, void *pDest, size_t readSize )
{
	if ( readSize != fread( pDest, 1, readSize, pFile ) )
	{
		fclose( pFile );
		return false;
	}

	fclose( pFile );
	return true;
}

//==================================================================
bool FileExists( const char *pFileName )
{
	FILE	*pFile;
	errno_t err = fopen_s(&pFile, pFileName, "rb" );
	if(err == 0)
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
const char *GetFileNameOnly( const char *pPathFileName )
{
	int	idx = 0;
	int	len = (int)strlen( pPathFileName );
	
	for (int i=len-1; i >= 0; --i)
		if ( pPathFileName[i] == '/' || pPathFileName[i] == '\\' )
			return pPathFileName + i + 1;
			
	return pPathFileName + len;
}

//==================================================================
const char *GetFileNameExt( const char *pPathFileName )
{
	int	idx = 0;
	int	len = (int)strlen( pPathFileName );
	
	for (int i=len-1; i >= 0; --i)
		if ( pPathFileName[i] == '.' )
			return pPathFileName + i + 1;
			
	return pPathFileName + len;
}

//==================================================================
char *GetFileNameExt( char *pPathFileName )
{
	return (char *)GetFileNameExt( (const char *)pPathFileName );
}

//==================================================================
}
