//-------------------------------------------------------------------------------------------
//  University of Victoria Computer Science Department
//	FrameWork for OpenGL application under QT
//  Course title: Computer Graphics CSC305
//-------------------------------------------------------------------------------------------
//These two lines are header guiardians against multiple includes
#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QGLWidget>
#include <QGLShaderProgram>
#include <QStandardItemModel>
#include <QTimer>
#include <QModelIndex>

#include "PS_FrameWork/include/PS_ArcBallCamera.h"
#include "PS_FrameWork/include/PS_Vector.h"
#include "PS_FrameWork/include/TaskManager.h"
#include "PS_FrameWork/include/PS_SplineCatmullRom.h"
#include "CLayerManager.h"
//#include "CPolyParsipServer.h"
#include "CPolyParsipOptimized.h"
#include "CBlobTreeNetwork.h"
#include "DlgFieldFunctionEditor.h"

using namespace PS;
using namespace PS::MATH;
using namespace PS::BLOBTREE;

typedef enum SHOWMESH {smNone = 0, smWireFrame = 1, smSurface = 2};

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
	DAnsiStr strLastScene;
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
	//vec3f bgColor;
};

/*
class GLGizmo : public QGLWidget
{

};
*/

//This is our OpenGL Component we built it on top of QGLWidget
class GLWidget : public QGLWidget
{
    Q_OBJECT

public slots:
		void setParsipGridDim(int nComboItem);
		void setParsipCellSize(float value);
		void setParsipNormalsGeodesicAngle(float value);
		void setParsipThreadsCount(int value = tbb::task_scheduler_init::automatic);
		void setParsipUseTBB(bool bEnable);
		void setParsipUseAdaptiveSubDivision(bool bEnable);
		void setParsipUseComputeShaders(bool bEnable);
		void setParsipCellShapeCube(bool bCube);
		void setParsipCellShapeTetraHedra(bool bTetra);
		void setParsipForceMC(bool bEnable);
		
		void setDisplayMeshNone(bool bEnable);
		void setDisplayMeshSurface(bool bEnable);
		void setDisplayMeshWireFrame(bool bEnable);
		
		void setDisplayBoxLayer(bool bEnable);
		void setDisplayBoxPrimitive(bool bEnable);
		void setDisplayBoxPoly(bool bEnable);
		
		void setDisplaySeedPoints(bool bEnable);
		void setDisplayNormals(bool bEnable);
		void setDisplayNormalsLength(int length);
		void setDisplayChessBoard(bool bEnable);

		void setDisplayColorCodedMPUs(bool bEnable);
		void setDisplayGraph(bool bEnable);
		void setDisplayAnimCurves(bool bEnable);
		
		void setDisplayMtrlMeshWires(int index);

		void setAnimationSpeed(int speed);


		void setPrimitiveColorFromColorDlg();

		void selectLayer(int iLayer);
		void selectLayer(QModelIndex idx);

		void selectBlobNode(int iLayer, CBlobTree* aNode);
		void selectBlobNode(QModelIndex idx);
		
		void actOpenMedusa();
		void actOpenProject(QString strFile);
		void actOpenProject();
		void actFileExportMesh();
		void actSaveProject();
		void actCloseProject();

		//Mesh methods
		void actMeshSubDivide();
		void actMeshPolygonize();
		void actMeshPolygonize(int idxLayer);
		void actMeshInsert();

		void actViewResetCamera();
		void actViewEnablePan(bool bEnable);

		//Layers
		void actLayerAdd();
		void actLayerDelete();
		void actLayerDuplicate();
		void actLayerSelectAll();

		//Add Primitives
		void actAddSphere();
		void actAddCylinder();
		void actAddRing();
		void actAddDisc();
		void actAddPolygonPlane();
		void actAddCube();
		void actAddTriangle();

		//Add Operators
		void actAddUnion();
		void actAddIntersection();
		void actAddDifference();
		void actAddSmoothDif();
		void actAddBlend();
		void actAddRicciBlend();
		void actAddCacheNode();
		void actAddWarpTwist();
		void actAddWarpTaper();
		void actAddWarpBend();

