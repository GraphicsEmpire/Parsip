#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "glwidget.h"
#include "_GlobalSettings.h"
#include "PS_FrameWork/include/PS_AppConfig.h"
#include "PS_FrameWork/include/PS_FileDirectory.h"
#include "PS_FrameWork/include/PS_HWUtils.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //Create an instance of glWidget
    m_glWidget = new GLWidget;

    //Design Network
    m_pDesignNet = CDesignNet::GetDesignNet();
    connect(m_pDesignNet, SIGNAL(sig_memberslist(QStringList)), this, SLOT(actNetShowMembers(QStringList)));
    connect(m_pDesignNet, SIGNAL(sig_newMessage(int,QString)), this, SLOT(actNetNewMessage(int,QString)));
    connect(m_pDesignNet, SIGNAL(sig_newMessage(int,QString)), m_glWidget, SLOT(actNetRecvCommand(int,QString)));

    //Fill scroll area with GLWidget
    ui->scrollAreaOpenGL->setWidget(m_glWidget);


    //Fill Material
    std::vector<string> lstMtrlNames;
    CMaterial::getMaterialNames(lstMtrlNames);
    for (size_t i=0; i < lstMtrlNames.size(); i++)
        ui->cboMaterial->addItem(QString(lstMtrlNames[i].c_str()));
    ui->cboMaterial->setCurrentIndex(lstMtrlNames.size() - 1);

    connect(ui->sliderNormalsAngle, SIGNAL(valueChanged(int)), this, SLOT(setParsipNormalsAngle(int)));
    connect(ui->udNormalsAngle, SIGNAL(valueChanged(double)), this, SLOT(setParsipNormalsAngle(double)));

    //connect(this, SLOT(keyPressEvent(QKeyEvent)), m_glWidget, SLOT(keyPressEvent(QKeyEvent)));
    connect(ui->sliderCubeSize, SIGNAL(valueChanged(int)), this, SLOT(setParsipCellSize(int)));
    connect(ui->udCubeSize, SIGNAL(valueChanged(double)), this, SLOT(setParsipCellSize(double)));

    //Combobox for grid dimensions
    connect(ui->cboGridDim, SIGNAL(currentIndexChanged(int)), m_glWidget, SLOT(setParsipGridDim(int)));
    connect(ui->sliderThreadsCount, SIGNAL(valueChanged(int)), m_glWidget, SLOT(setParsipThreadsCount(int)));
    connect(m_glWidget, SIGNAL(sig_setParsipThreadsCount(int)), ui->sliderThreadsCount, SLOT(setValue(int)));

    connect(ui->rbCube, SIGNAL(toggled(bool)), m_glWidget, SLOT(setParsipCellShapeCube(bool)));
    connect(ui->rbTetraHedra, SIGNAL(toggled(bool)), m_glWidget, SLOT(setParsipCellShapeTetraHedra(bool)));
    connect(ui->chkComputeShaders, SIGNAL(toggled(bool)), m_glWidget, SLOT(setParsipUseComputeShaders(bool)));
    connect(ui->chkTBB, SIGNAL(toggled(bool)), m_glWidget, SLOT(setParsipUseTBB(bool)));
    connect(ui->chkEnableAdaptive, SIGNAL(toggled(bool)), m_glWidget, SLOT(setParsipUseAdaptiveSubDivision(bool)));

    connect(ui->rbShowMeshNone, SIGNAL(toggled(bool)), m_glWidget, SLOT(setDisplayMeshNone(bool)));
    connect(ui->rbShowMeshWireFrame, SIGNAL(toggled(bool)), m_glWidget, SLOT(setDisplayMeshWireFrame(bool)));
    connect(ui->rbShowMeshSurface, SIGNAL(toggled(bool)), m_glWidget, SLOT(setDisplayMeshSurface(bool)));


    connect(ui->chkShowBoxLayer, SIGNAL(toggled(bool)), m_glWidget, SLOT(setDisplayBoxLayer(bool)));
    connect(ui->chkShowBoxPrimitive, SIGNAL(toggled(bool)), m_glWidget, SLOT(setDisplayBoxPrimitive(bool)));
    connect(ui->chkShowBoxPoly, SIGNAL(toggled(bool)), m_glWidget, SLOT(setDisplayBoxPoly(bool)));

    connect(ui->chkShowSeedPoints, SIGNAL(toggled(bool)), m_glWidget, SLOT(setDisplaySeedPoints(bool)));
    connect(ui->chkShowNormals, SIGNAL(toggled(bool)), m_glWidget, SLOT(setDisplayNormals(bool)));

    connect(ui->chkColorCodeMPUMesh, SIGNAL(toggled(bool)), m_glWidget, SLOT(setDisplayColorCodedMPUs(bool)));
    connect(ui->chkShowGraph, SIGNAL(toggled(bool)), m_glWidget, SLOT(setDisplayGraph(bool)));
    connect(ui->chkShowAnimationCurves, SIGNAL(toggled(bool)), m_glWidget, SLOT(setDisplayAnimCurves(bool)));

    connect(ui->chkForceMC, SIGNAL(toggled(bool)), m_glWidget, SLOT(setParsipForceMC(bool)));

    connect(ui->sliderNormalLength, SIGNAL(valueChanged(int)), m_glWidget, SLOT(setDisplayNormalsLength(int)));
    connect(ui->sliderAnimationSpeed, SIGNAL(valueChanged(int)), m_glWidget, SLOT(setAnimationSpeed(int)));
    connect(ui->chkShowChessBoard, SIGNAL(toggled(bool)), m_glWidget, SLOT(setDisplayChessBoard(bool)));
    connect(ui->cboMaterial, SIGNAL(currentIndexChanged(int)), m_glWidget, SLOT(setDisplayMtrlMeshWires(int)));

    connect(ui->actFileExportMesh, SIGNAL(triggered()), m_glWidget, SLOT(actFileExportMesh()));
    connect(ui->actFileOpen, SIGNAL(triggered()), m_glWidget, SLOT(actOpenProject()));
    connect(ui->actFileOpenLoadBenchmark, SIGNAL(triggered()), m_glWidget, SLOT(actOpenMedusa()));
    connect(ui->actFileSave, SIGNAL(triggered()), m_glWidget, SLOT(actSaveProject()));
    connect(ui->actFileClose, SIGNAL(triggered()), m_glWidget, SLOT(actCloseProject()));
    connect(ui->actFileExit, SIGNAL(triggered()), this, SLOT(close()));

    connect(ui->actMeshSubDivide, SIGNAL(triggered()), m_glWidget, SLOT(actMeshSubDivide()));
    connect(ui->actMeshPolygonize, SIGNAL(triggered()), m_glWidget, SLOT(actMeshPolygonize()));
    connect(ui->actMeshInsert, SIGNAL(triggered()), m_glWidget, SLOT(actMeshInsert()));

    connect(ui->actAnimDrawGuide, SIGNAL(triggered()), m_glWidget, SLOT(actAnimDrawGuide()));
    connect(ui->actAnimStart, SIGNAL(triggered()), m_glWidget, SLOT(actAnimStart()));
    connect(ui->actAnimStop, SIGNAL(triggered()), m_glWidget, SLOT(actAnimStop()));
    connect(ui->actAnimSetStartLoc, SIGNAL(triggered()), m_glWidget, SLOT(actAnimSetStartLoc()));
    connect(ui->actAnimSetEndLoc, SIGNAL(triggered()), m_glWidget, SLOT(actAnimSetEndLoc()));
    connect(ui->actAnimSetStartScale, SIGNAL(triggered()), m_glWidget, SLOT(actAnimSetStartScale()));
    connect(ui->actAnimSetEndScale, SIGNAL(triggered()), m_glWidget, SLOT(actAnimSetEndScale()));
    connect(ui->actAnimRemoveAll, SIGNAL(triggered()), m_glWidget, SLOT(actAnimRemoveAll()));


    connect(m_glWidget, SIGNAL(sig_readApplySettings()), this, SLOT(readApplySetting()));
    //=========================================================================================
    connect(m_glWidget, SIGNAL(sig_addNetLog(const QString&)), ui->lstNetLog, SLOT(addItem(const QString&)));
    connect(m_glWidget, SIGNAL(sig_showBlobTree(QStandardItemModel*)), this, SLOT(showBlobTree(QStandardItemModel*)));
    connect(m_glWidget, SIGNAL(sig_showLayerManager(QStandardItemModel*)), this, SLOT(showLayerManager(QStandardItemModel*)));
    connect(m_glWidget, SIGNAL(sig_showPrimitiveProperty(QStandardItemModel*)), this, SLOT(showPrimitiveProperty(QStandardItemModel*)));
    connect(m_glWidget, SIGNAL(sig_showStats(QStandardItemModel*)), this, SLOT(showStats(QStandardItemModel*)));

    connect(m_glWidget, SIGNAL(sig_showColorRibbon(QStandardItemModel*)), this, SLOT(showColorRibbon(QStandardItemModel*)));
    connect(m_glWidget, SIGNAL(sig_setPrimitiveColor(QColor)), this, SLOT(setPrimitiveColor(QColor)));
    connect(ui->btnPrimitiveColor, SIGNAL(clicked()), m_glWidget, SLOT(setPrimitiveColorFromColorDlg()));

    connect(ui->treeBlob, SIGNAL(clicked(QModelIndex)), m_glWidget, SLOT(selectBlobNode(QModelIndex)));
    connect(ui->tblLayers, SIGNAL(clicked(QModelIndex)), m_glWidget, SLOT(selectLayer(QModelIndex)));

    //connect(m_glWidget, SIGNAL(sig_showLayerManager(const QStringList&)), this, SLOT(showLayerManager(const QStringList&)));
    //connect(ui->lstLayers, SIGNAL(currentRowChanged(int)), m_glWidget, SLOT(selectLayer(int)));
    connect(ui->btnLayerAdd, SIGNAL(clicked()), m_glWidget, SLOT(actLayerAdd()));
    connect(ui->btnLayerDelete, SIGNAL(clicked()), m_glWidget, SLOT(actLayerDelete()));
    connect(ui->btnLayerDuplicate, SIGNAL(clicked()), m_glWidget, SLOT(actLayerDuplicate()));
    connect(ui->btnLayerSelectAll, SIGNAL(clicked()), m_glWidget, SLOT(actLayerSelectAll()));
    connect(m_glWidget, SIGNAL(sig_enableHasLayer(bool)), ui->btnLayerDelete, SLOT(setEnabled(bool)));
    connect(m_glWidget, SIGNAL(sig_enableHasLayer(bool)), ui->btnLayerDuplicate, SLOT(setEnabled(bool)));
    connect(m_glWidget, SIGNAL(sig_enableHasLayer(bool)), ui->btnLayerSelectAll, SLOT(setEnabled(bool)));
    //=========================================================================================
    connect(ui->actViewResetCamera, SIGNAL(triggered()), m_glWidget, SLOT(actViewResetCamera()));
    connect(ui->actViewFullScreen, SIGNAL(triggered()), this, SLOT(actViewFullScreen()));
    //=========================================================================================
    connect(ui->actAddSphere, SIGNAL(triggered()), m_glWidget, SLOT(actAddSphere()));
    connect(ui->actAddCylinder, SIGNAL(triggered()), m_glWidget, SLOT(actAddCylinder()));
    connect(ui->actAddRing, SIGNAL(triggered()), m_glWidget, SLOT(actAddRing()));
    connect(ui->actAddDisc, SIGNAL(triggered()), m_glWidget, SLOT(actAddDisc()));
    connect(ui->actAddPolygonPlane, SIGNAL(triggered()), m_glWidget, SLOT(actAddPolygonPlane()));
    connect(ui->actAddCube, SIGNAL(triggered()), m_glWidget, SLOT(actAddCube()));
    connect(ui->actAddTriangle, SIGNAL(triggered()), m_glWidget, SLOT(actAddTriangle()));

    connect(ui->actAddUnion, SIGNAL(triggered()), m_glWidget, SLOT(actAddUnion()));
    connect(ui->actAddIntersection, SIGNAL(triggered()), m_glWidget, SLOT(actAddIntersection()));
    connect(ui->actAddDifference, SIGNAL(triggered()), m_glWidget, SLOT(actAddDifference()));
    connect(ui->actAddSmoothDif, SIGNAL(triggered()), m_glWidget, SLOT(actAddSmoothDif()));
    connect(ui->actAddBlend, SIGNAL(triggered()), m_glWidget, SLOT(actAddBlend()));
    connect(ui->actAddRicciBlend, SIGNAL(triggered()), m_glWidget, SLOT(actAddRicciBlend()));
    connect(ui->actAddCache, SIGNAL(triggered()), m_glWidget, SLOT(actAddCache()));
    connect(ui->actAddTwist, SIGNAL(triggered()), m_glWidget, SLOT(actAddWarpTwist()));
    connect(ui->actAddTaper, SIGNAL(triggered()), m_glWidget, SLOT(actAddWarpTaper()));
    connect(ui->actAddBend, SIGNAL(triggered()), m_glWidget, SLOT(actAddWarpBend()));


    connect(ui->actEditSelect, SIGNAL(triggered()), m_glWidget, SLOT(actEditSelect()));
    connect(ui->actEditMultiSelect, SIGNAL(triggered(bool)), m_glWidget, SLOT(actEditMultiSelect(bool)));
    connect(ui->actEditTranslate, SIGNAL(triggered()), m_glWidget, SLOT(actEditTranslate()));
    connect(ui->actEditRotate, SIGNAL(triggered()), m_glWidget, SLOT(actEditRotate()));
    connect(ui->actEditScale, SIGNAL(triggered()), m_glWidget, SLOT(actEditScale()));
    connect(ui->actEditAlongX, SIGNAL(triggered()), m_glWidget, SLOT(actEditAxisX()));
    connect(ui->actEditAlongY, SIGNAL(triggered()), m_glWidget, SLOT(actEditAxisY()));
    connect(ui->actEditAlongZ, SIGNAL(triggered()), m_glWidget, SLOT(actEditAxisZ()));
    connect(ui->actEditDeletePrimitive, SIGNAL(triggered()), m_glWidget, SLOT(actEditDelete()));
    connect(ui->actEditTransformSkeleton, SIGNAL(triggered(bool)), m_glWidget, SLOT(actEditTransformSkeleton(bool)));
    connect(ui->actEditConvert, SIGNAL(triggered()), m_glWidget, SLOT(actEditConvertToBinaryTree()));


    connect(ui->actEditUndo, SIGNAL(triggered()), m_glWidget, SLOT(actEditUndo()));
    connect(ui->actEditRedo, SIGNAL(triggered()), m_glWidget, SLOT(actEditRedo()));

    connect(ui->actEditCopy, SIGNAL(triggered()), m_glWidget, SLOT(actEditCopy()));
    connect(ui->actEditField, SIGNAL(triggered()), this, SLOT(actEditFieldEditor()));
    connect(m_glWidget, SIGNAL(sig_enableUndo(bool)), ui->actEditUndo, SLOT(setEnabled(bool)));
    connect(m_glWidget, SIGNAL(sig_enableRedo(bool)), ui->actEditRedo, SLOT(setEnabled(bool)));


    connect(ui->actViewPanCamera, SIGNAL(triggered(bool)), m_glWidget, SLOT(actViewEnablePan(bool)));

    connect(ui->sliderMouseDragScale, SIGNAL(valueChanged(int)), m_glWidget, SLOT(setMouseDragScale(int)));

    //Color Ribbon
    connect(ui->btnEditMaterial, SIGNAL(clicked()), m_glWidget, SLOT(edit_tblColorRibbon()));
    connect(ui->tblColors, SIGNAL(clicked(QModelIndex)), m_glWidget, SLOT(select_tblColorRibbon(QModelIndex)));

    connect(ui->actHelpAbout, SIGNAL(triggered()), this, SLOT(actHelpAbout()));

    //tests
    connect(ui->btnRunTests, SIGNAL(clicked()), m_glWidget, SLOT(actTestStart()));
    connect(ui->udTestRuns, SIGNAL(valueChanged(int)), m_glWidget, SLOT(actTestSetRuns(int)));

    //Stats
    connect(m_glWidget, SIGNAL(sig_setTimeFPS(double, int)), this, SLOT(updateStatusBar(double, int)));
    connect(m_glWidget, SIGNAL(sig_setProgress(int,int, int)), this, SLOT(updatePrgBar(int,int,int)));

    //For updating and initializing variables
    connect(this, SIGNAL(sig_userInterfaceReady()), m_glWidget, SLOT(userInterfaceReady()));
    connect(this, SIGNAL(sig_loadProject(QString)), m_glWidget, SLOT(actOpenProject(QString)));


    connect(ui->btnNetStart, SIGNAL(clicked()), this, SLOT(actNetStart()));
    connect(ui->btnNetStop, SIGNAL(clicked()), this, SLOT(actNetStop()));
    connect(ui->btnNetAddParticipant, SIGNAL(clicked()), this, SLOT(actNetAddParticipant()));
    connect(ui->btnNetDelParticipant, SIGNAL(clicked()), this, SLOT(actNetDelParticipant()));
    connect(ui->edtNetSendText, SIGNAL(returnPressed()), this, SLOT(actNetSendText()));
    connect(ui->lstNetParticipants, SIGNAL(currentRowChanged(int)), this, SLOT(actNetSendText(int)));

    //Grouping primitive actions in an action group
    m_actgroupPrims = new QActionGroup(this);
    m_actgroupPrims->addAction(ui->actAddSphere);
    m_actgroupPrims->addAction(ui->actAddCylinder);
    m_actgroupPrims->addAction(ui->actAddRing);
    m_actgroupPrims->addAction(ui->actAddDisc);
    m_actgroupPrims->addAction(ui->actAddCube);
    m_actgroupPrims->addAction(ui->actAddTriangle);
    m_actgroupPrims->setExclusive(true);
    /*
 m_actgroupOps = new QActionGroup(this);
 m_actgroupOps->addAction(ui->actAddUnion);
 m_actgroupOps->addAction(ui->actAddIntersection);
 m_actgroupOps->addAction(ui->actAddDifference);
 m_actgroupOps->addAction(ui->actAddSmoothDif);
 m_actgroupOps->addAction(ui->actAddBlend);
 m_actgroupOps->addAction(ui->actAddRicciBlend);
 m_actgroupOps->addAction(ui->actAddTwist);
 m_actgroupOps->addAction(ui->actAddTaper);
 m_actgroupOps->addAction(ui->actAddBend);
*/

    m_dlgFieldEditor = new DlgFieldFunctionEditor(this);
    //
    m_statusBarMsg = new QLabel();
    m_statusBarMsg->setMinimumWidth(m_statusBarMsg->fontMetrics().width("CACHE: L1=0000, L2=0000 KB, RAM= 00000 MB"));
    statusBar()->addPermanentWidget(m_statusBarMsg);

    // Status bar widgets
    m_prgBar = new QProgressBar();
    m_prgBar->setMinimumWidth(200);
    statusBar()->addPermanentWidget(m_prgBar);

    m_statusBarTime = new QLabel();
    m_statusBarTime->setMinimumWidth(m_statusBarTime->fontMetrics().width("Poly Time: 000.00ms"));
    statusBar()->addPermanentWidget(m_statusBarTime);

    m_statusBarFPS = new QLabel();
    m_statusBarFPS->setMinimumWidth(m_statusBarTime->fontMetrics().width("FPS: 000"));
    statusBar()->addPermanentWidget(m_statusBarFPS);

    resetStatusBar();

    //Read and display settings
    readApplySetting();
    readApplySettingNet();

    emit sig_userInterfaceReady();
}

