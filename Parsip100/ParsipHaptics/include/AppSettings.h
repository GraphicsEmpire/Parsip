#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include "loki/Singleton.h"
#include "_PolygonizerStructs.h"
#include "PS_Material.h"

using namespace Loki;

enum SHOWMESH {smNone = 0, smWireFrame = 1, smSurface = 2};

class AppSettings{

public:
	enum SettingsType {stParsip, stSketch, stDisplay};

	AppSettings() 
	{ 
		memset(&setParsip, 0, sizeof(setParsip));
		memset(&setDisplay, 0, sizeof(setDisplay));
		memset(&setSketch, 0, sizeof(setSketch));

		if(!loadSettings())
			setDefault();
	}

	~AppSettings()
	{
		saveSettings();
	}

	void copySettings(SettingsType t, void* lpSettings, U32 szSettings)
	{
		if(lpSettings == NULL)
			return;
		if(t == stDisplay && szSettings == sizeof(SettingsDisplay))
			memcpy(lpSettings, &setDisplay, szSettings);
		else if(t == stParsip && szSettings == sizeof(SettingsParsip))
			memcpy(lpSettings, &setParsip, szSettings);
		else if(t == stSketch && szSettings == sizeof(SettingsSketch))
			memcpy(lpSettings, &setSketch, szSettings);
	}

	void setDefault();
	bool loadSettings();
	bool saveSettings();
	struct SettingsParsip
	{
		int	ctThreads;
		int	griddim;
		int	testRuns;
		float adaptiveParam;
		float cellSize;

		CellShape	cellShape;
		bool bUseAdaptiveSubDivision;
		bool bUseComputeShaders;
		bool bUseTBB;
		bool bForceMC;		
	};

	struct SettingsDisplay
	{
		int  showMesh;
		bool bDarkBackground;
		bool bShowSeedPoints;
		bool bShowNormals;
		bool bDrawChessboardGround;
		bool bShowBoxLayer;
		bool bShowBoxPrimitive;
		bool bShowBoxPoly;
		bool bShowAnimCurves;

		bool bShowGraph;
		bool bShowColorCodedMPUs;
		int  normalLength;
		CMaterial mtrlMeshWires;		
	};

	struct SettingsSketch
	{
		int mouseDragScale;
	};

	SettingsParsip setParsip;
	SettingsDisplay setDisplay;
	SettingsSketch setSketch;
};

typedef SingletonHolder<AppSettings, CreateUsingNew, PhoenixSingleton> TheAppSettings;

#endif
