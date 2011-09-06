/****************************************************************************
** Meta object code from reading C++ file 'CBlobTreeNetwork.h'
**
** Created: Sun May 15 21:24:30 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../CBlobTreeNetwork.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CBlobTreeNetwork.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_PS__CMember[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      13,   12,   12,   12, 0x05,
      23,   12,   12,   12, 0x05,

 // slots: signature, parameters, type, tag, flags
      35,   12,   12,   12, 0x0a,
      61,   53,   12,   12, 0x0a,
     101,   89,   12,   12, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_PS__CMember[] = {
    "PS::CMember\0\0upgrade()\0downgrade()\0"
    "actNetConnected()\0written\0"
    "actNetBytesWritten(quint64)\0socketError\0"
    "actNetDisplayError(QAbstractSocket::SocketError)\0"
};

const QMetaObject PS::CMember::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_PS__CMember,
      qt_meta_data_PS__CMember, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &PS::CMember::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *PS::CMember::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *PS::CMember::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_PS__CMember))
        return static_cast<void*>(const_cast< CMember*>(this));
    return QObject::qt_metacast(_clname);
}

int PS::CMember::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: upgrade(); break;
        case 1: downgrade(); break;
        case 2: actNetConnected(); break;
        case 3: actNetBytesWritten((*reinterpret_cast< quint64(*)>(_a[1]))); break;
        case 4: actNetDisplayError((*reinterpret_cast< QAbstractSocket::SocketError(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void PS::CMember::upgrade()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void PS::CMember::downgrade()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}
static const uint qt_meta_data_PS__CDesignNet[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      30,   16,   15,   15, 0x05,
      60,   58,   15,   15, 0x05,

 // slots: signature, parameters, type, tag, flags
      89,   15,   15,   15, 0x0a,
     114,   15,   15,   15, 0x0a,
     149,  137,   15,   15, 0x0a,
     208,   15,  204,   15, 0x0a,
     233,   15,  204,   15, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_PS__CDesignNet[] = {
    "PS::CDesignNet\0\0member,strMsg\0"
    "sig_newMessage(int,QString)\0q\0"
    "sig_memberslist(QStringList)\0"
    "actNetAcceptConnection()\0"
    "actNetServerReadData()\0socketError\0"
    "actNetServerDisplayError(QAbstractSocket::SocketError)\0"
    "int\0actNetUpgradeToMembers()\0"
    "actNetDowngradeToPending()\0"
};

const QMetaObject PS::CDesignNet::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_PS__CDesignNet,
      qt_meta_data_PS__CDesignNet, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &PS::CDesignNet::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *PS::CDesignNet::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *PS::CDesignNet::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_PS__CDesignNet))
        return static_cast<void*>(const_cast< CDesignNet*>(this));
    return QObject::qt_metacast(_clname);
}

int PS::CDesignNet::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: sig_newMessage((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 1: sig_memberslist((*reinterpret_cast< const QStringList(*)>(_a[1]))); break;
        case 2: actNetAcceptConnection(); break;
        case 3: actNetServerReadData(); break;
        case 4: actNetServerDisplayError((*reinterpret_cast< QAbstractSocket::SocketError(*)>(_a[1]))); break;
        case 5: { int _r = actNetUpgradeToMembers();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 6: { int _r = actNetDowngradeToPending();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        default: ;
        }
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void PS::CDesignNet::sig_newMessage(int _t1, QString _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void PS::CDesignNet::sig_memberslist(const QStringList & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
