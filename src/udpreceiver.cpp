#include "udpreceiver.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QDebug>
#include <QRegularExpression>

UdpReceiver::UdpReceiver(QObject *parent)
    : QObject(parent)
    , m_udpSocket(nullptr)
    , m_port(0)
    , m_isListening(false)
    , m_connectionTimer(nullptr)
    , m_lastDataTime(0)
    , m_isConnected(false)
{
    m_udpSocket = new QUdpSocket(this);
    connect(m_udpSocket, &QUdpSocket::readyRead,
            this, &UdpReceiver::processPendingDatagrams);
    
    // Setup connection monitoring timer
    m_connectionTimer = new QTimer(this);
    connect(m_connectionTimer, &QTimer::timeout,
            this, &UdpReceiver::checkConnectionTimeout);
    m_connectionTimer->start(1000); // Check every second
}

UdpReceiver::~UdpReceiver()
{
    stopListening();
}

bool UdpReceiver::startListening(quint16 port)
{
    if (m_isListening) {
        stopListening();
    }
    
    if (m_udpSocket->bind(QHostAddress::Any, port)) {
        m_port = port;
        m_isListening = true;
        m_lastDataTime = 0;
        m_isConnected = false;
        
        qDebug() << "UDP receiver started on port" << port;
        return true;
    } else {
        qDebug() << "Failed to bind UDP socket to port" << port;
        emit errorOccurred(QString("Failed to bind to port %1").arg(port));
        return false;
    }
}

void UdpReceiver::stopListening()
{
    if (m_isListening) {
        m_udpSocket->close();
        m_isListening = false;
        m_port = 0;
        m_isConnected = false;
        emit connectionStatusChanged(false);
        qDebug() << "UDP receiver stopped";
    }
}

bool UdpReceiver::isListening() const
{
    return m_isListening;
}

quint16 UdpReceiver::currentPort() const
{
    return m_port;
}

void UdpReceiver::processPendingDatagrams()
{
    while (m_udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_udpSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        
        m_udpSocket->readDatagram(datagram.data(), datagram.size(),
                                 &sender, &senderPort);
        
        qDebug() << "Received datagram from" << sender.toString() << ":" << senderPort;
        qDebug() << "Data:" << datagram;
        
        double latitude, longitude, altitude;
        if (parseGpsData(datagram, latitude, longitude, altitude)) {
            m_lastDataTime = QDateTime::currentMSecsSinceEpoch();
            
            if (!m_isConnected) {
                m_isConnected = true;
                emit connectionStatusChanged(true);
            }
            
            emit gpsDataReceived(latitude, longitude, altitude);
        } else {
            qDebug() << "Failed to parse GPS data:" << datagram;
            emit errorOccurred("Failed to parse GPS data");
        }
    }
}

void UdpReceiver::checkConnectionTimeout()
{
    if (!m_isListening) {
        return;
    }
    
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    
    if (m_isConnected && m_lastDataTime > 0) {
        if (currentTime - m_lastDataTime > CONNECTION_TIMEOUT_MS) {
            m_isConnected = false;
            emit connectionStatusChanged(false);
            qDebug() << "Connection timeout - no data received for" << CONNECTION_TIMEOUT_MS << "ms";
        }
    }
}

bool UdpReceiver::parseGpsData(const QByteArray &data, double &latitude, double &longitude, double &altitude)
{
    // Try different formats
    
    // 1. Try JSON format first
    if (parseJsonFormat(data, latitude, longitude, altitude)) {
        return true;
    }
    
    // 2. Try CSV format
    if (parseCSVFormat(data, latitude, longitude, altitude)) {
        return true;
    }
    
    // 3. Try NMEA format
    if (parseNMEAFormat(data, latitude, longitude, altitude)) {
        return true;
    }
    
    return false;
}

bool UdpReceiver::parseJsonFormat(const QByteArray &data, double &latitude, double &longitude, double &altitude)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        return false;
    }
    
    QJsonObject obj = doc.object();
    
    // Check for required fields
    if (!obj.contains("latitude") || !obj.contains("longitude")) {
        return false;
    }
    
    latitude = obj["latitude"].toDouble();
    longitude = obj["longitude"].toDouble();
    altitude = obj.value("altitude").toDouble(0.0); // Default to 0 if not present
    
    // Basic validation
    if (latitude < -90.0 || latitude > 90.0 || longitude < -180.0 || longitude > 180.0) {
        return false;
    }
    
    return true;
}

bool UdpReceiver::parseCSVFormat(const QByteArray &data, double &latitude, double &longitude, double &altitude)
{
    QString str = QString::fromUtf8(data).trimmed();
    QStringList parts = str.split(',');
    
    if (parts.size() < 2) {
        return false;
    }
    
    bool latOk, lonOk, altOk = true;
    latitude = parts[0].toDouble(&latOk);
    longitude = parts[1].toDouble(&lonOk);
    
    if (parts.size() >= 3) {
        altitude = parts[2].toDouble(&altOk);
    } else {
        altitude = 0.0;
    }
    
    if (!latOk || !lonOk || !altOk) {
        return false;
    }
    
    // Basic validation
    if (latitude < -90.0 || latitude > 90.0 || longitude < -180.0 || longitude > 180.0) {
        return false;
    }
    
    return true;
}

bool UdpReceiver::parseNMEAFormat(const QByteArray &data, double &latitude, double &longitude, double &altitude)
{
    QString str = QString::fromUtf8(data).trimmed();
    
    // Simple GPGGA parser
    if (!str.startsWith("$GPGGA")) {
        return false;
    }
    
    QStringList parts = str.split(',');
    if (parts.size() < 15) {
        return false;
    }
    
    // Parse latitude (field 2 and 3)
    if (parts[2].isEmpty() || parts[3].isEmpty()) {
        return false;
    }
    
    double lat = parts[2].left(2).toDouble() + parts[2].mid(2).toDouble() / 60.0;
    if (parts[3] == "S") lat = -lat;
    
    // Parse longitude (field 4 and 5)
    if (parts[4].isEmpty() || parts[5].isEmpty()) {
        return false;
    }
    
    double lon = parts[4].left(3).toDouble() + parts[4].mid(3).toDouble() / 60.0;
    if (parts[5] == "W") lon = -lon;
    
    // Parse altitude (field 9)
    double alt = 0.0;
    if (!parts[9].isEmpty()) {
        alt = parts[9].toDouble();
    }
    
    latitude = lat;
    longitude = lon;
    altitude = alt;
    
    // Basic validation
    if (latitude < -90.0 || latitude > 90.0 || longitude < -180.0 || longitude > 180.0) {
        return false;
    }
    
    return true;
}