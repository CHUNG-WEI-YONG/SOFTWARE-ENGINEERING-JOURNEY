/****************************************************************************
** Meta object code from reading C++ file 'httpmgr.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../full-stack-project/cpp_project/WhatsApp_like_app/qtui/httpmgr.h"
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'httpmgr.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN7HttpmgrE_t {};
} // unnamed namespace

template <> constexpr inline auto Httpmgr::qt_create_metaobjectdata<qt_meta_tag_ZN7HttpmgrE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Httpmgr",
        "sig_http_finish",
        "",
        "ReqId",
        "id",
        "res",
        "ErrorCode",
        "error",
        "Modules",
        "mod",
        "sig_reg_mod_finish",
        "slot_http_finish"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'sig_http_finish'
        QtMocHelpers::SignalData<void(ReqId, QString, ErrorCode, Modules)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 }, { QMetaType::QString, 5 }, { 0x80000000 | 6, 7 }, { 0x80000000 | 8, 9 },
        }}),
        // Signal 'sig_reg_mod_finish'
        QtMocHelpers::SignalData<void(ReqId, QString, ErrorCode)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 }, { QMetaType::QString, 5 }, { 0x80000000 | 6, 7 },
        }}),
        // Slot 'slot_http_finish'
        QtMocHelpers::SlotData<void(ReqId, QString, ErrorCode, Modules)>(11, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 3, 4 }, { QMetaType::QString, 5 }, { 0x80000000 | 6, 7 }, { 0x80000000 | 8, 9 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<Httpmgr, qt_meta_tag_ZN7HttpmgrE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Httpmgr::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7HttpmgrE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7HttpmgrE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7HttpmgrE_t>.metaTypes,
    nullptr
} };

void Httpmgr::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<Httpmgr *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->sig_http_finish((*reinterpret_cast<std::add_pointer_t<ReqId>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<ErrorCode>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<Modules>>(_a[4]))); break;
        case 1: _t->sig_reg_mod_finish((*reinterpret_cast<std::add_pointer_t<ReqId>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<ErrorCode>>(_a[3]))); break;
        case 2: _t->slot_http_finish((*reinterpret_cast<std::add_pointer_t<ReqId>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<ErrorCode>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<Modules>>(_a[4]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (Httpmgr::*)(ReqId , QString , ErrorCode , Modules )>(_a, &Httpmgr::sig_http_finish, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (Httpmgr::*)(ReqId , QString , ErrorCode )>(_a, &Httpmgr::sig_reg_mod_finish, 1))
            return;
    }
}

const QMetaObject *Httpmgr::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Httpmgr::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7HttpmgrE_t>.strings))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "Singleton<Httpmgr>"))
        return static_cast< Singleton<Httpmgr>*>(this);
    if (!strcmp(_clname, "std::enable_shared_from_this<Httpmgr>"))
        return static_cast< std::enable_shared_from_this<Httpmgr>*>(this);
    return QObject::qt_metacast(_clname);
}

int Httpmgr::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void Httpmgr::sig_http_finish(ReqId _t1, QString _t2, ErrorCode _t3, Modules _t4)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2, _t3, _t4);
}

// SIGNAL 1
void Httpmgr::sig_reg_mod_finish(ReqId _t1, QString _t2, ErrorCode _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2, _t3);
}
QT_WARNING_POP
