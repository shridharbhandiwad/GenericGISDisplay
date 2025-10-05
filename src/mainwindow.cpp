#include "mainwindow.h"
#include "udpreceiver.h"
#include "mapwidget.h"

#include <QApplication>
#include <QMessageBox>
#include <QDateTime>
#include <QSplitter>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_mainLayout(nullptr)
    , m_controlGroup(nullptr)
    , m_controlLayout(nullptr)
    , m_portLabel(nullptr)
    , m_portSpinBox(nullptr)
    , m_startButton(nullptr)
    , m_stopButton(nullptr)
    , m_gpsGroup(nullptr)
    , m_gpsLayout(nullptr)
    , m_latitudeLabel(nullptr)
    , m_longitudeLabel(nullptr)
    , m_altitudeLabel(nullptr)
    , m_latitudeEdit(nullptr)
    , m_longitudeEdit(nullptr)
    , m_altitudeEdit(nullptr)
    , m_mapWidget(nullptr)
    , m_logGroup(nullptr)
    , m_logTextEdit(nullptr)
    , m_statusLabel(nullptr)
    , m_statusTimer(nullptr)
    , m_udpReceiver(nullptr)
    , m_currentLatitude(0.0)
    , m_currentLongitude(0.0)
    , m_currentAltitude(0.0)
    , m_isListening(false)
{
    setupUI();
    setupConnections();
    
    // Initialize UDP receiver
    m_udpReceiver = new UdpReceiver(this);
    connect(m_udpReceiver, &UdpReceiver::gpsDataReceived,
            this, &MainWindow::onGpsDataReceived);
    connect(m_udpReceiver, &UdpReceiver::connectionStatusChanged,
            this, &MainWindow::onConnectionStatusChanged);
    
    // Setup status timer
    m_statusTimer = new QTimer(this);
    connect(m_statusTimer, &QTimer::timeout, this, &MainWindow::updateStatusBar);
    m_statusTimer->start(1000); // Update every second
    
    setWindowTitle("GPS Map Viewer - UDP Receiver");
    resize(1200, 800);
    
    updateStatusBar();
}

MainWindow::~MainWindow()
{
    if (m_udpReceiver && m_isListening) {
        m_udpReceiver->stopListening();
    }
}

void MainWindow::setupUI()
{
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    
    // Control Panel
    m_controlGroup = new QGroupBox("UDP Control", this);
    m_controlLayout = new QHBoxLayout(m_controlGroup);
    
    m_portLabel = new QLabel("Port:", this);
    m_portSpinBox = new QSpinBox(this);
    m_portSpinBox->setRange(1024, 65535);
    m_portSpinBox->setValue(12345);
    
    m_startButton = new QPushButton("Start Listening", this);
    m_stopButton = new QPushButton("Stop Listening", this);
    m_stopButton->setEnabled(false);
    
    m_controlLayout->addWidget(m_portLabel);
    m_controlLayout->addWidget(m_portSpinBox);
    m_controlLayout->addWidget(m_startButton);
    m_controlLayout->addWidget(m_stopButton);
    m_controlLayout->addStretch();
    
    m_mainLayout->addWidget(m_controlGroup);
    
    // Create splitter for GPS data and map
    QSplitter *topSplitter = new QSplitter(Qt::Horizontal, this);
    
    // GPS Data Display
    m_gpsGroup = new QGroupBox("GPS Data", this);
    m_gpsLayout = new QVBoxLayout(m_gpsGroup);
    
    m_latitudeLabel = new QLabel("Latitude:", this);
    m_latitudeEdit = new QLineEdit(this);
    m_latitudeEdit->setReadOnly(true);
    m_latitudeEdit->setText("0.000000");
    
    m_longitudeLabel = new QLabel("Longitude:", this);
    m_longitudeEdit = new QLineEdit(this);
    m_longitudeEdit->setReadOnly(true);
    m_longitudeEdit->setText("0.000000");
    
    m_altitudeLabel = new QLabel("Altitude (m):", this);
    m_altitudeEdit = new QLineEdit(this);
    m_altitudeEdit->setReadOnly(true);
    m_altitudeEdit->setText("0.00");
    
    m_gpsLayout->addWidget(m_latitudeLabel);
    m_gpsLayout->addWidget(m_latitudeEdit);
    m_gpsLayout->addWidget(m_longitudeLabel);
    m_gpsLayout->addWidget(m_longitudeEdit);
    m_gpsLayout->addWidget(m_altitudeLabel);
    m_gpsLayout->addWidget(m_altitudeEdit);
    m_gpsLayout->addStretch();
    
    m_gpsGroup->setMaximumWidth(250);
    topSplitter->addWidget(m_gpsGroup);
    
    // Map Widget
    m_mapWidget = new MapWidget(this);
    topSplitter->addWidget(m_mapWidget);
    
    topSplitter->setSizes({250, 800});
    m_mainLayout->addWidget(topSplitter);
    
    // Log Display
    m_logGroup = new QGroupBox("Log", this);
    QVBoxLayout *logLayout = new QVBoxLayout(m_logGroup);
    
    m_logTextEdit = new QTextEdit(this);
    m_logTextEdit->setMaximumHeight(150);
    m_logTextEdit->setReadOnly(true);
    logLayout->addWidget(m_logTextEdit);
    
    m_mainLayout->addWidget(m_logGroup);
    
    // Status Bar
    m_statusLabel = new QLabel("Ready", this);
    statusBar()->addWidget(m_statusLabel);
}

