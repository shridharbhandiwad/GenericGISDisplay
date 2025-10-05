QT += core widgets network

CONFIG += c++17

TARGET = gps-map-viewer
TEMPLATE = app

# Define application version
VERSION = 1.0.0
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

# Source files
SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/mapwidget.cpp \
    src/udpreceiver.cpp

# Header files
HEADERS += \
    src/mainwindow.h \
    src/mapwidget.h \
    src/udpreceiver.h

# UI files
FORMS += \
    src/mainwindow.ui

# Include paths for source directory
INCLUDEPATH += src

# QGIS Configuration
# Note: Adjust paths according to your QGIS installation
unix {
    # Common QGIS paths on Linux
    QGIS_PREFIX = /usr
    exists(/usr/include/qgis) {
        QGIS_PREFIX = /usr
    } else:exists(/usr/local/include/qgis) {
        QGIS_PREFIX = /usr/local
    }
    
    INCLUDEPATH += $$QGIS_PREFIX/include/qgis
    LIBS += -L$$QGIS_PREFIX/lib
    
    # QGIS libraries
    LIBS += -lqgis_core \
            -lqgis_gui \
            -lqgis_app
}

win32 {
    # Windows QGIS configuration
    # Adjust QGIS_ROOT to your QGIS installation path
    QGIS_ROOT = C:/Program Files/QGIS 3.28
    
    INCLUDEPATH += "$$QGIS_ROOT/include"
    LIBS += -L"$$QGIS_ROOT/lib"
    
    # QGIS libraries for Windows
    LIBS += -lqgis_core \
            -lqgis_gui \
            -lqgis_app
}

macx {
    # macOS QGIS configuration
    QGIS_ROOT = /Applications/QGIS.app/Contents/MacOS
    
    INCLUDEPATH += $$QGIS_ROOT/include
    LIBS += -L$$QGIS_ROOT/lib
    
    # QGIS libraries for macOS
    LIBS += -lqgis_core \
            -lqgis_gui \
            -lqgis_app
}

# Additional defines
DEFINES += QT_DEPRECATED_WARNINGS

# Compiler flags
QMAKE_CXXFLAGS += -Wall -Wextra

# Install configuration
unix:!android {
    target.path = /usr/local/bin
    INSTALLS += target
}

# Debug/Release configuration
CONFIG(debug, debug|release) {
    DEFINES += DEBUG_BUILD
    TARGET = $$TARGET-debug
}

CONFIG(release, debug|release) {
    DEFINES += RELEASE_BUILD
}

# Resource files (if any in the future)
# RESOURCES += resources.qrc

# Deployment
win32 {
    RC_ICONS = icon.ico
}

macx {
    ICON = icon.icns
}