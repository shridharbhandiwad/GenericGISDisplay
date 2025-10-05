#!/bin/bash

# GPS Map Viewer Build Script

set -e

echo "GPS Map Viewer - Build Script"
echo "=============================="

# Check if build directory exists
if [ -d "build" ]; then
    echo "Removing existing build directory..."
    rm -rf build
fi

# Create build directory
echo "Creating build directory..."
mkdir build
cd build

# Check for required dependencies
echo "Checking dependencies..."

# Check for Qt5
if ! pkg-config --exists Qt5Core Qt5Widgets Qt5Network; then
    echo "Error: Qt5 development packages not found!"
    echo "Please install Qt5 development packages:"
    echo "  Ubuntu/Debian: sudo apt-get install qtbase5-dev qttools5-dev"
    echo "  CentOS/RHEL: sudo yum install qt5-qtbase-devel qt5-qttools-devel"
    echo "  Arch: sudo pacman -S qt5-base qt5-tools"
    exit 1
fi

# Check for QGIS development packages
if ! pkg-config --exists qgis; then
    echo "Warning: QGIS development packages not found!"
    echo "The application may not compile without QGIS development headers."
    echo "To install QGIS development packages:"
    echo "  Ubuntu/Debian: sudo apt-get install libqgis-dev qgis-dev"
    echo "  For other distributions, please install QGIS development packages"
    echo ""
    echo "Continuing anyway..."
fi

# Configure with CMake
echo "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
echo "Building..."
make -j$(nproc)

echo ""
echo "Build completed successfully!"
echo "Executable: build/GPSMapViewer"
echo ""
echo "To run the application:"
echo "  cd build && ./GPSMapViewer"
echo ""
echo "To test with simulated GPS data:"
echo "  python3 ../test_sender.py --simulate"