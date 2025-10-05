cmake_minimum_required(VERSION 3.16)
project(GPSMapViewer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required Qt5 components
find_package(Qt5 5.15.3 REQUIRED COMPONENTS Core Widgets Network)

# Find QGIS
find_package(QGIS REQUIRED)

# Set up Qt5
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${QGIS_INCLUDE_DIR})
include_directories(${QGIS_INCLUDE_DIR}/qgis)

# Source files
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/udpreceiver.cpp
    src/mapwidget.cpp
)

set(HEADERS
    src/mainwindow.h
    src/udpreceiver.h
    src/mapwidget.h
)

set(UI_FILES
    src/mainwindow.ui
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${UI_FILES})

# Link libraries
target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Widgets
    Qt5::Network
    ${QGIS_CORE_LIBRARY}
    ${QGIS_GUI_LIBRARY}
)

# Install target
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# Copy QGIS plugins and resources if needed
if(WIN32)
    # Windows specific QGIS setup
    set(QGIS_PREFIX_PATH "" CACHE PATH "Path to QGIS installation")
    if(QGIS_PREFIX_PATH)
        install(DIRECTORY ${QGIS_PREFIX_PATH}/plugins DESTINATION .)
        install(DIRECTORY ${QGIS_PREFIX_PATH}/share DESTINATION .)
    endif()
endif()