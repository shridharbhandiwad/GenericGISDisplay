#ifndef UDPRECEIVER_H
#define UDPRECEIVER_H

#include <QObject>
#include <QUdpSocket>
#include <QTimer>
#include <QHostAddress>

class UdpReceiver : public QObject
{
    Q_OBJECT

public:
    explicit UdpReceiver(QObject *parent = nullptr);
    ~UdpReceiver();

    bool startListening(quint16 port);
    void stopListening();
    bool isListening() const;
    quint16 currentPort() const;

signals:
    void gpsDataReceived(double latitude, double longitude, double altitude);
    void connectionStatusChanged(bool connected);
    void errorOccurred(const QString &error);

private slots:
    void processPendingDatagrams();
    void checkConnectionTimeout();

private:
    bool parseGpsData(const QByteArray &data, double &latitude, double &longitude, double &altitude);
    bool parseJsonFormat(const QByteArray &data, double &latitude, double &longitude, double &altitude);
    bool parseCSVFormat(const QByteArray &data, double &latitude, double &longitude, double &altitude);
    bool parseNMEAFormat(const QByteArray &data, double &latitude, double &longitude, double &altitude);
    
    QUdpSocket *m_udpSocket;
    quint16 m_port;
    bool m_isListening;
    
    // Connection monitoring
    QTimer *m_connectionTimer;
    qint64 m_lastDataTime;
    static const int CONNECTION_TIMEOUT_MS = 5000; // 5 seconds
    bool m_isConnected;
};

#endif // UDPRECEIVER_H