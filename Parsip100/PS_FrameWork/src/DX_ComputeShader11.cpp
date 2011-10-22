#include <fstream>
#include "DX_ComputeShader11.h"
#include "PS_FileDirectory.h"

#include "PS_ErrorManager.h"

#ifdef WIN32

namespace PS
{

DXShader11::DXShader11()
{
	m_bCompiled = false;
	m_pDXDevice = DXDevice11::getInstance();
	m_pComputeShader = NULL;	
}

DXShader11::DXShader11(LPCWSTR pSrcFile, 
					   LPCSTR pFuncName,
					   const D3D_SHADER_MACRO * pDefines)
{
	m_bCompiled = false;
	m_pDXDevice = DXDevice11::getInstance();
	m_pComputeShader = NULL;

	compile(pSrcFile, pFuncName, pDefines, false);
}



DXShader11::DXShader11(const DXDevice11 * lpDevice)
{
	m_bCompiled = false;
	m_pDXDevice = const_cast<DXDevice11*>(lpDevice);
	m_pComputeShader = NULL;
}

DXShader11::DXShader11(const DXDevice11 * lpDevice,
					   LPCWSTR pSrcFile, 
					   LPCSTR pFuncName,
					   const D3D_SHADER_MACRO * pDefines)
{
	m_bCompiled = false;
	m_pDXDevice = const_cast<DXDevice11*>(lpDevice);
	m_pComputeShader = NULL;

	compile(pSrcFile, pFuncName, pDefines, false);
}

DXShader11::~DXShader11()
{
	release();
}

void DXShader11::release()
{
	if(m_bCompiled)
	{
		SAFE_RELEASE( m_pComputeShader );
	}	
}

vec3ui	DXShader11::createDispatchThreadGroups(UINT szInputWays)
{
	vec3ui ctThreadGroups(szInputWays, 1, 1);
	if(ctThreadGroups.x > 65535)
	{
		ctThreadGroups.y = ctThreadGroups.x / 32768;
		if(ctThreadGroups.x % 32768 > 0)
			ctThreadGroups.y++;
		ctThreadGroups.x = 32768;		
	}
	return ctThreadGroups;
}

void DXShader11::setDXDevice(const DXDevice11* lpDevice)
{
	m_pDXDevice = const_cast<DXDevice11*>(lpDevice);
}

bool DXShader11::loadFromBinary( const char* chrShaderByteCode )									
{
	ifstream ifs(chrShaderByteCode, ios::in | ios::binary );
	if(!ifs.is_open())
		return false;

	
	ifstream::pos_type szBuffer;
	long begin, end;
	begin = ifs.tellg();
	ifs.seekg(0, ios::end);
	end = ifs.tellg();

	szBuffer = end - begin;

	//Define Buffer 
	char* buffer = new char[szBuffer];

	ifs.seekg(0, ios::beg);
	ifs.read(buffer, szBuffer);
	ifs.close();


	HRESULT hr = m_pDXDevice->getDevice()->CreateComputeShader( buffer, szBuffer, NULL, &m_pComputeShader);
	//Free Buffer
	delete[] buffer; buffer = NULL;

    return	SUCCEEDED(hr);
}


HRESULT DXShader11::compile( LPCWSTR pSrcFile, 
							 LPCSTR pFuncName,
							 const D3D_SHADER_MACRO * pDefines,
							 bool bSaveByteCode)
{
	HRESULT hr;
    
	if(!PS::FILESTRINGUTILS::FileExists( pSrcFile))
		return E_FAIL;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    // We generally prefer to use the higher CS shader profile when possible as CS 5.0 is better performance on 11-class hardware
	LPCSTR pProfile = ( m_pDXDevice->getDevice()->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0 ) ? "cs_5_0" : "cs_4_0";

    ID3DBlob* pErrorBlob = NULL;
    ID3DBlob* pBlob = NULL;
	hr = D3DX11CompileFromFile(pSrcFile, pDefines, NULL, pFuncName, pProfile, 
						       dwShaderFlags, NULL, NULL, &pBlob, &pErrorBlob, NULL );
    if ( FAILED(hr) )
    {
        if ( pErrorBlob )
		{
            OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
			ReportError((const char*)pErrorBlob->GetBufferPointer());
		}

        SAFE_RELEASE( pErrorBlob );
        SAFE_RELEASE( pBlob );    

        return hr;
    }    

	hr = m_pDXDevice->getDevice()->CreateComputeShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &m_pComputeShader );
	if(SUCCEEDED(hr))
	{
		m_bCompiled = true;
		if(bSaveByteCode)
		{
			DAnsiStr strBinFile;
			strBinFile.copyFromW(pSrcFile);
			strBinFile =  PS::FILESTRINGUTILS::ChangeFileExt(strBinFile, DAnsiStr(".bin"));
			ofstream ofs(strBinFile.ptr(), ios::out | ios::trunc | ios::binary);
			if(!ofs.is_open())
				return false;

			ofs.write( reinterpret_cast<const char*>(pBlob->GetBufferPointer()), pBlob->GetBufferSize());
			ofs.close();
		}
	}


#if defined(DEBUG) || defined(PROFILE)
	if ( m_pComputeShader )
		(m_pComputeShader)->SetPrivateData( WKPDID_D3DDebugObjectName, lstrlenA(pFuncName), pFuncName);
#endif

