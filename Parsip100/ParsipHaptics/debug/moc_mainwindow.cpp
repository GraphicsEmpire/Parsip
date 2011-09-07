/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created: Wed Sep 7 15:54:40 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../mainwindow.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MainWindow[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      31,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x05,
      45,   37,   11,   11, 0x05,

 // slots: signature, parameters, type, tag, flags
      82,   70,   11,   11, 0x0a,
     111,  105,   11,   11, 0x0a,
     137,   70,   11,   11, 0x0a,
     164,  105,   11,   11, 0x0a,
     201,  194,   11,   11, 0x0a,
     235,  223,   11,   11, 0x0a,
     270,  261,   11,   11, 0x0a,
     298,   11,   11,   11, 0x0a,
     315,   11,   11,   11, 0x0a,
     334,   11,   11,   11, 0x0a,
     356,   37,   11,   11, 0x0a,
     400,  385,   11,   11, 0x0a,
     449,  437,   11,   11, 0x0a,
     499,  483,   11,   11, 0x0a,
     553,  537,   11,   11, 0x0a,
     605,  596,   11,   11, 0x0a,
     639,  636,   11,   11, 0x0a,
     665,   11,   11,   11, 0x0a,
     685,   11,   11,   11, 0x0a,
     700,   11,   11,   11, 0x0a,
     714,   11,   11,   11, 0x0a,
     727,   11,   11,   11, 0x0a,
     759,  744,   11,   11, 0x0a,
     790,  779,   11,   11, 0x0a,
     820,   11,   11,   11, 0x0a,
     843,   11,   11,   11, 0x0a,
     872,  866,   11,   11, 0x0a,
     903,   11,   11,   11, 0x0a,
     924,   11,   11,   11, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_MainWindow[] = {
    "MainWindow\0\0sig_userInterfaceReady()\0"
    "strFile\0sig_loadProject(QString)\0"
    "sliderValue\0setParsipCellSize(int)\0"
    "value\0setParsipCellSize(double)\0"
    "setParsipNormalsAngle(int)\0"
    "setParsipNormalsAngle(double)\0strMsg\0"
    "updateMsgBar(QString)\0min,max,val\0"
    "updatePrgBar(int,int,int)\0time,fps\0"
    "updateStatusBar(double,int)\0"
    "resetStatusBar()\0readApplySetting()\0"
    "readApplySettingNet()\0"
    "setCommandLineParam(QString)\0"
    "mdlColorRibbon\0showColorRibbon(QStandardItemModel*)\0"
    "mdlBlobTree\0showBlobTree(QStandardItemModel*)\0"
    "mdlLayerManager\0showLayerManager(QStandardItemModel*)\0"
    "mdlPrimProperty\0"
    "showPrimitiveProperty(QStandardItemModel*)\0"
    "mdlStats\0showStats(QStandardItemModel*)\0"
    "cl\0setPrimitiveColor(QColor)\0"
    "actViewFullScreen()\0actHelpAbout()\0"
    "actNetStart()\0actNetStop()\0actNetSendText()\0"
    "idxParticipant\0actNetSendText(int)\0"
    "idx,strMsg\0actNetNewMessage(int,QString)\0"
    "actNetAddParticipant()\0actNetDelParticipant()\0"
    "names\0actNetShowMembers(QStringList)\0"
    "actNetSaveSettings()\0actEditFieldEditor()\0"
};

const QMetaObject MainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_MainWindow,
      qt_meta_data_MainWindow, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MainWindow::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow))
        return static_cast<void*>(const_cast< MainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: sig_userInterfaceReady(); break;
        case 1: sig_loadProject((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: setParsipCellSize((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: setParsipCellSize((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 4: setParsipNormalsAngle((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: setParsipNormalsAngle((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 6: updateMsgBar((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 7: updatePrgBar((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 8: updateStatusBar((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 9: resetStatusBar(); break;
        case 10: readApplySetting(); break;
        case 11: readApplySettingNet(); break;
        case 12: setCommandLineParam((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 13: showColorRibbon((*reinterpret_cast< QStandardItemModel*(*)>(_a[1]))); break;
        case 14: showBlobTree((*reinterpret_cast< QStandardItemModel*(*)>(_a[1]))); break;
        case 15: showLayerManager((*reinterpret_cast< QStandardItemModel*(*)>(_a[1]))); break;
        case 16: showPrimitiveProperty((*reinterpret_cast< QStandardItemModel*(*)>(_a[1]))); break;
        case 17: showStats((*reinterpret_cast< QStandardItemModel*(*)>(_a[1]))); break;
        case 18: setPrimitiveColor((*reinterpret_cast< QColor(*)>(_a[1]))); break;
        case 19: actViewFullScreen(); break;
        case 20: actHelpAbout(); break;
        case 21: actNetStart(); break;
        case 22: actNetStop(); break;
        case 23: actNetSendText(); break;
        case 24: actNetSendText((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 25: actNetNewMessage((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 26: actNetAddParticipant(); break;
        case 27: actNetDelParticipant(); break;
        case 28: actNetShowMembers((*reinterpret_cast< QStringList(*)>(_a[1]))); break;
        case 29: actNetSaveSettings(); break;
        case 30: actEditFieldEditor(); break;
        default: ;
        }
        _id -= 31;
    }
    return _id;
}

// SIGNAL 0
void MainWindow::sig_userInterfaceReady()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void MainWindow::sig_loadProject(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
