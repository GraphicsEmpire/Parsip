#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QLabel>
#include <QProgressBar>
#include <QString>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTime>
#include <QActionGroup>
#include "DSystem/include/DContainers.h"
#include "CBlobTreeNetwork.h"
#include "DlgFieldFunctionEditor.h"

class QTcpServer;
class QTcpSocket;
class QString;
class QTime;

using namespace PS;

namespace Ui {
    class MainWindow;
}

class GLWidget;

struct PARTICIPANT{
        QTcpSocket* socket;
        QString strAddress;
        QTime lastActiveTime;
        qint64 ctSent;
        qint64 ctRecv;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
        void setParsipCellSize(int sliderValue);
        void setParsipCellSize(double value);

        void setParsipNormalsAngle(int sliderValue);
        void setParsipNormalsAngle(double value);

        void updateMsgBar(QString strMsg);
        void updatePrgBar(int min, int max, int val);
        void updateStatusBar(double time, int fps);
        void resetStatusBar();
        void readApplySetting();
        void readApplySettingNet();

        void setCommandLineParam(QString strFile);


        void showColorRibbon(QStandardItemModel* mdlColorRibbon);
        void showBlobTree(QStandardItemModel* mdlBlobTree);
        void showLayerManager(QStandardItemModel* mdlLayerManager);
        void showPrimitiveProperty(QStandardItemModel* mdlPrimProperty);
        void showStats(QStandardItemModel* mdlStats);
        void setPrimitiveColor(QColor cl);
        void actViewFullScreen();
        void actHelpAbout();


        void actNetStart();
        void actNetStop();
        void actNetSendText();
        void actNetSendText(int idxParticipant);
        void actNetNewMessage(int idx, QString strMsg);
        void actNetAddParticipant();
        void actNetDelParticipant();
        void actNetShowMembers(QStringList names);
        //void actNetLoadSettings();
        void actNetSaveSettings();

        void actEditFieldEditor();
        /*
        void actNetAcceptConnection();
        void actNetClientConnected();
        void actNetAddParticipant(QString address, quint16 port, bool bNeedsAck = false);
        void actNetDisplayError(QAbstractSocket::SocketError socketError);
        void actNetRecvData();
        */
signals:
        void sig_userInterfaceReady();
        void sig_loadProject(QString strFile);

private:
        DlgFieldFunctionEditor* m_dlgFieldEditor;
        CDesignNet*  m_pDesignNet;
        SettingsNetwork m_setSketchNet;
    Ui::MainWindow *ui;
    GLWidget    *m_glWidget;
        QLabel		*m_statusBarMsg;
        QLabel		*m_statusBarFPS;
        QLabel		*m_statusBarTime;
        QProgressBar *m_prgBar;

        QActionGroup *m_actgroupPrims;
        //QActionGroup *m_actgroupOps;

};

#endif // MAINWINDOW_H
