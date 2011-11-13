#pragma once
//A Wrapper for DX11 device creation and management

#ifndef DX_DEVICE11_H
#define DX_DEVICE11_H

#ifdef WIN32DXSDK

#include <d3d11.h>
#include <d3dx11.h>


#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) { if(p) { delete (p); (p)=NULL; } }
#endif

namespace PS
{
//Creates a DirectX11 device
//Manage device creation and release
class DXDevice11
{
protected:
	static DXDevice11* m_instance;


private:
	bool					m_bDeviceCreated; //true if devide is created and initialized

	//late binding to D3D11CreateDevice function in d3d11.dll for graceful user feedback
	HRESULT WINAPI Dynamic_D3D11CreateDevice( IDXGIAdapter* pAdapter,
                                          D3D_DRIVER_TYPE DriverType,
                                          HMODULE Software,
                                          UINT32 Flags,
                                          CONST D3D_FEATURE_LEVEL* pFeatureLevels,
                                          UINT FeatureLevels,
                                          UINT32 SDKVersion,
                                          ID3D11Device** ppDevice,
                                          D3D_FEATURE_LEVEL* pFeatureLevel,
                                          ID3D11DeviceContext** ppImmediateContext );

public: 
	struct OSVERSION
	{
		DWORD major;
		DWORD minor;
		DWORD buildNumber;
	};

	//Defines the requested or created device 
	//settings
	struct D3D_Device_Settings
	{
		bool bComputeShaders;
		bool bDoublePrecision;
		bool bReference;
		UINT uCreationFlags;
		D3D_FEATURE_LEVEL featureLevel;
	};

	//Pointer to DX11 device
	ID3D11Device*			m_pDevice;

	//Pointer to DX11 device running context
	ID3D11DeviceContext*	m_pContext;

	static DXDevice11* getInstance();
	static void freeInstance();

	DXDevice11();
	~DXDevice11();

	void release();
public:

	//Parameters specify the capabilities needed for this App
	HRESULT createDevice(bool bDoublePrecision	= false,
						 bool bForceRef			= false,
						 UINT uCreationFlags	= D3D11_CREATE_DEVICE_SINGLETHREADED,
						 D3D_FEATURE_LEVEL	minFeatureLevel = D3D_FEATURE_LEVEL_11_0);



	static bool GetDeviceSettings(ID3D11Device* lpDevice, D3D_Device_Settings& settings);
	static OSVERSION GetOperatingSystemVersion(); //Get OS Version info



	ID3D11DeviceContext* getContext() const {return m_pContext;}
	ID3D11Device* getDevice() const {return m_pDevice;}
	bool isDeviceCreated() const {return m_bDeviceCreated; } //True if device is created
};

}

#endif

#endif
