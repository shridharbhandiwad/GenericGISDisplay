#include <QApplication>
#include <QDir>
#include <QDebug>
#include <QMessageBox>
#include <QStyleFactory>
#include <QStandardPaths>

// QGIS includes
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsmessagelog.h>

#include "mainwindow.h"

void setupQGISEnvironment()
{
    // Set up QGIS paths
    QString qgisPrefix;
    
    // Try different common QGIS installation paths
    QStringList possiblePaths = {
        "/usr/share/qgis",
        "/usr/local/share/qgis",
        QDir::homePath() + "/.local/share/QGIS/QGIS3",
        "/opt/qgis/share/qgis"
    };
    
    for (const QString &path : possiblePaths) {
        if (QDir(path).exists()) {
            qgisPrefix = path;
            break;
        }
    }
    
    if (qgisPrefix.isEmpty()) {
        qDebug() << "Warning: QGIS installation not found in standard locations";
        qgisPrefix = "/usr/share/qgis"; // Default fallback
    }
    
    // Set QGIS application paths
    QgsApplication::setPrefixPath(qgisPrefix, true);
    
    // Set plugin path
    QString pluginPath = qgisPrefix + "/plugins";
    QgsApplication::setPluginPath(pluginPath);
    
    // Set package data path
    QString pkgDataPath = qgisPrefix;
    QgsApplication::setPkgDataPath(pkgDataPath);
    
    qDebug() << "QGIS Prefix Path:" << qgisPrefix;
    qDebug() << "QGIS Plugin Path:" << pluginPath;
    qDebug() << "QGIS Package Data Path:" << pkgDataPath;
}

int main(int argc, char *argv[])
{
    // Create QgsApplication instead of QApplication for QGIS support
    QgsApplication app(argc, argv, true);
    
    // Set application properties
    app.setApplicationName("GPS Map Viewer");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("GPS Map Viewer");
    app.setOrganizationDomain("gps-map-viewer.local");
    
    // Setup QGIS environment
    setupQGISEnvironment();
    
    // Initialize QGIS
    QgsApplication::initQgis();
    
    // Check if QGIS was initialized properly
    QgsProviderRegistry *providerRegistry = QgsProviderRegistry::instance();
    if (!providerRegistry) {
        QMessageBox::critical(nullptr, "Error", 
                             "Failed to initialize QGIS provider registry.\n"
                             "Please ensure QGIS is properly installed.");
        return -1;
    }
    
    qDebug() << "Available data providers:" << providerRegistry->providerList();
    
    // Set application style
    app.setStyle(QStyleFactory::create("Fusion"));
    
    // Apply dark theme
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    app.setPalette(darkPalette);
    
    // Create and show main window
    MainWindow window;
    window.show();
    
    qDebug() << "GPS Map Viewer started successfully";
    
    // Run application
    int result = app.exec();
    
    // Cleanup QGIS
    QgsApplication::exitQgis();
    
    return result;
}