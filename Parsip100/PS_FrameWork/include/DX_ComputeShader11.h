#pragma once
#ifndef DX_COMPUTESHADER_H
#define DX_COMPUTESHADER_H

#include "DX_Device11.h"
#include "PS_Vector.h"

#ifdef WIN32
#include <d3dcompiler.h>

namespace PS{

using namespace MATH;

class DXShader11
{
private:
	bool				 m_bCompiled;	
	DXDevice11*			 m_pDXDevice;
	ID3D11ComputeShader* m_pComputeShader;
	LPCWSTR m_pSrcFile;
	LPCSTR  m_pFuncEntryPoint;

public:
	//Without DXDEVICE
	DXShader11();
	DXShader11(LPCWSTR pSrcFile, 
			   LPCSTR pFuncName,
			   const D3D_SHADER_MACRO * pDefines);


	//With DXDevice
	DXShader11(const DXDevice11* lpDevice);
	DXShader11(const DXDevice11* lpDevice,
			   LPCWSTR pSrcFile, 
			   LPCSTR pFuncName,
			   const D3D_SHADER_MACRO * pDefines);

	~DXShader11();

	void release();

	static vec3ui createDispatchThreadGroups(UINT szInputWays);

	//Accessors (Getters)
	ID3D11ComputeShader*	getShader() const { return m_pComputeShader;}
	LPCWSTR getSourceFile() const { return m_pSrcFile;}
	LPCSTR  getEntryPointFunc() const { return m_pFuncEntryPoint;}

	//Setters
	void setDXDevice(const DXDevice11* lpDevice);
	

	//Creates Compute Shader from the specified source file
	//Loading from a bin file
	bool loadFromBinary( const char* chrShaderByteCode );
	HRESULT compile( LPCWSTR pSrcFile, 
					 LPCSTR pFuncName,
					 const D3D_SHADER_MACRO * pDefines,
					 bool bSaveByteCode = false);
	bool tryLoadBinThenCompile(LPCWSTR pSrcFile, 
							   LPCSTR pFuncName,
							   const D3D_SHADER_MACRO * pDefines);


	HRESULT createStructuredBuffer(UINT uElementSize, UINT uCount, VOID* pInitData, ID3D11Buffer** ppBufOut);
	HRESULT createRawBuffer(UINT uSize, VOID* pInitData, ID3D11Buffer** ppBufOut );
	HRESULT createBufferSRV( ID3D11Buffer* pBuffer, ID3D11ShaderResourceView** ppSRVOut );
	HRESULT createBufferUAV( ID3D11Buffer* pBuffer, ID3D11UnorderedAccessView** ppUAVOut );
	HRESULT createBuffer(const D3D11_BUFFER_DESC *pDesc, const D3D11_SUBRESOURCE_DATA *pInitialData, ID3D11Buffer** ppBuffer);

	HRESULT map(ID3D11Resource *pResource, UINT Subresource, D3D11_MAP MapType, UINT MapFlags, D3D11_MAPPED_SUBRESOURCE *pMappedResource);
	void	unMap(ID3D11Resource *pResource, UINT SubResource);


	void run( UINT nNumViews, ID3D11ShaderResourceView** pShaderResourceViews, 
              ID3D11Buffer* pCBCS, void* pCSData, DWORD dwNumDataBytes,
              UINT nNumUAVs, ID3D11UnorderedAccessView** pUnorderedAccessViews,
              UINT X, UINT Y, UINT Z );

	void run( UINT nNumViews, ID3D11ShaderResourceView** pShaderResourceViews, 
              ID3D11Buffer* pCBCS, void* pCSData, DWORD dwNumDataBytes,
              ID3D11UnorderedAccessView* pUnorderedAccessView,
              UINT X, UINT Y, UINT Z );


	ID3D11Buffer* createAndCopyToDebugBuffer( ID3D11Buffer* pGPUBuffer );


};

}

#endif
#endif

