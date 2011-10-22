#ifndef DLGFIELDFUNCEDITOR_H
#define DLGFIELDFUNCEDITOR_H

#include <QDialog>
#include <QGLWidget>
#include <QTimer>
#include "PS_FrameWork/include/PS_SplineCatmullRom.h"

using namespace PS;

namespace Ui {
    class DlgFieldFunctionEditor;
}


class GLFieldEditor : public QGLWidget
{
        Q_OBJECT
public:
    //Constructor for GLWidget
        GLFieldEditor(QWidget *parent = 0);

    //Destructor for GLWidget
    ~GLFieldEditor();

public slots:

signals:



protected:
    //Initialize the OpenGL Graphics Engine
    void initializeGL();

    //All our painting stuff are here
    void paintGL();

    //When user resizes main window, the scrollArea will be resized and it will call this function from
    //its attached GLWidget
    void resizeGL(int width, int height);

    //Handle mouse press event in scrollArea
    void mousePressEvent(QMouseEvent *event);

        //Handle mouse release event in scrollArea
        void mouseReleaseEvent(QMouseEvent *event);

        //Handle mouse move event in scrollArea
        void mouseMoveEvent(QMouseEvent *event);

        //Handle keyboard for blender-style shortcut keys
        //void keyPressEvent(QKeyEvent *event);
private:
        CSplineCatmullRom* m_lpCurve;
};
//////////////////////////////////////////////////////////////////////////

class DlgFieldFunctionEditor : public QDialog
{
    Q_OBJECT

public:
    explicit DlgFieldFunctionEditor(QWidget *parent = 0);
    ~DlgFieldFunctionEditor();

public slots:
        void actPopulateList(int indexMember, const QStringList& lstActions);
        void actRunAll();
        void actRunNext();
        void actStop();
        void actDelete();
        void actDeleteAll();
        void actOpenFile();

signals:
        bool sig_actExecuteCommand(int idxMember, QString strMsg);
        void sig_actSetProgress(int iMin, int iMax, int iVal);


private:
        Ui::DlgFieldFunctionEditor	*ui;
        GLFieldEditor*		m_glEditor;
        QTimer* m_timer;
        int m_actIndexMember;
        int m_actCurrentItem;

};



#endif // DLGFIELDFUNCTIONEDITOR_H
