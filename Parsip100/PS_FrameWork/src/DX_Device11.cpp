//#include "stdafx.h"
#include "DX_Device11.h"

#ifdef WIN32

namespace PS
{

DXDevice11* DXDevice11::m_instance = NULL;

DXDevice11::DXDevice11()
{
	m_pDevice = NULL;
	m_pContext = NULL;
	m_bDeviceCreated = false;
}

DXDevice11::~DXDevice11()
{
	m_instance = NULL;
	this->release();	
}

DXDevice11* DXDevice11::getInstance()
{
	if(!m_instance)
	{
		m_instance = new DXDevice11();
		m_instance->createDevice();
	}
	
	return m_instance;
}

void DXDevice11::freeInstance()
{
	if(m_instance)
		SAFE_DELETE(m_instance);
}

void DXDevice11::release()
{
	if(m_bDeviceCreated)
	{		
		SAFE_RELEASE( m_pContext );
		SAFE_RELEASE( m_pDevice );
	}
}

HRESULT DXDevice11::createDevice(bool bDoublePrecision	/* = false */,
								 bool bForceRef  /* = false */,
								 UINT uCreationFlags /* = D3D11_CREATE_DEVICE_SINGLETHREADED */,
								 D3D_FEATURE_LEVEL	minFeatureLevel /* = D3D_FEATURE_LEVEL_11_0 */)
{
    HRESULT hr = S_OK;

#if defined(DEBUG) || defined(_DEBUG)
    uCreationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL flOut;
    static const D3D_FEATURE_LEVEL flvl[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };
    
    BOOL bNeedRefDevice = FALSE;
    if ( !bForceRef )
    {
        hr = Dynamic_D3D11CreateDevice( NULL,                        // Use default graphics card
                                        D3D_DRIVER_TYPE_HARDWARE,    // Try to create a hardware accelerated device
                                        NULL,                        // Do not use external software rasterizer module
                                        uCreationFlags,              // Device creation flags
                                        flvl,
                                        sizeof(flvl) / sizeof(D3D_FEATURE_LEVEL),
                                        D3D11_SDK_VERSION,           // SDK version
										&m_pDevice,                 // Device out
                                        &flOut,                      // Actual feature level created
										&m_pContext );              // Context out
        
        if ( SUCCEEDED( hr ) )
        {

            // A hardware accelerated device has been created, so check for Compute Shader support
            // If we have a device >= D3D_FEATURE_LEVEL_11_0 created, full CS5.0 support is guaranteed, no need for further checks
			if ( flOut < minFeatureLevel )            
            {
				if(bDoublePrecision)
				{
					bNeedRefDevice = TRUE;	
					//throw "No hardware Compute Shader 5.0 capable device found, trying to create ref device.\n";					
				}
				else
				{
					// Otherwise, we need further check whether this device support CS4.x (Compute on 10)
					D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS hwopts;
					m_pDevice->CheckFeatureSupport( D3D11_FEATURE_D3D10_X_HARDWARE_OPTIONS, &hwopts, sizeof(hwopts) );
					if ( !hwopts.ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x )
					{
						bNeedRefDevice = TRUE;
						//throw "No hardware Compute Shader capable device found, trying to create ref device.\n";
					}
				}
            }

			if(bDoublePrecision)
			{
                // Double-precision support is an optional feature of CS 5.0
                D3D11_FEATURE_DATA_DOUBLES hwopts;
				m_pDevice->CheckFeatureSupport( D3D11_FEATURE_DOUBLES, &hwopts, sizeof(hwopts) );
                if ( !hwopts.DoublePrecisionFloatShaderOps )
                {
                    bNeedRefDevice = TRUE;
                    //throw( "No hardware double-precision capable device found, trying to create ref device.\n" );
                }

			}
        }
    }
    
    if ( bForceRef || FAILED(hr) || bNeedRefDevice )
    {
        // Either because of failure on creating a hardware device or hardware lacking CS capability, we create a ref device here
        SAFE_RELEASE( m_pDevice);
		SAFE_RELEASE( m_pContext );
        
        hr = Dynamic_D3D11CreateDevice( NULL,                        // Use default graphics card
                                        D3D_DRIVER_TYPE_REFERENCE,   // Try to create a hardware accelerated device
                                        NULL,                        // Do not use external software rasterizer module
                                        uCreationFlags,              // Device creation flags
                                        flvl,
                                        sizeof(flvl) / sizeof(D3D_FEATURE_LEVEL),
                                        D3D11_SDK_VERSION,           // SDK version
										&m_pDevice,                 // Device out
                                        &flOut,                      // Actual feature level created
										&m_pContext );              // Context out
        if ( FAILED(hr) )
        {
            //throw( "Reference rasterizer device create failure\n" );
            return hr;
        }
		else

			m_bDeviceCreated = true;
    }
	else
		m_bDeviceCreated = true;

    return hr;
}

bool DXDevice11::GetDeviceSettings(ID3D11Device* lpDevice, D3D_Device_Settings& settings)
{
	if(lpDevice == NULL)
		return false;

	//Check is the device supports Compute shaders
	D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS hwOptions;
	HRESULT res = lpDevice->CheckFeatureSupport(D3D11_FEATURE_D3D10_X_HARDWARE_OPTIONS, &hwOptions, sizeof(hwOptions));
	if(SUCCEEDED(res))
		settings.bComputeShaders = (hwOptions.ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x == TRUE);

	//Check for Double floating point support
	D3D11_FEATURE_DATA_DOUBLES hwDouble;
	res = lpDevice->CheckFeatureSupport(D3D11_FEATURE_DOUBLES, &hwDouble, sizeof(hwDouble));
	if(SUCCEEDED(res))	
		settings.bDoublePrecision = (hwDouble.DoublePrecisionFloatShaderOps == TRUE);
	else
		settings.bDoublePrecision = false;

	//Get Feature level
	settings.featureLevel = lpDevice->GetFeatureLevel();

	settings.uCreationFlags = lpDevice->GetCreationFlags();

	return true;
}

DXDevice11::OSVERSION DXDevice11::GetOperatingSystemVersion()
{
	OSVERSIONINFOEX osv;
	memset( &osv, 0, sizeof(osv) );
	osv.dwOSVersionInfoSize = sizeof(osv);
	GetVersionEx( (LPOSVERSIONINFO)&osv );

	OSVERSION ospacked;  
	ospacked.major = osv.dwMajorVersion;
	ospacked.minor = osv.dwMinorVersion;
	ospacked.buildNumber = osv.dwBuildNumber;

	return ospacked;
}

//--------------------------------------------------------------------------------------
// This is equivalent to D3D11CreateDevice, except it dynamically loads d3d11.dll,
// this gives us a graceful way to message the user on systems with no d3d11 installed
//--------------------------------------------------------------------------------------
HRESULT WINAPI DXDevice11::Dynamic_D3D11CreateDevice( IDXGIAdapter* pAdapter,
                                          D3D_DRIVER_TYPE DriverType,
                                          HMODULE Software,
                                          UINT32 Flags,
                                          CONST D3D_FEATURE_LEVEL* pFeatureLevels,
                                          UINT FeatureLevels,
                                          UINT32 SDKVersion,
                                          ID3D11Device** ppDevice,
                                          D3D_FEATURE_LEVEL* pFeatureLevel,
                                          ID3D11DeviceContext** ppImmediateContext )
{
    typedef HRESULT (WINAPI * LPD3D11CREATEDEVICE)( IDXGIAdapter*, D3D_DRIVER_TYPE, HMODULE, UINT32, CONST D3D_FEATURE_LEVEL*, UINT, UINT32, ID3D11Device**, D3D_FEATURE_LEVEL*, ID3D11DeviceContext** );
    static LPD3D11CREATEDEVICE  s_DynamicD3D11CreateDevice = NULL;
    
    if ( s_DynamicD3D11CreateDevice == NULL )
    {            
        HMODULE hModD3D11 = LoadLibrary( L"d3d11.dll" );

        if ( hModD3D11 == NULL )
        {
            // Ensure this "D3D11 absent" message is shown only once. As sometimes, the app would like to try
            // to create device multiple times
            static bool bMessageAlreadyShwon = false;
            
            if ( !bMessageAlreadyShwon )
            {
                OSVERSIONINFOEX osv;
                memset( &osv, 0, sizeof(osv) );
                osv.dwOSVersionInfoSize = sizeof(osv);
                GetVersionEx( (LPOSVERSIONINFO)&osv );

                if ( ( osv.dwMajorVersion > 6 )
                    || ( osv.dwMajorVersion == 6 && osv.dwMinorVersion >= 1 ) 
                    || ( osv.dwMajorVersion == 6 && osv.dwMinorVersion == 0 && osv.dwBuildNumber > 6002 ) )
                {

                    MessageBox( 0, L"Direct3D 11 components were not found.", L"Error", MB_ICONEXCLAMATION );
                    // This should not happen, but is here for completeness as the system could be
                    // corrupted or some future OS version could pull D3D11.DLL for some reason
                }
                else if ( osv.dwMajorVersion == 6 && osv.dwMinorVersion == 0 && osv.dwBuildNumber == 6002 )
                {

                    MessageBox( 0, L"Direct3D 11 components were not found, but are available for"\
                        L" this version of Windows.\n"\
                        L"For details see Microsoft Knowledge Base Article #971644\n"\
                        L"http://support.microsoft.com/default.aspx/kb/971644/", L"Error", MB_ICONEXCLAMATION );

                }
                else if ( osv.dwMajorVersion == 6 && osv.dwMinorVersion == 0 )
                {
                    MessageBox( 0, L"Direct3D 11 components were not found. Please install the latest Service Pack.\n"\
                        L"For details see Microsoft Knowledge Base Article #935791\n"\
                        L" http://support.microsoft.com/default.aspx/kb/935791", L"Error", MB_ICONEXCLAMATION );

                }
                else
                {
                    MessageBox( 0, L"Direct3D 11 is not supported on this OS.", L"Error", MB_ICONEXCLAMATION );
                }

                bMessageAlreadyShwon = true;
            }            

            return E_FAIL;
        }

        s_DynamicD3D11CreateDevice = ( LPD3D11CREATEDEVICE )GetProcAddress( hModD3D11, "D3D11CreateDevice" );           
    }

    return s_DynamicD3D11CreateDevice( pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels,
                                       SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext );
}


}

#endif