		//Animation
		void actAnimSetStartLoc();
		void actAnimSetEndLoc();
		void actAnimSetStartScale();
		void actAnimSetEndScale();

		void actAnimDrawGuide();
		void actAnimStart();
		void actAnimStop();
		void actAnimRemoveAll();

		//Editing Operations
		void actEditSelect();
		void actEditMultiSelect(bool bEnable);
		void actEditTranslate();
		void actEditRotate();
		void actEditScale();
		void actEditAxisX();
		void actEditAxisY();
		void actEditAxisZ();
		void actEditAxisFree();
		void actEditDelete();
		void actEditTransformSkeleton(bool bEnable);
		void actEditUndo();
		void actEditRedo();
		void actEditCopy();
		void actEditPaste();
		void actEditFieldEditor();
		void actEditConvertToBinaryTree();


		void actTestStart();
		void actTestSetRuns(int value);

		void actTestPerformUtilizationTest();
		void actTestPerformIncreasingThreads();
		void actTestPerformStd();


		//SKETCHNET Interactions
		bool actNetSendAck(SKETCHACK ack, int idxMember, int msgID);
		bool actNetSendCommand(SKETCHCMD command, CBlobTree* node, vec4f param);
		bool actNetRecvCommand(int idxMember, QString strMsg);

		//
		void setMouseDragScale(int k);
		void userInterfaceReady();

		//Animation functions
		void advanceAnimation();

		void dataChanged_tblLayers(const QModelIndex& topLeft, const QModelIndex& bottomRight);
		void dataChanged_treeBlob(const QModelIndex& topLeft, const QModelIndex& bottomRight);
		void dataChanged_tblBlobProperty(const QModelIndex& topLeft, const QModelIndex& bottomRight);
		void select_tblColorRibbon(const QModelIndex& index);
		void edit_tblColorRibbon();

signals:
		void sig_addNetLog(const QString& strMsg);
		void sig_setProgress(int min, int max, int val);
		void sig_setParsipThreadsCount(int value);
		void sig_setParsipCellSize(int sideScale);
		void sig_readApplySettings();
		void sig_showBlobTree(QStandardItemModel * model);
		void sig_showPrimitiveProperty(QStandardItemModel * model);
		void sig_showLayerManager(QStandardItemModel * model);
		void sig_showLayerManager(const QStringList& labels);

		void sig_showStats(QStandardItemModel * model);
		void sig_showColorRibbon(QStandardItemModel * model);		
		
		//Enable Undo/Redo
		void sig_enableUndo(bool bEnable);
		void sig_enableRedo(bool bEnable);
		
		void sig_enableHasLayer(bool bEnable);
		void sig_setPrimitiveColor(QColor color);
		void sig_setTimeFPS(double time, int FPS);


public:
    //Constructor for GLWidget
    GLWidget(QWidget *parent = 0);

    //Destructor for GLWidget
    ~GLWidget();

protected:
    //Initialize the OpenGL Graphics Engine
    void initializeGL();

    //All our painting stuff are here
    void paintGL();

    //When user resizes main window, the scrollArea will be resized and it will call this function from
    //its attached GLWidget
    void resizeGL(int width, int height);

	//Handle Double click event
	void mouseDoubleClickEvent(QMouseEvent * event);

    //Handle mouse press event in scrollArea
    void mousePressEvent(QMouseEvent *event);

	//Handle mouse release event in scrollArea
	void mouseReleaseEvent(QMouseEvent *event);

	//Handle mouse move event in scrollArea
	void mouseMoveEvent(QMouseEvent *event);

	//Handle keyboard for blender-style shortcut keys
	void keyPressEvent(QKeyEvent *event);

	void wheelEvent(QWheelEvent *event);

private:
	typedef enum UIMODE {uimSelect, uimSketch, uimTransform, uimAnimation};
	typedef enum UITRANSFORMTYPE {uitTranslate, uitRotate, uitScale};
	typedef enum UITRANSFORMAXIS {uiaX, uiaY, uiaZ, uiaFree};

	struct UITRANSFORM{
		UITRANSFORMTYPE type;
		UITRANSFORMAXIS axis;
		vec3f		translate;
		vec3f		scale;
		CQuaternion rotate;
		size_t		nStep;
		vec3f mouseDown;
		vec3f mouseMove;
	};
	