MainWindow::~MainWindow()
{
    actNetSaveSettings();
    delete m_dlgFieldEditor;
    delete m_pDesignNet;
    delete ui;
    delete m_glWidget;
    delete m_statusBarTime;
    delete m_statusBarFPS;
    delete m_statusBarMsg;
    delete m_prgBar;
    delete m_actgroupPrims;
}

void MainWindow::actEditFieldEditor()
{
    m_dlgFieldEditor->setModal(true);
    m_dlgFieldEditor->show();

}

void MainWindow::updateStatusBar(double time, int fps)
{
    QString timeStr;
    timeStr.setNum(time, 'f', 2);

    m_statusBarTime->setText(QString("Time: %1ms").arg(timeStr));
    m_statusBarFPS->setText(QString("FPS: %1").arg(fps));
}

void MainWindow::updatePrgBar( int min, int max, int val )
{
    m_prgBar->setRange(min, max);
    m_prgBar->setValue(val);
}

void MainWindow::resetStatusBar()
{
    // Restore the status bar items to their default values
    uint32 cl1, cl2;
    get_cache_sizes(&cl1, &cl2);
    cl1 = cl1 >> 10;
    cl2 = cl2 >> 10;

    uint64 szRam = get_ram_size();
    szRam = szRam >> 20; //In MB
    m_statusBarMsg->setText(QString("CACHE: L1=%1, L2=%2 KB, RAM= %3 MB").arg(cl1).arg(cl2).arg(szRam));
    m_statusBarTime->setText("Poly Time: 000.00ms");
    m_statusBarFPS->setText("FPS: 0");
    m_prgBar->setValue(0);
}

