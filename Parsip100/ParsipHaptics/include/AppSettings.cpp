#include "AppSettings.h"
#include "PS_FileDirectory.h"
#include "PS_AppConfig.h"
#include "tbb/task_scheduler_init.h"

using namespace PS::FILESTRINGUTILS;

bool AppSettings::loadSettings()
{
	DAnsiStr strFP = CreateNewFileAtRoot(".ini");
	if(!PS::FILESTRINGUTILS::FileExists(strFP))
		return false;

	CAppConfig* cfg = new CAppConfig(strFP, CAppConfig::fmRead);
	//m_setParsip.ctThreads = cfg->readInt("parsip", "threadscount", m_setParsip.ctThreads);
	setParsip.griddim		  = cfg->readInt("parsip", "griddimension", setParsip.griddim);
	setParsip.adaptiveParam = cfg->readFloat("parsip", "adaptiveparam", setParsip.adaptiveParam);
	setParsip.cellSize	  = cfg->readFloat("parsip", "cellsize", setParsip.cellSize);
	setParsip.cellShape     = static_cast<CellShape>(cfg->readInt("parsip", "cellshape", static_cast<int>(setParsip.cellShape)));
	setParsip.bUseAdaptiveSubDivision = cfg->readBool("parsip", "useadaptivesubdivision", setParsip.bUseAdaptiveSubDivision);

	setParsip.bUseTBB = cfg->readBool("parsip", "usetbb", setParsip.bUseTBB);
	setParsip.bUseComputeShaders = cfg->readBool("parsip", "usecomputeshaders", setParsip.bUseComputeShaders);
	setParsip.bForceMC     = cfg->readBool("parsip", "forcemarchingcubes", setParsip.bForceMC);
	//m_setParsip.strLastScene = cfg->readString("parsip", "lastscene", m_setParsip.strLastScene);

	setDisplay.bDarkBackground	= cfg->readBool("display", "darkbackground", setDisplay.bDarkBackground);
	setDisplay.showMesh			= cfg->readInt("display", "drawmesh", setDisplay.showMesh);
	setDisplay.bShowBoxLayer		= cfg->readBool("display", "drawboxlayer", setDisplay.bShowBoxLayer);
	setDisplay.bShowBoxPrimitive	= cfg->readBool("display", "drawboxprimitive", setDisplay.bShowBoxPrimitive);
	setDisplay.bShowBoxPoly		= cfg->readBool("display", "drawboxpoly", setDisplay.bShowBoxPoly);
	setDisplay.bShowAnimCurves    = cfg->readBool("display", "drawanimcurves", setDisplay.bShowAnimCurves);



	setDisplay.bDrawChessboardGround = cfg->readBool("display", "drawchessboard", setDisplay.bDrawChessboardGround);
	setDisplay.bShowSeedPoints  = cfg->readBool("display", "drawseedpoints", setDisplay.bShowSeedPoints);
	setDisplay.bShowNormals     = cfg->readBool("display", "drawnormals", setDisplay.bShowNormals);
	setDisplay.normalLength	  = cfg->readInt("display", "normallength", setDisplay.normalLength);

	setDisplay.bShowColorCodedMPUs = cfg->readBool("display", "colorcodedmpu", setDisplay.bShowColorCodedMPUs);
	setDisplay.bShowGraph			 = cfg->readBool("display", "graph", setDisplay.bShowGraph);
	setSketch.mouseDragScale	 = cfg->readInt("sketch", "mousedragscale", setSketch.mouseDragScale);

	SAFE_DELETE(cfg);
	return true;
}

bool AppSettings::saveSettings()
{
	DAnsiStr strFP = PS::FILESTRINGUTILS::CreateNewFileAtRoot(".ini");
	CAppConfig* cfg = new CAppConfig(strFP, CAppConfig::fmWrite);
	cfg->writeInt("parsip", "griddimension", setParsip.griddim);
	cfg->writeInt("parsip", "threadscount", setParsip.ctThreads);
	cfg->writeFloat("parsip", "adaptiveparam", setParsip.adaptiveParam);
	cfg->writeFloat("parsip", "cellsize", setParsip.cellSize);
	cfg->writeInt("parsip", "cellshape", static_cast<int>(setParsip.cellShape));
	cfg->writeBool("parsip", "usetbb", setParsip.bUseTBB);
	cfg->writeBool("parsip", "usecomputeshaders", setParsip.bUseComputeShaders);
	cfg->writeBool("parsip", "useadaptivesubdivision", setParsip.bUseAdaptiveSubDivision);
	cfg->writeBool("parsip", "forcemarchingcubes", setParsip.bForceMC);
	//cfg->writeString("parsip", "lastscene", setParsip.strLastScene);


	cfg->writeBool("display", "darkbackground", setDisplay.bDarkBackground);
	cfg->writeInt("display", "drawmesh", setDisplay.showMesh);
	cfg->writeBool("display", "drawboxlayer", setDisplay.bShowBoxLayer);
	cfg->writeBool("display", "drawboxprimitive", setDisplay.bShowBoxPrimitive);
	cfg->writeBool("display", "drawboxpoly", setDisplay.bShowBoxPoly);


	cfg->writeBool("display", "drawchessboard", setDisplay.bDrawChessboardGround);
	cfg->writeBool("display", "drawseedpoints", setDisplay.bShowSeedPoints);
	cfg->writeBool("display", "drawnormals", setDisplay.bShowNormals);
	cfg->writeInt("display", "normallength", setDisplay.normalLength);
	cfg->writeBool("display", "colorcodedmpu", setDisplay.bShowColorCodedMPUs);
	cfg->writeBool("display", "drawanimcurves", setDisplay.bShowAnimCurves);
	cfg->writeBool("display", "graph", setDisplay.bShowGraph);

	cfg->writeInt("sketch", "mousedragscale", setSketch.mouseDragScale);

	SAFE_DELETE(cfg);
	return true;
}

void AppSettings::setDefault()
{
	setParsip.bUseComputeShaders = false;
	setParsip.bUseTBB	  = true;
	setParsip.cellShape = csCube;
	setParsip.bUseAdaptiveSubDivision = true;
	setParsip.adaptiveParam = DEFAULT_EDGE_NORMALS_ANGLE;
	setParsip.cellSize  = DEFAULT_CELL_SIZE;
	setParsip.griddim = 8;
	setParsip.ctThreads = tbb::task_scheduler_init::default_num_threads();
	setParsip.testRuns = 1000;



	setDisplay.showMesh		   = smSurface;
	setDisplay.bDarkBackground  = true;
	setDisplay.bShowBoxLayer	   = true;
	setDisplay.bShowBoxPrimitive = true;
	setDisplay.bShowBoxPoly	   = true;

	setDisplay.bDrawChessboardGround = true;
	setDisplay.bShowSeedPoints	   = true;
	setDisplay.bShowNormals	= true;
	setDisplay.mtrlMeshWires  = CMaterial::mtrlRedPlastic();
	setDisplay.normalLength   = DEFAULT_NORMAL_LEN;

	setSketch.mouseDragScale = DEFAULT_MOUSE_DRAG_SCALE;
}

