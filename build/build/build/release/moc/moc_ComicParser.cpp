/****************************************************************************
** Meta object code from reading C++ file 'ComicParser.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../include/core/parsers/ComicParser.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ComicParser.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.1. It"
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
struct qt_meta_tag_ZN11ComicParserE_t {};
} // unnamed namespace

template <> constexpr inline auto ComicParser::qt_create_metaobjectdata<qt_meta_tag_ZN11ComicParserE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ComicParser",
        "parseStarted",
        "",
        "parseProgress",
        "current",
        "total",
        "parseCompleted",
        "ComicInfo",
        "info",
        "parseFailed",
        "error",
        "pageLoaded",
        "pageNumber",
        "ComicPage",
        "page",
        "pageLoadFailed",
        "preloadProgress",
        "loaded",
        "preloadCompleted",
        "onRarProcessFinished",
        "exitCode",
        "QProcess::ExitStatus",
        "exitStatus",
        "onRarProcessError"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'parseStarted'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'parseProgress'
        QtMocHelpers::SignalData<void(int, int)>(3, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 }, { QMetaType::Int, 5 },
        }}),
        // Signal 'parseCompleted'
        QtMocHelpers::SignalData<void(const ComicInfo &)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 7, 8 },
        }}),
        // Signal 'parseFailed'
        QtMocHelpers::SignalData<void(const QString &)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 10 },
        }}),
        // Signal 'pageLoaded'
        QtMocHelpers::SignalData<void(int, const ComicPage &)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 12 }, { 0x80000000 | 13, 14 },
        }}),
        // Signal 'pageLoadFailed'
        QtMocHelpers::SignalData<void(int, const QString &)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 12 }, { QMetaType::QString, 10 },
        }}),
        // Signal 'preloadProgress'
        QtMocHelpers::SignalData<void(int, int)>(16, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 17 }, { QMetaType::Int, 5 },
        }}),
        // Signal 'preloadCompleted'
        QtMocHelpers::SignalData<void()>(18, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onRarProcessFinished'
        QtMocHelpers::SlotData<void(int, QProcess::ExitStatus)>(19, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 20 }, { 0x80000000 | 21, 22 },
        }}),
        // Slot 'onRarProcessError'
        QtMocHelpers::SlotData<void()>(23, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<ComicParser, qt_meta_tag_ZN11ComicParserE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject ComicParser::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11ComicParserE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11ComicParserE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN11ComicParserE_t>.metaTypes,
    nullptr
} };

void ComicParser::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ComicParser *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->parseStarted(); break;
        case 1: _t->parseProgress((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 2: _t->parseCompleted((*reinterpret_cast< std::add_pointer_t<ComicInfo>>(_a[1]))); break;
        case 3: _t->parseFailed((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->pageLoaded((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<ComicPage>>(_a[2]))); break;
        case 5: _t->pageLoadFailed((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 6: _t->preloadProgress((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 7: _t->preloadCompleted(); break;
        case 8: _t->onRarProcessFinished((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QProcess::ExitStatus>>(_a[2]))); break;
        case 9: _t->onRarProcessError(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (ComicParser::*)()>(_a, &ComicParser::parseStarted, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (ComicParser::*)(int , int )>(_a, &ComicParser::parseProgress, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (ComicParser::*)(const ComicInfo & )>(_a, &ComicParser::parseCompleted, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (ComicParser::*)(const QString & )>(_a, &ComicParser::parseFailed, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (ComicParser::*)(int , const ComicPage & )>(_a, &ComicParser::pageLoaded, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (ComicParser::*)(int , const QString & )>(_a, &ComicParser::pageLoadFailed, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (ComicParser::*)(int , int )>(_a, &ComicParser::preloadProgress, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (ComicParser::*)()>(_a, &ComicParser::preloadCompleted, 7))
            return;
    }
}

const QMetaObject *ComicParser::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ComicParser::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11ComicParserE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int ComicParser::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void ComicParser::parseStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void ComicParser::parseProgress(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2);
}

// SIGNAL 2
void ComicParser::parseCompleted(const ComicInfo & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void ComicParser::parseFailed(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void ComicParser::pageLoaded(int _t1, const ComicPage & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1, _t2);
}

// SIGNAL 5
void ComicParser::pageLoadFailed(int _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1, _t2);
}

// SIGNAL 6
void ComicParser::preloadProgress(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1, _t2);
}

// SIGNAL 7
void ComicParser::preloadCompleted()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}
QT_WARNING_POP