void MainWindow::readApplySetting()
{
    SettingsParsip setParsip;
    SettingsDisplay setDisplay;
    int dragScale = 100;

    DAnsiStr strFP = PS::FILESTRINGUTILS::GetExePath() + ".ini";
    if(!PS::FILESTRINGUTILS::FileExists(strFP))
        return;

    CAppConfig* cfg = new CAppConfig(strFP, CAppConfig::fmRead);

    //setParsip.ctThreads = cfg->readInt("parsip", "threadscount", setParsip.ctThreads);
    memset(&setParsip, 0, sizeof(setParsip));
    memset(&setDisplay, 0, sizeof(setDisplay));
    setParsip.griddim		= cfg->readInt("parsip", "griddimension", setParsip.griddim);
    setParsip.cellSize		= cfg->readFloat("parsip", "cellsize", setParsip.cellSize);
    setParsip.adaptiveParam = cfg->readFloat("parsip", "adaptiveparam", setParsip.adaptiveParam);
    setParsip.cellShape = static_cast<CellShape>(cfg->readInt("parsip", "cellshape", static_cast<int>(setParsip.cellShape)));
    setParsip.bUseAdaptiveSubDivision = cfg->readBool("parsip", "useadaptivesubdivision", setParsip.bUseAdaptiveSubDivision);
    setParsip.bUseTBB			 = cfg->readBool("parsip", "usetbb", setParsip.bUseTBB);
    setParsip.bUseComputeShaders = cfg->readBool("parsip", "usecomputeshaders", setParsip.bUseComputeShaders);
    setParsip.bForceMC			 = cfg->readBool("parsip", "forcemarchingcubes", setParsip.bForceMC);


    setDisplay.showMesh			 = cfg->readInt("display", "drawmesh", setDisplay.showMesh);
    setDisplay.bShowBoxLayer	 = cfg->readBool("display", "drawboxlayer", setDisplay.bShowBoxLayer);
    setDisplay.bShowBoxPrimitive = cfg->readBool("display", "drawboxprimitive", setDisplay.bShowBoxPrimitive);
    setDisplay.bShowBoxPoly		 = cfg->readBool("display", "drawboxpoly", setDisplay.bShowBoxPoly);
    setDisplay.bShowAnimCurves = cfg->readBool("display", "drawanimcurves", setDisplay.bShowAnimCurves);

    setDisplay.bShowSeedPoints   = cfg->readBool("display", "drawseedpoints", setDisplay.bShowSeedPoints);
    setDisplay.bShowNormals		 = cfg->readBool("display", "drawnormals", setDisplay.bShowNormals);

    setDisplay.normalLength			 = cfg->readInt("display", "normallength", setDisplay.normalLength);
    setDisplay.bDrawChessboardGround = cfg->readBool("display", "drawchessboard", setDisplay.bDrawChessboardGround);
    setDisplay.bShowColorCodedMPUs = cfg->readBool("display", "colorcodedmpu", setDisplay.bShowColorCodedMPUs);
    setDisplay.bShowGraph		   = cfg->readBool("display", "graph", setDisplay.bShowGraph);
    dragScale					   = cfg->readInt("sketch", "mousedragscale", dragScale);

    SAFE_DELETE(cfg);


    //ui->sliderThreadsCount->setValue(setParsip.ctThreads);
    ui->cboGridDim->setCurrentIndex(setParsip.griddim > 0?(Log2i(setParsip.griddim) - 3):0);
    ui->udNormalsAngle->setValue(setParsip.adaptiveParam);
    ui->udThreadsCount->setValue(setParsip.ctThreads);
    ui->rbCube->setChecked(setParsip.cellShape == csCube);
    ui->rbTetraHedra->setChecked(setParsip.cellShape == csTetrahedra);
    ui->udCubeSize->setValue(setParsip.cellSize);

    ui->chkTBB->setChecked(setParsip.bUseTBB);
    ui->chkComputeShaders->setChecked(setParsip.bUseComputeShaders);
    ui->chkForceMC->setChecked(setParsip.bForceMC);
    ui->chkEnableAdaptive->setChecked(setParsip.bUseAdaptiveSubDivision);

    ui->rbShowMeshNone->setChecked(setDisplay.showMesh == smNone);
    ui->rbShowMeshWireFrame->setChecked(setDisplay.showMesh == smWireFrame);
    ui->rbShowMeshSurface->setChecked(setDisplay.showMesh == smSurface);
    ui->chkShowBoxLayer->setChecked(setDisplay.bShowBoxLayer);
    ui->chkShowBoxPrimitive->setChecked(setDisplay.bShowBoxPrimitive);
    ui->chkShowBoxPoly->setChecked(setDisplay.bShowBoxPoly);
    ui->chkShowAnimationCurves->setChecked(setDisplay.bShowAnimCurves);

    ui->chkShowChessBoard->setChecked(setDisplay.bDrawChessboardGround);
    ui->chkShowSeedPoints->setChecked(setDisplay.bShowSeedPoints);
    ui->chkShowNormals->setChecked(setDisplay.bShowNormals);
    ui->sliderNormalLength->setValue(setDisplay.normalLength);

    ui->chkColorCodeMPUMesh->setChecked(setDisplay.bShowColorCodedMPUs);
    ui->chkShowGraph->setChecked(setDisplay.bShowGraph);
    ui->sliderMouseDragScale->setValue(dragScale);
}

