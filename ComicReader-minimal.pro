QT += core widgets network

CONFIG += c++17

TARGET = ComicReader
TEMPLATE = app

# 版本信息
VERSION = 1.0.0
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

# 包含路径
INCLUDEPATH += include/

# 源文件 - 最小版本
SOURCES += \
    src/main.cpp

# 简化的main.cpp兼容头文件
HEADERS += \
    include/ui/MainWindow.h

# 输出目录
CONFIG(debug, debug|release) {
    DESTDIR = build/debug
} else {
    DESTDIR = build/release
}

# 编译器标志
QMAKE_CXXFLAGS += -Wall -Wextra
CONFIG(release, debug|release) {
    QMAKE_CXXFLAGS += -O2
}