    SAFE_RELEASE( pErrorBlob );
    SAFE_RELEASE( pBlob );

    return hr;
}
//================================================================================================
bool DXShader11::tryLoadBinThenCompile(LPCWSTR pSrcFile, 
									   LPCSTR pFuncName,
									   const D3D_SHADER_MACRO * pDefines)
{
	//First try loading from bin
	DAnsiStr strBinFile;
	strBinFile.copyFromW(pSrcFile);
	strBinFile =  PS::FILESTRINGUTILS::ChangeFileExt(strBinFile, DAnsiStr(".bin"));

	if(loadFromBinary(strBinFile.ptr())	)
		return true;

	//Could not find bin so try compiling the HLSL code and save bin afterwards.
	HRESULT res = compile(pSrcFile, pFuncName, pDefines, true);	
	return SUCCEEDED(res);
}

//================================================================================================
HRESULT DXShader11::createStructuredBuffer(UINT uElementSize, UINT uCount, VOID* pInitData, ID3D11Buffer** ppBufOut)
{
    *ppBufOut = NULL;

    D3D11_BUFFER_DESC desc;
    ZeroMemory( &desc, sizeof(desc) );
    desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    desc.ByteWidth = uElementSize * uCount;
    desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    desc.StructureByteStride = uElementSize;

    if ( pInitData )
    {
        D3D11_SUBRESOURCE_DATA InitData;
        InitData.pSysMem = pInitData;
		
		return m_pDXDevice->getDevice()->CreateBuffer( &desc, &InitData, ppBufOut );
    } else
        return m_pDXDevice->getDevice()->CreateBuffer( &desc, NULL, ppBufOut );
}

//--------------------------------------------------------------------------------------
// Create Raw Buffer
//--------------------------------------------------------------------------------------
HRESULT DXShader11::createRawBuffer(UINT uSize, VOID* pInitData, ID3D11Buffer** ppBufOut )
{
    *ppBufOut = NULL;

    D3D11_BUFFER_DESC desc;
    ZeroMemory( &desc, sizeof(desc) );
    desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_INDEX_BUFFER | D3D11_BIND_VERTEX_BUFFER;
    desc.ByteWidth = uSize;
    desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;

    if ( pInitData )
    {
        D3D11_SUBRESOURCE_DATA InitData;
        InitData.pSysMem = pInitData;
        return m_pDXDevice->getDevice()->CreateBuffer( &desc, &InitData, ppBufOut );
    } else
        return m_pDXDevice->getDevice()->CreateBuffer( &desc, NULL, ppBufOut );
}

//--------------------------------------------------------------------------------------
// Create Shader Resource View for Structured or Raw Buffers
//--------------------------------------------------------------------------------------
HRESULT DXShader11::createBufferSRV( ID3D11Buffer* pBuffer, ID3D11ShaderResourceView** ppSRVOut )
{
    D3D11_BUFFER_DESC descBuf;
    ZeroMemory( &descBuf, sizeof(descBuf) );
    pBuffer->GetDesc( &descBuf );

    D3D11_SHADER_RESOURCE_VIEW_DESC desc;
    ZeroMemory( &desc, sizeof(desc) );
    desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
    desc.BufferEx.FirstElement = 0;

    if ( descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS )
    {
        // This is a Raw Buffer

        desc.Format = DXGI_FORMAT_R32_TYPELESS;
        desc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
        desc.BufferEx.NumElements = descBuf.ByteWidth / 4;
    } 
	else if ( descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED )
    {
        // This is a Structured Buffer

        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.BufferEx.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;
    } 
	else
    {
        return E_INVALIDARG;
    }

    return m_pDXDevice->getDevice()->CreateShaderResourceView( pBuffer, &desc, ppSRVOut );
}

//--------------------------------------------------------------------------------------
// Create Unordered Access View for Structured or Raw Buffers
//-------------------------------------------------------------------------------------- 
HRESULT DXShader11::createBufferUAV( ID3D11Buffer* pBuffer, ID3D11UnorderedAccessView** ppUAVOut )
{
    D3D11_BUFFER_DESC descBuf;
    ZeroMemory( &descBuf, sizeof(descBuf) );
    pBuffer->GetDesc( &descBuf );
        
    D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
    ZeroMemory( &desc, sizeof(desc) );
    desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    desc.Buffer.FirstElement = 0;

    if ( descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS )
    {
        // This is a Raw Buffer

        desc.Format = DXGI_FORMAT_R32_TYPELESS; // Format must be DXGI_FORMAT_R32_TYPELESS, when creating Raw Unordered Access View
        desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
        desc.Buffer.NumElements = descBuf.ByteWidth / 4; 
    } else
    if ( descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED )
    {
        // This is a Structured Buffer

        desc.Format = DXGI_FORMAT_UNKNOWN;      // Format must be must be DXGI_FORMAT_UNKNOWN, when creating a View of a Structured Buffer
        desc.Buffer.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride; 
    } else
    {
        return E_INVALIDARG;
    }
    
    return m_pDXDevice->getDevice()->CreateUnorderedAccessView( pBuffer, &desc, ppUAVOut );
}