void MainWindow::setParsipCellSize(int sliderValue)
{
    float cellsize = sliderValue / static_cast<float>(CELL_SIZE_SCALE);
    ui->udCubeSize->setValue(cellsize);
    m_glWidget->setParsipCellSize(cellsize);
}

void MainWindow::setParsipCellSize(double value)
{
    int sliderValue = static_cast<int>(value * CELL_SIZE_SCALE);
    ui->sliderCubeSize->setValue(sliderValue);
}

void MainWindow::setParsipNormalsAngle(int sliderValue)
{
    float value = static_cast<float>(sliderValue);
    ui->udNormalsAngle->setValue(value);
    m_glWidget->setParsipNormalsGeodesicAngle(value);
}

void MainWindow::setParsipNormalsAngle(double value)
{
    int sliderValue = static_cast<int>(value);
    ui->sliderNormalsAngle->setValue(sliderValue);
}

void MainWindow::showColorRibbon(QStandardItemModel* mdlColorRibbon)
{
    ui->tblColors->setModel(static_cast<QAbstractItemModel*>(mdlColorRibbon));
    ui->tblColors->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tblColors->show();
}

void MainWindow::showBlobTree(QStandardItemModel * mdlBlobTree)
{
    ui->treeBlob->setModel(static_cast<QAbstractItemModel*>(mdlBlobTree));
    ui->treeBlob->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->treeBlob->show();
    ui->treeBlob->expandAll();
}

