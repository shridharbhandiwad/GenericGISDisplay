#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QSlider>
#include <QCheckBox>

// QGIS includes
#include <qgsmapcanvas.h>
#include <qgsmaplayer.h>
#include <qgsvectorlayer.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsproject.h>
#include <qgsgeometry.h>
#include <qgsfeature.h>
#include <qgssymbol.h>
#include <qgsrenderer.h>
#include <qgssingleSymbolRenderer.h>
#include <qgsfillsymbol.h>
#include <qgsmarkersymbol.h>
#include <qgsrectangle.h>
#include <qgscoordinatereferencesystem.h>
#include <qgscoordinatetransform.h>
#include <qgsmaprendererparalleljob.h>
#include <qgsmessagelog.h>

class QgsMapCanvas;
class QgsVectorLayer;
class QgsMarkerSymbol;

class MapWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MapWidget(QWidget *parent = nullptr);
    ~MapWidget();

    void updatePosition(double latitude, double longitude, double altitude);
    void zoomToPosition();
    void addBaseMap();

private slots:
    void onZoomIn();
    void onZoomOut();
    void onZoomToFit();
    void onCenterOnPosition();
    void onBaseMapChanged(const QString &baseMapType);
    void onShowTrailToggled(bool show);
    void onClearTrail();

private:
    void setupUI();
    void setupMapCanvas();
    void initializeQGIS();
    void createPositionLayer();
    void createTrailLayer();
    void addOpenStreetMapLayer();
    void addSatelliteLayer();
    void updatePositionMarker();
    void addTrailPoint();
    
    // UI Components
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_controlLayout;
    QPushButton *m_zoomInButton;
    QPushButton *m_zoomOutButton;
    QPushButton *m_zoomToFitButton;
    QPushButton *m_centerButton;
    QLabel *m_baseMapLabel;
    QComboBox *m_baseMapCombo;
    QCheckBox *m_showTrailCheckBox;
    QPushButton *m_clearTrailButton;
    
    // QGIS Components
    QgsMapCanvas *m_mapCanvas;
    QgsVectorLayer *m_positionLayer;
    QgsVectorLayer *m_trailLayer;
    QgsVectorLayer *m_baseMapLayer;
    
    // Current position
    double m_currentLatitude;
    double m_currentLongitude;
    double m_currentAltitude;
    bool m_hasPosition;
    
    // Trail tracking
    QList<QgsPointXY> m_trailPoints;
    bool m_showTrail;
    
    // Map settings
    QgsCoordinateReferenceSystem m_mapCrs;
    static const int ZOOM_LEVEL_DEFAULT = 15;
};

#endif // MAPWIDGET_H