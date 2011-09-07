/****************************************************************************
** Meta object code from reading C++ file 'DlgFieldFunctionEditor.h'
**
** Created: Wed Sep 7 15:54:40 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../DlgFieldFunctionEditor.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DlgFieldFunctionEditor.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_GLFieldEditor[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_GLFieldEditor[] = {
    "GLFieldEditor\0"
};

const QMetaObject GLFieldEditor::staticMetaObject = {
    { &QGLWidget::staticMetaObject, qt_meta_stringdata_GLFieldEditor,
      qt_meta_data_GLFieldEditor, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &GLFieldEditor::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *GLFieldEditor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *GLFieldEditor::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_GLFieldEditor))
        return static_cast<void*>(const_cast< GLFieldEditor*>(this));
    return QGLWidget::qt_metacast(_clname);
}

int GLFieldEditor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGLWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_DlgFieldFunctionEditor[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      46,   29,   24,   23, 0x05,
      96,   81,   23,   23, 0x05,

 // slots: signature, parameters, type, tag, flags
     151,  128,   23,   23, 0x0a,
     184,   23,   23,   23, 0x0a,
     196,   23,   23,   23, 0x0a,
     209,   23,   23,   23, 0x0a,
     219,   23,   23,   23, 0x0a,
     231,   23,   23,   23, 0x0a,
     246,   23,   23,   23, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_DlgFieldFunctionEditor[] = {
    "DlgFieldFunctionEditor\0\0bool\0"
    "idxMember,strMsg\0sig_actExecuteCommand(int,QString)\0"
    "iMin,iMax,iVal\0sig_actSetProgress(int,int,int)\0"
    "indexMember,lstActions\0"
    "actPopulateList(int,QStringList)\0"
    "actRunAll()\0actRunNext()\0actStop()\0"
    "actDelete()\0actDeleteAll()\0actOpenFile()\0"
};

const QMetaObject DlgFieldFunctionEditor::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_DlgFieldFunctionEditor,
      qt_meta_data_DlgFieldFunctionEditor, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &DlgFieldFunctionEditor::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *DlgFieldFunctionEditor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *DlgFieldFunctionEditor::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DlgFieldFunctionEditor))
        return static_cast<void*>(const_cast< DlgFieldFunctionEditor*>(this));
    return QDialog::qt_metacast(_clname);
}

int DlgFieldFunctionEditor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: { bool _r = sig_actExecuteCommand((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 1: sig_actSetProgress((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 2: actPopulateList((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const QStringList(*)>(_a[2]))); break;
        case 3: actRunAll(); break;
        case 4: actRunNext(); break;
        case 5: actStop(); break;
        case 6: actDelete(); break;
        case 7: actDeleteAll(); break;
        case 8: actOpenFile(); break;
        default: ;
        }
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
bool DlgFieldFunctionEditor::sig_actExecuteCommand(int _t1, QString _t2)
{
    bool _t0;
    void *_a[] = { const_cast<void*>(reinterpret_cast<const void*>(&_t0)), const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
    return _t0;
}

// SIGNAL 1
void DlgFieldFunctionEditor::sig_actSetProgress(int _t1, int _t2, int _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