	QTimer*			m_timer;	
	
	PS::CArcBallCamera m_camera;
	PS::CArcBallCamera::MOUSEBUTTONSTATE m_mouseButton;	
	int		m_mouseDragScale;
	vec2i	m_mouseLastPos;
	vec2i	m_scrDim;
	SettingsDisplay m_setDisplay;
	SettingsParsip	m_setParsip;
	CLayerManager	m_layerManager;
	//CParsipServer	m_parsip;
	//CParsipOptimized m_optParsip;
		
	int				m_uniformTime;
	CMaterial		m_materials[8];
	int				m_idxRibbonSelection;
	
	//Sketching variables
	UIMODE			m_uiMode;			
	UITRANSFORM		m_uiTransform;
	vec3f			m_globalPan;
	bool			m_bEnablePan;
	bool			m_bEnableMultiSelect;	
	bool			m_bTransformSkeleton;


	float			m_animTime;
	float			m_animSpeed;
	SkeletonType	  m_sketchSkeletType;
	//Reference to object. created using makeObject()
	GLuint			  m_glChessBoard;
	GLuint			  m_glTopCornerCube;
	DVec<vec3>		  m_lstSketchControlPoints;
	CBlobTree*		  m_lpSelectedBlobNode;
	
	QStandardItemModel*	m_modelBlobNodeProperty;
	QStandardItemModel*	m_modelBlobTree;
	QStandardItemModel* m_modelLayerManager;
	QStandardItemModel* m_modelStats;
	QStandardItemModel* m_modelColorRibbon;

	QGLShaderProgram* m_glShaderNormalMesh;
	QGLShaderProgram* m_glShaderSelection;

	GLuint m_glShaderSelTime;

	//Undo/Redo Functionality
	DVec<CSketchConfig*> m_lstUndo;
	int m_curUndoLevel;
	int m_ctUndoLevels;

	bool queryScreenToRay(int x, int y, CRay& outRay);

	void getFrustumPlane(float z, vec3f& bottomleft, vec3f& topRight);
	void addUndoLevel();
	void resetUndoLevels();

	void resetTransformState();
	vec3f mask(vec3f v, UITRANSFORMAXIS axis);
	void maskMaterial(UITRANSFORMAXIS axis);

	CBlobTree* addBlobPrimitive(SkeletonType skeletType, const vec3f& pos, int preferredID = -1, bool bSendToNet = true);
	CBlobTree* addBlobOperator(BlobNodeType operatorType, int preferredID = -1, bool bSendToNet = true);

	//Display and Management of Scene Data-Structure
	DlgFieldFunctionEditor* m_dlgFieldEditor;

	QStandardItemModel* getModelPrimitiveProperty(CBlobTree* lpNode);
	QStandardItemModel* getModelBlobTree(int iLayer);
	QStandardItemModel* getModelLayerManager();
	QStandardItemModel* getModelStats();
	QStandardItemModel* getModelColorRibbon();

	bool windowToObject(vec3f window, vec3f& object);

	//Serialize App Settings
	void saveSettings();
	void loadSetting();

	//Draw everything
	void drawLayerManager();
	void drawMapTranslate(vec3f pos);
	void drawMapRotate(vec3f pos);
	void drawMapScale(vec3f pos);
	void drawOctree(vec3 lo, vec3 hi, vec4f color, bool bSelected = false);	
	void drawPolygon(const DVec<vec3>& lstPoints);
	void drawRay(const CRay& ray, vec4f color);
	void drawLineSegment(vec3f s, vec3f e, vec4f color);
	void drawString(const char* str, float x, float y);
	void drawGraph();

    //Makes each face of the polygon
    void drawCubePolygonTextured(int a, int b, int c, int d);
	void drawCubePolygon(int a, int b, int c, int d);

    //It is much more efficient if you create all the objects in a scene
    //in one place and group them in a list. Then this will be plugged into graphics hardware at runtime.
    GLuint drawDefaultColoredCube();
	void	drawChessBoard();
	GLuint drawTopCornerCube();

	void benchmarkSIMD();

	void switchToOrtho();
	void switchToProjection();
};


#endif