void MainWindow::setupConnections()
{
    connect(m_startButton, &QPushButton::clicked, this, &MainWindow::onStartListening);
    connect(m_stopButton, &QPushButton::clicked, this, &MainWindow::onStopListening);
}

void MainWindow::onStartListening()
{
    int port = m_portSpinBox->value();
    
    if (m_udpReceiver->startListening(port)) {
        m_isListening = true;
        m_startButton->setEnabled(false);
        m_stopButton->setEnabled(true);
        m_portSpinBox->setEnabled(false);
        
        QString message = QString("Started listening on UDP port %1").arg(port);
        m_logTextEdit->append(QString("[%1] %2")
                             .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                             .arg(message));
    } else {
        QMessageBox::warning(this, "Error", 
                           QString("Failed to start UDP listener on port %1").arg(port));
    }
}

void MainWindow::onStopListening()
{
    if (m_udpReceiver) {
        m_udpReceiver->stopListening();
        m_isListening = false;
        m_startButton->setEnabled(true);
        m_stopButton->setEnabled(false);
        m_portSpinBox->setEnabled(true);
        
        QString message = "Stopped UDP listener";
        m_logTextEdit->append(QString("[%1] %2")
                             .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                             .arg(message));
    }
}

void MainWindow::onGpsDataReceived(double latitude, double longitude, double altitude)
{
    m_currentLatitude = latitude;
    m_currentLongitude = longitude;
    m_currentAltitude = altitude;
    
    // Update GPS data display
    m_latitudeEdit->setText(QString::number(latitude, 'f', 6));
    m_longitudeEdit->setText(QString::number(longitude, 'f', 6));
    m_altitudeEdit->setText(QString::number(altitude, 'f', 2));
    
    // Update map
    m_mapWidget->updatePosition(latitude, longitude, altitude);
    
    // Log the data
    QString message = QString("GPS: Lat=%1, Lon=%2, Alt=%3m")
                     .arg(latitude, 0, 'f', 6)
                     .arg(longitude, 0, 'f', 6)
                     .arg(altitude, 0, 'f', 2);
    
    m_logTextEdit->append(QString("[%1] %2")
                         .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                         .arg(message));
    
    // Auto-scroll to bottom
    m_logTextEdit->moveCursor(QTextCursor::End);
}

void MainWindow::onConnectionStatusChanged(bool connected)
{
    if (connected) {
        m_statusLabel->setText("Connected - Receiving GPS data");
        m_statusLabel->setStyleSheet("color: green;");
    } else {
        m_statusLabel->setText(m_isListening ? "Listening - No data" : "Disconnected");
        m_statusLabel->setStyleSheet("color: orange;");
    }
}

void MainWindow::updateStatusBar()
{
    if (m_isListening) {
        QString status = QString("Listening on port %1 | GPS: %2, %3 | Alt: %4m")
                        .arg(m_portSpinBox->value())
                        .arg(m_currentLatitude, 0, 'f', 6)
                        .arg(m_currentLongitude, 0, 'f', 6)
                        .arg(m_currentAltitude, 0, 'f', 2);
        setWindowTitle(QString("GPS Map Viewer - %1").arg(status));
    } else {
        setWindowTitle("GPS Map Viewer - Not listening");
    }
}