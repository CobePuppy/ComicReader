QT += core widgets network

CONFIG += c++17

# 检查 WebEngine 是否可用
qtHaveModule(webenginewidgets) {
    QT += webenginewidgets webenginecore
    message("WebEngine available")
} else {
    message("WebEngine not available, using simplified browser")
    DEFINES += NO_WEBENGINE
}e widgets network webenginewidgets webenginecore

CONFIG += c++17

# 检查 WebEngine 是否可用
!qtHaveModule(webenginewidgets) {
    message("WebEngine not available, using QTextBrowser instead")
    DEFINES += NO_WEBENGINE
}

TARGET = ComicReader
TEMPLATE = app

# 定义版本信息
VERSION = 1.0.0
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

# 包含路径
INCLUDEPATH += include/

# 源文件
SOURCES += \
    src/main.cpp \
    src/ui/mainwindow/MainWindow.cpp \
    src/ui/reader/ReaderWidget.cpp \
    src/ui/browser/BrowserWidget.cpp \
    src/ui/downloadmanager/DownloadManagerWidget.cpp \
    src/ui/settings/SettingsWidget.cpp \
    src/core/network/NetworkManager.cpp \
    src/core/config/ConfigManager.cpp \
    src/utils/FileUtils.cpp

# 头文件
HEADERS += \
    include/ui/MainWindow.h \
    include/ui/ReaderWidget.h \
    include/ui/BrowserWidget.h \
    include/ui/DownloadManagerWidget.h \
    include/ui/SettingsWidget.h \
    include/core/NetworkManager.h \
    include/core/ConfigManager.h \
    include/utils/FileUtils.h

# 资源文件
RESOURCES += resources/resources.qrc

# Windows 特定设置
win32 {
    RC_ICONS = resources/icons/app.ico
    QMAKE_TARGET_COMPANY = "ComicReader Team"
    QMAKE_TARGET_DESCRIPTION = "ComicReader - A Powerful Comic Reader"
    QMAKE_TARGET_COPYRIGHT = "Copyright (c) 2025 ComicReader Team"
    QMAKE_TARGET_PRODUCT = "ComicReader"
}

# macOS 特定设置
macx {
    ICON = resources/icons/app.icns
    QMAKE_INFO_PLIST = Info.plist
}

# Linux 特定设置
unix:!macx {
    target.path = /usr/local/bin
    INSTALLS += target
    
    desktop.files = ComicReader.desktop
    desktop.path = /usr/share/applications
    INSTALLS += desktop
    
    icon.files = resources/icons/app.png
    icon.path = /usr/share/pixmaps
    INSTALLS += icon
}

# 输出目录
CONFIG(debug, debug|release) {
    DESTDIR = build/debug
    OBJECTS_DIR = build/debug/obj
    MOC_DIR = build/debug/moc
    RCC_DIR = build/debug/rcc
    UI_DIR = build/debug/ui
} else {
    DESTDIR = build/release
    OBJECTS_DIR = build/release/obj
    MOC_DIR = build/release/moc
    RCC_DIR = build/release/rcc
    UI_DIR = build/release/ui
}

# 编译器标志
QMAKE_CXXFLAGS += -Wall -Wextra
CONFIG(debug, debug|release) {
    QMAKE_CXXFLAGS += -g -O0
    DEFINES += DEBUG
} else {
    QMAKE_CXXFLAGS += -O2
    DEFINES += QT_NO_DEBUG_OUTPUT
}

# 清理规则
QMAKE_CLEAN += -r build/
