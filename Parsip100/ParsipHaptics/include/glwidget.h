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
#include "CPolyParsipOptimized.h"
#include "CBlobTreeNetwork.h"
#include "DlgFieldFunctionEditor.h"
#include "AppSettings.h"
#include "CUIWidgets.h"

using namespace PS;
using namespace PS::MATH;
using namespace PS::BLOBTREE;
//using namespace PS::SIMDPOLY;


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

    void selectBlobNode(int iLayer, CBlobNode* aNode);
    void selectBlobNode(QModelIndex idx);

    void actFileModelPiza();

    void actFileModelMedusa();
    void actFileOpen(QString strFile);
    void actFileOpen();
    void actFileExportMesh();
    void actFileSave();
    void actFileClose();

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
    void actAddGradientBlend();
    void actAddCacheNode();
    void actAddWarpTwist();
    void actAddWarpTaper();
    void actAddWarpBend();
    void actAddPCM();


    //Animation
    void actAnimSetStartLoc();
    void actAnimSetEndLoc();
    void actAnimSetStartScale();
    void actAnimSetEndScale();

    void actAnimDrawGuide();
    void actAnimStart();
    void actAnimStop();
    void actAnimRemoveAll();

    //Viewing
    void actViewCamLeftKey(bool bEnable);
    void actViewZoomIn();
    void actViewZoomOut();
    void actViewSetZoom(int value);

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
    void actEditProbe(bool bEnable);
    void actEditBlobTreeStats();
    void actEditAssignIDs();

    void actTestStart();
    void actTestSetRuns(int value);

    void actTestPerformUtilizationTest();
    void actTestPerformIncreasingThreads();
    void actTestPerformStd();


    //SKETCHNET Interactions
    bool actNetSendAck(SKETCHACK ack, int idxMember, int msgID);
    bool actNetSendCommand(SKETCHCMD command, CBlobNode* node, vec4f param);
    bool actNetRecvCommand(int idxMember, QString strMsg);

    //
    void setMouseDragScale(int k);
    void userInterfaceReady();
    void updateProbe();

    //Animation functions
    void advanceAnimation();

    void dataChanged_tblLayers(const QModelIndex& topLeft, const QModelIndex& bottomRight);
    void dataChanged_treeBlob(const QModelIndex& topLeft, const QModelIndex& bottomRight);
    void dataChanged_tblBlobProperty(const QModelIndex& topLeft, const QModelIndex& bottomRight);
    void select_tblColorRibbon(const QModelIndex& index);
    void edit_tblColorRibbon();

signals:
    void sig_viewZoomChanged(int);
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
    enum UIMODE {uimSelect, uimSketch, uimTransform, uimAnimation};

    QTimer*	m_timer;

    PS::CArcBallCamera m_camera;
    PS::CArcBallCamera::MOUSEBUTTONSTATE m_mouseButton;
    vec2i	m_mouseLastPos;
    vec2i	m_scrDim;
    CLayerManager	m_layerManager;

    int                 m_uniformTime;
    CMaterial		m_materials[8];
    int			m_idxRibbonSelection;

    //Sketching variables
    CUIWidget*          m_lpUIWidget;
    UIMODE		m_uiMode;
    vec3f		m_globalPan;
    bool                m_bEnableCamLeftKey;
    bool		m_bEnablePan;
    bool		m_bEnableMultiSelect;
    bool		m_bTransformSkeleton;

    //Variables for field Probing
    vec3f               m_probePoint;
    vec3f               m_probeProjected;

    float               m_probeValue;
    bool                m_bProbing;

    float		m_animTime;
    float		m_animSpeed;
    BlobNodeType	m_sketchType;
    //Reference to object. created using makeObject()
    GLuint			  m_glChessBoard;
    GLuint			  m_glTopCornerCube;
    vector<vec3f>		  m_lstSketchControlPoints;

    QStandardItemModel*	m_modelBlobNodeProperty;
    QStandardItemModel*	m_modelBlobTree;
    QStandardItemModel* m_modelLayerManager;
    QStandardItemModel* m_modelStats;
    QStandardItemModel* m_modelColorRibbon;

    QGLShaderProgram* m_glShaderNormalMesh;
    QGLShaderProgram* m_glShaderSelection;

    GLuint m_glShaderSelTime;

    //Undo/Redo Functionality
    vector<CSketchConfig*> m_lstUndo;
    int m_curUndoLevel;
    int m_ctUndoLevels;

    bool queryScreenToRay(int x, int y, CRay& outRay);

    void getFrustumPlane(float z, vec3f& bottomleft, vec3f& topRight);
    void addUndoLevel();
    void resetUndoLevels();

    void resetTransformState();
    vec3f mask(vec3f v, UITRANSFORMAXIS axis);
    void maskMaterial(UITRANSFORMAXIS axis);

    CBlobNode* addBlobPrimitive(BlobNodeType primitiveType, const vec3f& pos, int preferredID = -1, bool bSendToNet = true);
    CBlobNode* addBlobOperator(BlobNodeType operatorType, int preferredID = -1, bool bSendToNet = true);

    //Display and Management of Scene Data-Structure
    DlgFieldFunctionEditor* m_dlgFieldEditor;

    QStandardItemModel* getModelPrimitiveProperty(CBlobNode* lpNode);
    QStandardItemModel* getModelBlobTree(int iLayer);
    QStandardItemModel* getModelLayerManager();
    QStandardItemModel* getModelStats();
    QStandardItemModel* getModelColorRibbon();

    bool windowToObject(vec3f window, vec3f& object);

    //Draw everything
    void drawLayerManager();
    void drawMapTranslate(vec3f pos);
    void drawMapRotate(vec3f pos);
    void drawMapScale(vec3f pos);
    void drawOctree(vec3f lo, vec3f hi, vec4f color, bool bSelected = false);

    void drawPolygon(const vector<vec3f>& lstPoints);
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
    void   drawChessBoard();
    GLuint drawTopCornerCube();


    void switchToOrtho();
    void switchToProjection();
};


#endif
