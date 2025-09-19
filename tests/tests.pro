QT += testlib core widgets network
CONFIG += testcase

# 包含主项目的头文件路径
INCLUDEPATH += ../include/

# 链接主项目的库文件（如果需要）
DEPENDPATH += ../

# 测试源文件
SOURCES += \
    main.cpp \
    unit/TestCacheManager.cpp \
    unit/TestBookmarkManager.cpp \
    unit/TestErrorHandler.cpp \
    unit/TestConfigManager.cpp

HEADERS += \
    unit/TestCacheManager.h \
    unit/TestBookmarkManager.h \
    unit/TestErrorHandler.h \
    unit/TestConfigManager.h

# 主项目的源文件（测试需要）
SOURCES += \
    ../src/core/cache/CacheManager.cpp \
    ../src/core/bookmark/BookmarkManager.cpp \
    ../src/utils/error/ErrorHandler.cpp \
    ../src/core/ConfigManager.cpp

HEADERS += \
    ../include/core/CacheManager.h \
    ../include/core/bookmark/BookmarkManager.h \
    ../include/utils/error/ErrorHandler.h \
    ../include/core/ConfigManager.h

# 目标名称
TARGET = ComicReaderTests

# 输出目录
CONFIG(debug, debug|release) {
    DESTDIR = ../build/tests/debug
    OBJECTS_DIR = ../build/tests/debug/obj
    MOC_DIR = ../build/tests/debug/moc
} else {
    DESTDIR = ../build/tests/release
    OBJECTS_DIR = ../build/tests/release/obj
    MOC_DIR = ../build/tests/release/moc
}