//Redirect some useful functions to D3D11Device
HRESULT DXShader11::createBuffer(const D3D11_BUFFER_DESC *pDesc, const D3D11_SUBRESOURCE_DATA *pInitialData, ID3D11Buffer** ppBuffer)
{
	return m_pDXDevice->getDevice()->CreateBuffer(pDesc, pInitialData, ppBuffer);
}

HRESULT DXShader11::map(ID3D11Resource *pResource, UINT Subresource, D3D11_MAP MapType, UINT MapFlags, D3D11_MAPPED_SUBRESOURCE *pMappedResource)
{
	return m_pDXDevice->getContext()->Map(pResource, Subresource, MapType, MapFlags, pMappedResource);
}

void	DXShader11::unMap(ID3D11Resource *pResource, UINT SubResource)
{
	return m_pDXDevice->getContext()->Unmap(pResource, SubResource);
}

//Run compute shader
void DXShader11::run( UINT nNumViews, ID3D11ShaderResourceView** pShaderResourceViews, 
					  ID3D11Buffer* pCBCS, void* pCSData, DWORD dwNumDataBytes,
					  ID3D11UnorderedAccessView* pUnorderedAccessView,
					  UINT X, UINT Y, UINT Z )
{
	ID3D11UnorderedAccessView* uavs[1] = {pUnorderedAccessView};

	run(nNumViews, pShaderResourceViews, pCBCS, pCSData, dwNumDataBytes, 1, uavs, X, Y, Z);
}

//--------------------------------------------------------------------------------------
// Run Shader
//--------------------------------------------------------------------------------------
void DXShader11::run( UINT nNumViews, ID3D11ShaderResourceView** pShaderResourceViews, 
					  ID3D11Buffer* pCBCS, void* pCSData, DWORD dwNumDataBytes,
					  UINT nNumUAVs, ID3D11UnorderedAccessView** pUnorderedAccessViews,
					  UINT X, UINT Y, UINT Z )
{
	ID3D11DeviceContext* pd3dImmediateContext = m_pDXDevice->getContext();
	pd3dImmediateContext->CSSetShader( m_pComputeShader, NULL, 0 );
    pd3dImmediateContext->CSSetShaderResources( 0, nNumViews, pShaderResourceViews );
	pd3dImmediateContext->CSSetUnorderedAccessViews( 0, nNumUAVs, pUnorderedAccessViews, NULL );

	//Constant Buffers
    if ( pCBCS )
    {
        D3D11_MAPPED_SUBRESOURCE MappedResource;
        pd3dImmediateContext->Map( pCBCS, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource );
        memcpy( MappedResource.pData, pCSData, dwNumDataBytes );
        pd3dImmediateContext->Unmap( pCBCS, 0 );
        ID3D11Buffer* ppCB[1] = { pCBCS };
        pd3dImmediateContext->CSSetConstantBuffers( 0, 1, ppCB );
    }

    pd3dImmediateContext->Dispatch( X, Y, Z );

    pd3dImmediateContext->CSSetShader( NULL, NULL, 0 );

    ID3D11UnorderedAccessView* ppUAViewNULL[1] = { NULL };
    pd3dImmediateContext->CSSetUnorderedAccessViews( 0, 1, ppUAViewNULL, NULL );

    ID3D11ShaderResourceView* ppSRVNULL[2] = { NULL, NULL };
    pd3dImmediateContext->CSSetShaderResources( 0, 2, ppSRVNULL );

    ID3D11Buffer* ppCBNULL[1] = { NULL };
    pd3dImmediateContext->CSSetConstantBuffers( 0, 1, ppCBNULL );
}

//--------------------------------------------------------------------------------------
// create CPU buffer and copy a GPU buf to CPU buffer. Useful for Debugging
//--------------------------------------------------------------------------------------
ID3D11Buffer* DXShader11::createAndCopyToDebugBuffer( ID3D11Buffer* pGPUBuffer )
{
    ID3D11Buffer* debugbuf = NULL;

    D3D11_BUFFER_DESC desc;
    ZeroMemory( &desc, sizeof(desc) );
    pGPUBuffer->GetDesc( &desc );
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.MiscFlags = 0;
    if ( SUCCEEDED(m_pDXDevice->getDevice()->CreateBuffer(&desc, NULL, &debugbuf)) )
    {
#if defined(DEBUG) || defined(PROFILE)
        debugbuf->SetPrivateData( WKPDID_D3DDebugObjectName, sizeof( "Debug" ) - 1, "Debug" );
#endif

		m_pDXDevice->getContext()->CopyResource( debugbuf, pGPUBuffer );
    }

    return debugbuf;
}



}

#endif