void MainWindow::showLayerManager(QStandardItemModel* mdlLayerManager)
{
    ui->tblLayers->setModel(static_cast<QAbstractItemModel*>(mdlLayerManager));
    ui->tblLayers->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tblLayers->show();
}

void MainWindow::showPrimitiveProperty(QStandardItemModel* mdlPrimProperty)
{
    ui->tblBlobProperty->setModel(static_cast<QAbstractItemModel*>(mdlPrimProperty));
    ui->tblBlobProperty->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tblBlobProperty->show();
}

void MainWindow::showStats(QStandardItemModel* mdlStats)
{
    ui->tblStats->setModel(static_cast<QAbstractItemModel*>(mdlStats));
    ui->tblStats->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tblStats->show();
}

void MainWindow::actViewFullScreen()
{
    if(ui->actViewFullScreen->text() == QString("Full Screen"))
    {
        this->showFullScreen();
        m_glWidget->showFullScreen();
        ui->actViewFullScreen->setText("Normal View");
    }
    else
    {
        this->showNormal();
        m_glWidget->showNormal();
        ui->actViewFullScreen->setText("Full Screen");
    }

}

void MainWindow::setPrimitiveColor(QColor cl)
{
    QString strColor = QString("background-color: rgb(%1, %2, %3)").arg(cl.red()).arg(cl.green()).arg(cl.blue());
    ui->btnPrimitiveColor->setStyleSheet(strColor);
}

