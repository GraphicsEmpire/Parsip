#include "DlgFieldFunctionEditor.h"
#include "ui_DlgFieldFunction.h"

#include <QFileDialog>
#include <QFile>
#include <qtextstream.h>

GLFieldEditor::GLFieldEditor(QWidget* parent):QGLWidget(parent)
{

}

GLFieldEditor::~GLFieldEditor()
{
	m_curve.removeAll();
}

void GLFieldEditor::initializeGL()
{
	glClearColor(1.0, 1.0, 1.0, 1.0);
	m_curve.addPoint(vec3f(-1.0f, 1.0f, 0.0f));
	m_curve.addPoint(vec3f(0.0f, 1.0f, 0.0f));
	m_curve.addPoint(vec3f(0.5f, 0.42f, 0.0f));
	m_curve.addPoint(vec3f(1.0f, 0.0f, 0.0f));
	m_curve.addPoint(vec3f(2.0f, 0.0f, 0.0f));
	m_curve.populateTableAdaptive();
}

//All our painting stuff are here
void GLFieldEditor::paintGL()
{
    //Clear target buffer and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glPushAttrib(GL_ALL_ATTRIB_BITS);
		glLineWidth(3.0f);
		glColor3f(0.0f, 0.0f, 0.0f);
		m_curve.drawCurve(GL_LINE_STRIP);
	
		glPointSize(5.0f);
		glColor3f(0.0f, 1.0f, 0.0f);
		m_curve.drawCtrlLine(GL_POINTS);

		DVec<vec3f> lstPoints;
		m_curve.getArcPoints(lstPoints);
	
		glColor3f(0.35f, 0.43f, 0.86f);
		glBegin(GL_POLYGON);
			glVertex3f(0.0f, 0.0f, 0.0f);
			for(size_t i=0; i<lstPoints.size(); i++)
			{
				glVertex3fv(lstPoints[i].ptr());
			}
		glEnd();
	glPopAttrib();
}

//When user resizes main window, the scrollArea will be resized and it will call this function from
//its attached GLWidget
void GLFieldEditor::resizeGL(int width, int height)
{
	glViewport (0, 0, (GLsizei) width, (GLsizei) height);

	//Set projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, 1.0, 0.0, 1.0);

	//Return to ModelView Matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

//Handle mouse press event in scrollArea
void GLFieldEditor::mousePressEvent(QMouseEvent *event)
{
}

//Handle mouse release event in scrollArea
void GLFieldEditor::mouseReleaseEvent(QMouseEvent *event)
{
}

//Handle mouse move event in scrollArea
void GLFieldEditor::mouseMoveEvent(QMouseEvent *event)
{
}
/////////////////////////////////////////////////////////////////////////
DlgFieldFunctionEditor::DlgFieldFunctionEditor(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DlgFieldFunctionEditor)
{	
	ui->setupUi(this);

	m_glEditor = new GLFieldEditor;
	ui->scrollAreaOpenGL->setWidget(m_glEditor);
	m_timer = new QTimer(this);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(actRunNext()));
	connect(ui->btnRunNext, SIGNAL(clicked()), this, SLOT(actRunNext()));
	connect(ui->btnRunAll, SIGNAL(clicked()), this, SLOT(actRunAll()));
	connect(ui->btnStop, SIGNAL(clicked()), this, SLOT(actStop()));
	connect(ui->btnOpen, SIGNAL(clicked()), this, SLOT(actOpenFile()));
	connect(ui->btnClearAll, SIGNAL(clicked()), this, SLOT(actDeleteAll()));
	connect(ui->btnDelete, SIGNAL(clicked()), this, SLOT(actDelete()));
	//connect(ui->btnDelete, SIGNAL(clicked()), this, SLOT(actDelete()));

	m_actCurrentItem = 0;
	m_actIndexMember = 0;
}

DlgFieldFunctionEditor::~DlgFieldFunctionEditor()
{
	delete m_glEditor;
	delete m_timer;
}

void DlgFieldFunctionEditor::actPopulateList( int indexMember, const QStringList& lstActions )
{
	m_actIndexMember = indexMember;
	m_actCurrentItem = 0;
	m_timer->stop();
	ui->lstActions->clear();
	ui->lstActions->addItems(lstActions);
}

void DlgFieldFunctionEditor::actDeleteAll()
{
	ui->lstActions->clear();
}

void DlgFieldFunctionEditor::actDelete()
{
	int i=0;
	while(i < ui->lstActions->count())
	{
		if(ui->lstActions->item(i)->isSelected())
		{
			QListWidgetItem * w = ui->lstActions->item(i);
			delete (w);
		}			
		else
			i++;
	}
}

void DlgFieldFunctionEditor::actStop()
{
	m_timer->stop();
}

void DlgFieldFunctionEditor::actRunAll()
{
	m_timer->start(100);
}

void DlgFieldFunctionEditor::actRunNext()
{
	if((m_actCurrentItem >= 0) && (m_actCurrentItem < ui->lstActions->count()))
	{
		QString strAction = ui->lstActions->item(m_actCurrentItem)->text(); 
		
		//Send Signals
		emit sig_actExecuteCommand(0, strAction);
		emit sig_actSetProgress(0, ui->lstActions->count(), m_actCurrentItem + 1);


		ui->lstActions->setItemSelected(ui->lstActions->item(m_actCurrentItem), true);
		ui->lblRun->setText(QString("%1/%2").arg(m_actCurrentItem+1).arg(ui->lstActions->count()));
		m_actCurrentItem++;
	}
}

void DlgFieldFunctionEditor::actOpenFile()
{
	QString strFileName = QFileDialog::getOpenFileName(this, tr("Open Actions"), 
						  QString("C:\\"), tr("SketchNET Actions(*.acts)"));

	QStringList lstCommands;
	QFile fInput(strFileName);
	if (fInput.open(QFile::ReadOnly | QFile::Text)) 
	{
		QTextStream s(&fInput);
		while(!s.atEnd())
		{
			QString strLine = s.readLine();
			lstCommands.push_back(strLine);
		}
	} 
	fInput.close();

	actPopulateList(0, lstCommands);
}
