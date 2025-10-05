#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QGroupBox>
#include <QSpinBox>
#include <QStatusBar>
#include <QTimer>

QT_BEGIN_NAMESPACE
class QUdpSocket;
QT_END_NAMESPACE

class UdpReceiver;
class MapWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onStartListening();
    void onStopListening();
    void onGpsDataReceived(double latitude, double longitude, double altitude);
    void onConnectionStatusChanged(bool connected);
    void updateStatusBar();

private:
    void setupUI();
    void setupConnections();
    
    // UI Components
    QWidget *m_centralWidget;
    QVBoxLayout *m_mainLayout;
    
    // Control Panel
    QGroupBox *m_controlGroup;
    QHBoxLayout *m_controlLayout;
    QLabel *m_portLabel;
    QSpinBox *m_portSpinBox;
    QPushButton *m_startButton;
    QPushButton *m_stopButton;
    
    // GPS Data Display
    QGroupBox *m_gpsGroup;
    QVBoxLayout *m_gpsLayout;
    QLabel *m_latitudeLabel;
    QLabel *m_longitudeLabel;
    QLabel *m_altitudeLabel;
    QLineEdit *m_latitudeEdit;
    QLineEdit *m_longitudeEdit;
    QLineEdit *m_altitudeEdit;
    
    // Map Widget
    MapWidget *m_mapWidget;
    
    // Log Display
    QGroupBox *m_logGroup;
    QTextEdit *m_logTextEdit;
    
    // Status
    QLabel *m_statusLabel;
    QTimer *m_statusTimer;
    
    // Network
    UdpReceiver *m_udpReceiver;
    
    // Current GPS data
    double m_currentLatitude;
    double m_currentLongitude;
    double m_currentAltitude;
    bool m_isListening;
};

#endif // MAINWINDOW_H