void MainWindow::actHelpAbout()
{
    //QMessageBox::StandardButton reply;
    QMessageBox::information(this, tr("About Parsip"), "CopyRight 2010 - Pourya Shirazian");
}

void MainWindow::updateMsgBar( QString strMsg )
{
    m_statusBarMsg->setText(strMsg);
}

void MainWindow::setCommandLineParam( QString strFile )
{
    if(strFile.length() > 0)
        emit sig_loadProject(strFile);
}

void MainWindow::actNetStart()
{
    if(m_pDesignNet == NULL) return;
    quint16 port = ui->udServerPort->value();
    m_pDesignNet->setPort(port);
    if(m_pDesignNet->start())
    {
        ui->lstNetLog->addItem(QString(">>Server is Listening..."));
        ui->btnNetStart->setEnabled(false);
        ui->btnNetStop->setEnabled(true);
    }
    else ui->lstNetLog->addItem(QString(">>Unable to start server. Close all open connections!"));
}

void MainWindow::actNetStop()
{
    if(m_pDesignNet == NULL) return;
    m_pDesignNet->stop();
    ui->btnNetStart->setEnabled(true);
    ui->btnNetStop->setEnabled(false);
    ui->edtNetSendText->setEnabled(false);

    ui->lstNetLog->addItem(QString(">>Server is shutdown."));
    ui->lstNetParticipants->clear();
    ui->lstNetLog->addItem(QString(">>All connections are closed."));
}

