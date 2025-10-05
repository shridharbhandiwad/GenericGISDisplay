# GPS Map Viewer

A Qt5.15.3 GUI application that receives GPS coordinates (latitude, longitude, altitude) via UDP and displays the position on a global map using QGIS plugins.

## Features

- **UDP GPS Data Reception**: Receives GPS data via UDP in multiple formats (JSON, CSV, NMEA)
- **Real-time Map Display**: Shows GPS position on an interactive map using QGIS
- **Multiple Base Maps**: Support for OpenStreetMap and satellite imagery
- **GPS Trail Tracking**: Optional trail display showing GPS movement history
- **Multiple Data Formats**: Supports JSON, CSV, and NMEA GPS data formats
- **Modern UI**: Clean, dark-themed interface with real-time status updates

## Requirements

### System Dependencies

- **Qt 5.15.3** or compatible version
- **QGIS 3.x** with development headers
- **CMake 3.16** or later
- **C++17** compatible compiler

### Ubuntu/Debian Installation

```bash
# Install Qt5 development packages
sudo apt-get update
sudo apt-get install qtbase5-dev qttools5-dev qt5-qmake

# Install QGIS development packages
sudo apt-get install libqgis-dev qgis-dev qgis

# Install build tools
sudo apt-get install cmake build-essential
```

### CentOS/RHEL Installation

```bash
# Install Qt5 development packages
sudo yum install qt5-qtbase-devel qt5-qttools-devel

# Install QGIS (may require EPEL repository)
sudo yum install epel-release
sudo yum install qgis qgis-devel

# Install build tools
sudo yum install cmake gcc-c++
```

## Building

1. **Clone or extract the project**:
   ```bash
   cd /path/to/gps-map-viewer
   ```

2. **Run the build script**:
   ```bash
   ./build.sh
   ```

   Or build manually:
   ```bash
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release
   make -j$(nproc)
   ```

## Usage

### Running the Application

1. **Start the GPS Map Viewer**:
   ```bash
   cd build
   ./GPSMapViewer
   ```

2. **Configure UDP Settings**:
   - Set the UDP port (default: 12345)
   - Click "Start Listening" to begin receiving GPS data

3. **Send GPS Data**:
   Use the included test sender or your own UDP client to send GPS data.

### Testing with Simulated Data

The project includes a Python test sender for simulation:

```bash
# Send static GPS data (JSON format)
python3 test_sender.py

# Send moving GPS data with simulation
python3 test_sender.py --simulate --interval 0.5

# Send data in CSV format
python3 test_sender.py --format csv

# Send data in NMEA format
python3 test_sender.py --format nmea --simulate

# Send to different host/port
python3 test_sender.py --host 192.168.1.100 --port 54321
```

## GPS Data Formats

The application supports multiple GPS data formats:

### JSON Format
```json
{
  "latitude": 40.712800,
  "longitude": -74.006000,
  "altitude": 10.5,
  "timestamp": "2024-01-01T12:00:00",
  "accuracy": 3.5,
  "speed": 0.0,
  "heading": 0.0
}
```

### CSV Format
```
40.712800,-74.006000,10.5
```

### NMEA Format (GPGGA)
```
$GPGGA,120000,4042.7680,N,07400.3600,W,1,08,1.0,10.5,M,46.9,M,,*47
```

## Application Components

### Main Window (`mainwindow.h/cpp`)
- Main application interface
- UDP control panel
- GPS data display
- Map integration
- Logging system

### UDP Receiver (`udpreceiver.h/cpp`)
- UDP socket management
- Multi-format GPS data parsing
- Connection monitoring
- Data validation

### Map Widget (`mapwidget.h/cpp`)
- QGIS map canvas integration
- Base map layers (OpenStreetMap, Satellite)
- GPS position marker
- Trail tracking
- Map controls (zoom, pan, center)

### Main Application (`main.cpp`)
- QGIS initialization
- Application setup
- Theme configuration

## Map Features

- **Interactive Map**: Pan, zoom, and navigate the map
- **Base Map Options**: Choose between OpenStreetMap, satellite imagery, or no base map
- **GPS Position Marker**: Red marker showing current GPS position
- **Trail Display**: Blue dots showing GPS movement history
- **Auto-centering**: Automatically centers on first GPS position received
- **Coordinate Systems**: Supports WGS84 (EPSG:4326) input with Web Mercator (EPSG:3857) display

## Troubleshooting

### Common Issues

1. **QGIS not found during build**:
   - Ensure QGIS development packages are installed
   - Check that QGIS is in standard installation paths

2. **Qt5 version mismatch**:
   - Verify Qt5.15.3 or compatible version is installed
   - Use `qmake --version` to check Qt version

3. **Map not displaying**:
   - Check internet connection for base map tiles
   - Verify QGIS plugins are properly loaded

4. **UDP data not received**:
   - Check firewall settings
   - Verify correct port configuration
   - Test with the included Python sender

### Debug Information

The application provides debug output to help troubleshoot issues:

```bash
# Run with debug output
QT_LOGGING_RULES="*.debug=true" ./GPSMapViewer
```

## Development

### Project Structure
```
├── CMakeLists.txt          # Build configuration
├── build.sh               # Build script
├── test_sender.py         # UDP test sender
├── README.md             # This file
└── src/
    ├── main.cpp          # Application entry point
    ├── mainwindow.h/cpp  # Main window
    ├── mainwindow.ui     # UI layout
    ├── udpreceiver.h/cpp # UDP receiver
    └── mapwidget.h/cpp   # QGIS map widget
```

### Adding New Features

1. **New GPS Data Format**: Extend `UdpReceiver::parseGpsData()`
2. **Additional Map Layers**: Modify `MapWidget::addBaseMap()`
3. **UI Enhancements**: Update `MainWindow::setupUI()`

## License

This project is provided as-is for educational and development purposes.

## Contributing

Feel free to submit issues, feature requests, or pull requests to improve the application.

---

**Note**: This application requires QGIS to be properly installed and configured on your system. The map functionality depends on QGIS libraries and may require internet connectivity for base map tiles.