void MainWindow::actNetSendText()
{
    if(m_pDesignNet == NULL) return;
    QString strDest = ui->lstNetParticipants->currentItem()->text();
    QString strMsg = ui->edtNetSendText->text();

    if((strMsg.length() == 0) || (strDest.length() == 0)) return;
    bool bSendToAll = ui->chkNetAll->isChecked();

    strMsg += "\r\n";
    if(m_pDesignNet->sendMessageToMember(strDest, strMsg, bSendToAll))
    {
        ui->lstNetLog->addItem(QString(">>Msg sent to %1").arg(strDest));
        ui->edtNetSendText->clear();
    }
    else
    {
        ui->lstNetLog->addItem(QString(">>ERR: Msg not sent to %1").arg(strDest));
    }
}

void MainWindow::actNetSendText( int idxParticipant )
{
    if(ui->lstNetParticipants->count() > 0)
        ui->edtNetSendText->setEnabled(true);
    else
        ui->edtNetSendText->setEnabled(false);
}


void MainWindow::actNetShowMembers( QStringList names )
{
    ui->lstNetParticipants->clear();
    ui->lstNetParticipants->addItems(names);
    ui->cboNetAddParticipant->clear();
    ui->cboNetAddParticipant->addItems(m_setSketchNet.ips);
}

void MainWindow::actNetNewMessage( int idx, QString strMsg )
{
    if(m_pDesignNet == NULL) return;
    CMember* m = m_pDesignNet->getMember(idx);
    if(strMsg.endsWith("\r\n"))
        strMsg.chop(2);
    if(m)
        ui->lstNetLog->addItem(QString("Msg Recv From %1:%2").arg(m->m_strAlias).arg(strMsg));
    else
        ui->lstNetLog->addItem(QString("Msg Recv From Unknown:%1").arg(strMsg));
}

void MainWindow::actNetAddParticipant()
{
    if(m_pDesignNet == NULL) return;
    QString strAddress = ui->cboNetAddParticipant->currentText();
    m_pDesignNet->addMember(strAddress);

    if(m_setSketchNet.ips.indexOf(strAddress) == -1)
    {
        m_setSketchNet.ips.append(strAddress);
        actNetSaveSettings();
    }
}

void MainWindow::actNetDelParticipant()
{
    if(m_pDesignNet == NULL) return;
    QString strAlias = ui->cboNetAddParticipant->currentText();
    m_pDesignNet->removeMemberByName(strAlias);
}


void MainWindow::actNetSaveSettings()
{
    m_setSketchNet.bAutoConnect = true;
    m_setSketchNet.port = ui->udServerPort->value();

    DAnsiStr strFP = PS::FILESTRINGUTILS::CreateNewFileAtRoot(".net");
    CAppConfig* cfg = new CAppConfig(strFP, CAppConfig::fmWrite);
    cfg->writeBool("sketchnet", "autoconnect", m_setSketchNet.bAutoConnect);
    cfg->writeInt("sketchnet", "port", m_setSketchNet.port);
    cfg->writeInt("sketchnet", "members_count", m_setSketchNet.ips.size());
    DAnsiStr strMember, strValue;
    for(int i=0; i<m_setSketchNet.ips.size(); i++)
    {
        strMember = printToAStr("member%d", i);
        strValue = DAnsiStr(m_setSketchNet.ips[i].toAscii().data());
        cfg->writeString("sketchnet", strMember, strValue);
    }

    SAFE_DELETE(cfg);
}

void MainWindow::readApplySettingNet()
{
    DAnsiStr strFP = PS::FILESTRINGUTILS::CreateNewFileAtRoot(".net");
    if(!PS::FILESTRINGUTILS::FileExists(strFP))
        return;
    CAppConfig* cfg = new CAppConfig(strFP, CAppConfig::fmRead);

    m_setSketchNet.bAutoConnect = cfg->readBool("sketchnet", "autoconnect", m_setSketchNet.bAutoConnect);
    m_setSketchNet.port = cfg->readInt("sketchnet", "port", m_setSketchNet.port);
    int ctMems = cfg->readInt("sketchnet", "members_count", 0);
    DAnsiStr strMember;
    for(int i=0; i<ctMems; i++)
    {
        strMember = printToAStr("member%d", i);
        m_setSketchNet.ips.push_back(QString(cfg->readString("sketchnet", strMember, "0.0.0.0").ptr()));
    }
    SAFE_DELETE(cfg);

    ui->cboNetAddParticipant->clear();
    ui->cboNetAddParticipant->addItems(m_setSketchNet.ips);
    ui->udServerPort->setValue(m_setSketchNet.port);
    if(m_setSketchNet.bAutoConnect)
        actNetStart();
}
/*
void MainWindow::actNetAcceptConnection()
{
 tcpServerConnection = tcpServer.nextPendingConnection();
 connect(tcpServerConnection, SIGNAL(readyRead()), this, SLOT(actNetRscvData()));
 connect(tcpServerConnection, SIGNAL(error(QAbstractSocket::SocketError)),
   this, SLOT(displayError(QAbstractSocket::SocketError)));
 //Add new participant if not in the list of participants
 QString strPeerAddress = tcpServerConnection->peerAddress().toString();
 //if(strPeerAddress.length() > 0)
 //	actNetAddParticipant(strPeerAddress, tcpServerConnection->localPort(), false);
}

void MainWindow::actNetRecvData()
{
 if(tcpServerConnection == NULL) return;

 int ctBytesRecv = tcpServerConnection->bytesAvailable();
 QByteArray q;
 q.resize(ctBytesRecv);
 q = tcpServerConnection->readAll();

 QString strMsg(q);
 QString strPeerIP = tcpServerConnection->peerAddress().toString();
 ui->lstNetLog->addItem(QString(">>Msg Recv from %1: %2").arg(strPeerIP).arg(strMsg));
 for(size_t i=0;i<m_participants.size();i++)
 {
  if(m_participants[i].strAddress == strPeerIP)
  {
   m_participants[i].ctRecv++;
   m_participants[i].lastActiveTime = QTime::currentTime();
   return;
  }
 }

}

void MainWindow::actNetDisplayError( QAbstractSocket::SocketError socketError )
{
 if (socketError == QTcpSocket::RemoteHostClosedError)
  return;

 actNetStop();
}

void MainWindow::actNetClientConnected()
{
 if(m_pending.size() == 0) return;
 PARTICIPANT p = m_pending.back();
 if(p.lastActiveTime.secsTo(QTime::currentTime()) < 10)
 {
  p.strAddress = p.socket->peerAddress().toString();
  m_participants.push_back(p);
  ui->lstNetParticipants->addItem(p.strAddress);
  ui->lstNetLog->addItem(QString(">>Participant Added Successfully"));
 }
 m_pending.pop_back();

 int i=0;
 while(i < m_pending.size())
 {
  p = m_pending[i];
  if(p.lastActiveTime.secsTo(QTime::currentTime()) >= 10)
  {
   SAFE_DELETE(p.socket);
   m_pending.remove(i);
  }
  else
   i++;
 }
}

void MainWindow::actNetAddParticipant( QString address, quint16 port, bool bNeedsAck)
{
 for(size_t i=0;i<m_participants.size();i++)
 {
  if(m_participants[i].strAddress == address)
  {
   m_participants[i].ctRecv++;
   m_participants[i].lastActiveTime = QTime::currentTime();
   return;
  }
 }

 if(address.length() == 0) address = "127.0.0.1";

 PARTICIPANT p;
 p.strAddress = address;
 p.socket = new QTcpSocket();
 p.lastActiveTime = QTime::currentTime();
 p.ctRecv = 0;
 p.ctSent = 0;
 connect(p.socket, SIGNAL(connected()), this, SLOT(actNetClientConnected()));
 //connect(&tcpClient, SIGNAL(bytesWritten(qint64)), this, SLOT(updateClientProgress(qint64)));
 connect(p.socket, SIGNAL(error(QAbstractSocket::SocketError)),
   this, SLOT(displayError(QAbstractSocket::SocketError)));
 if(bNeedsAck)
 {
  m_pending.push_back(p);
 }
 else
 {
  m_participants.push_back(p);
  ui->lstNetParticipants->addItem(p.strAddress);
  ui->lstNetLog->addItem(QString(">>Participant Added Successfully"));
 }

 p.socket->connectToHost(address, port);
}

*/
