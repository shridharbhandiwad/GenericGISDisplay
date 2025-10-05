#include "mapwidget.h"

#include <QDebug>
#include <QMessageBox>
#include <QDir>
#include <QStandardPaths>

// Additional QGIS includes
#include <qgspoint.h>
#include <qgspointxy.h>
#include <qgsfields.h>
#include <qgsfield.h>
#include <qgsdatadefinedsizelegend.h>
#include <qgsmapthemecollection.h>
#include <qgslayertreemodel.h>
#include <qgslayertreeview.h>
#include <qgsrasterlayer.h>
#include <qgsmaptopixel.h>

MapWidget::MapWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_controlLayout(nullptr)
    , m_zoomInButton(nullptr)
    , m_zoomOutButton(nullptr)
    , m_zoomToFitButton(nullptr)
    , m_centerButton(nullptr)
    , m_baseMapLabel(nullptr)
    , m_baseMapCombo(nullptr)
    , m_showTrailCheckBox(nullptr)
    , m_clearTrailButton(nullptr)
    , m_mapCanvas(nullptr)
    , m_positionLayer(nullptr)
    , m_trailLayer(nullptr)
    , m_baseMapLayer(nullptr)
    , m_currentLatitude(0.0)
    , m_currentLongitude(0.0)
    , m_currentAltitude(0.0)
    , m_hasPosition(false)
    , m_showTrail(true)
    , m_mapCrs(QgsCoordinateReferenceSystem("EPSG:3857")) // Web Mercator
{
    initializeQGIS();
    setupUI();
    setupMapCanvas();
    createPositionLayer();
    createTrailLayer();
    addBaseMap();
}

MapWidget::~MapWidget()
{
    // Cleanup is handled by Qt parent-child relationship
}

void MapWidget::initializeQGIS()
{
    // Initialize QGIS Application
    // Note: This should ideally be done once in main(), but we'll do it here for simplicity
    static bool qgisInitialized = false;
    if (!qgisInitialized) {
        // Set QGIS paths
        QString qgisPrefix = QDir::homePath() + "/.local/share/QGIS/QGIS3";
        if (!QDir(qgisPrefix).exists()) {
            qgisPrefix = "/usr/share/qgis"; // Default Linux path
        }
        
        QgsApplication::setPrefixPath(qgisPrefix, true);
        QgsApplication::initQgis();
        
        qgisInitialized = true;
        qDebug() << "QGIS initialized with prefix:" << qgisPrefix;
    }
}

void MapWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // Control panel
    m_controlLayout = new QHBoxLayout();
    
    m_zoomInButton = new QPushButton("Zoom In", this);
    m_zoomOutButton = new QPushButton("Zoom Out", this);
    m_zoomToFitButton = new QPushButton("Zoom to Fit", this);
    m_centerButton = new QPushButton("Center on Position", this);
    
    m_baseMapLabel = new QLabel("Base Map:", this);
    m_baseMapCombo = new QComboBox(this);
    m_baseMapCombo->addItem("OpenStreetMap");
    m_baseMapCombo->addItem("Satellite");
    m_baseMapCombo->addItem("None");
    
    m_showTrailCheckBox = new QCheckBox("Show Trail", this);
    m_showTrailCheckBox->setChecked(true);
    
    m_clearTrailButton = new QPushButton("Clear Trail", this);
    
    m_controlLayout->addWidget(m_zoomInButton);
    m_controlLayout->addWidget(m_zoomOutButton);
    m_controlLayout->addWidget(m_zoomToFitButton);
    m_controlLayout->addWidget(m_centerButton);
    m_controlLayout->addWidget(m_baseMapLabel);
    m_controlLayout->addWidget(m_baseMapCombo);
    m_controlLayout->addWidget(m_showTrailCheckBox);
    m_controlLayout->addWidget(m_clearTrailButton);
    m_controlLayout->addStretch();
    
    m_mainLayout->addLayout(m_controlLayout);
    
    // Connect signals
    connect(m_zoomInButton, &QPushButton::clicked, this, &MapWidget::onZoomIn);
    connect(m_zoomOutButton, &QPushButton::clicked, this, &MapWidget::onZoomOut);
    connect(m_zoomToFitButton, &QPushButton::clicked, this, &MapWidget::onZoomToFit);
    connect(m_centerButton, &QPushButton::clicked, this, &MapWidget::onCenterOnPosition);
    connect(m_baseMapCombo, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            this, &MapWidget::onBaseMapChanged);
    connect(m_showTrailCheckBox, &QCheckBox::toggled, this, &MapWidget::onShowTrailToggled);
    connect(m_clearTrailButton, &QPushButton::clicked, this, &MapWidget::onClearTrail);
}

void MapWidget::setupMapCanvas()
{
    m_mapCanvas = new QgsMapCanvas(this);
    m_mapCanvas->setCanvasColor(QColor(255, 255, 255));
    m_mapCanvas->enableAntiAliasing(true);
    m_mapCanvas->setDestinationCrs(m_mapCrs);
    
    // Set initial extent (world view)
    QgsRectangle extent(-20037508.34, -20037508.34, 20037508.34, 20037508.34);
    m_mapCanvas->setExtent(extent);
    
    m_mainLayout->addWidget(m_mapCanvas);
    
    qDebug() << "Map canvas created";
}

void MapWidget::createPositionLayer()
{
    // Create memory layer for current position
    QString layerDef = "Point?crs=EPSG:4326&field=id:integer&field=name:string(20)";
    m_positionLayer = new QgsVectorLayer(layerDef, "Current Position", "memory");
    
    if (!m_positionLayer->isValid()) {
        qDebug() << "Failed to create position layer";
        return;
    }
    
    // Create marker symbol
    QgsMarkerSymbol *symbol = QgsMarkerSymbol::createSimple(QVariantMap());
    symbol->setColor(QColor(255, 0, 0)); // Red
    symbol->setSize(8);
    
    QgsSingleSymbolRenderer *renderer = new QgsSingleSymbolRenderer(symbol);
    m_positionLayer->setRenderer(renderer);
    
    // Add to project and canvas
    QgsProject::instance()->addMapLayer(m_positionLayer);
    
    qDebug() << "Position layer created";
}

void MapWidget::createTrailLayer()
{
    // Create memory layer for trail
    QString layerDef = "Point?crs=EPSG:4326&field=id:integer&field=timestamp:string(20)";
    m_trailLayer = new QgsVectorLayer(layerDef, "GPS Trail", "memory");
    
    if (!m_trailLayer->isValid()) {
        qDebug() << "Failed to create trail layer";
        return;
    }
    
    // Create marker symbol for trail points
    QgsMarkerSymbol *symbol = QgsMarkerSymbol::createSimple(QVariantMap());
    symbol->setColor(QColor(0, 0, 255)); // Blue
    symbol->setSize(4);
    
    QgsSingleSymbolRenderer *renderer = new QgsSingleSymbolRenderer(symbol);
    m_trailLayer->setRenderer(renderer);
    
    // Add to project and canvas
    QgsProject::instance()->addMapLayer(m_trailLayer);
    
    qDebug() << "Trail layer created";
}

void MapWidget::addBaseMap()
{
    // For now, we'll create a simple base map
    // In a full implementation, you would add actual tile layers
    addOpenStreetMapLayer();
}

void MapWidget::addOpenStreetMapLayer()
{
    // Create a raster layer for OpenStreetMap
    // Note: This is a simplified implementation
    // In reality, you'd need to configure tile server connections
    
    QString uri = "type=xyz&url=https://tile.openstreetmap.org/{z}/{x}/{y}.png&zmax=19&zmin=0";
    m_baseMapLayer = new QgsRasterLayer(uri, "OpenStreetMap", "wms");
    
    if (m_baseMapLayer->isValid()) {
        QgsProject::instance()->addMapLayer(m_baseMapLayer);
        qDebug() << "OpenStreetMap layer added";
    } else {
        qDebug() << "Failed to create OpenStreetMap layer";
        // Create a fallback empty layer
        delete m_baseMapLayer;
        m_baseMapLayer = nullptr;
    }
    
    updateMapLayers();
}

void MapWidget::addSatelliteLayer()
{
    // Remove existing base map
    if (m_baseMapLayer) {
        QgsProject::instance()->removeMapLayer(m_baseMapLayer);
        m_baseMapLayer = nullptr;
    }
    
    // Add satellite imagery (example with Esri World Imagery)
    QString uri = "type=xyz&url=https://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/{z}/{y}/{x}&zmax=19&zmin=0";
    m_baseMapLayer = new QgsRasterLayer(uri, "Satellite", "wms");
    
    if (m_baseMapLayer->isValid()) {
        QgsProject::instance()->addMapLayer(m_baseMapLayer);
        qDebug() << "Satellite layer added";
    } else {
        qDebug() << "Failed to create satellite layer";
        delete m_baseMapLayer;
        m_baseMapLayer = nullptr;
    }
    
    updateMapLayers();
}

void MapWidget::updateMapLayers()
{
    QList<QgsMapLayer*> layers;
    
    // Add layers in order (bottom to top)
    if (m_baseMapLayer) {
        layers.append(m_baseMapLayer);
    }
    if (m_trailLayer && m_showTrail) {
        layers.append(m_trailLayer);
    }
    if (m_positionLayer) {
        layers.append(m_positionLayer);
    }
    
    m_mapCanvas->setLayers(layers);
    m_mapCanvas->refresh();
}

void MapWidget::updatePosition(double latitude, double longitude, double altitude)
{
    m_currentLatitude = latitude;
    m_currentLongitude = longitude;
    m_currentAltitude = altitude;
    m_hasPosition = true;
    
    updatePositionMarker();
    
    if (m_showTrail) {
        addTrailPoint();
    }
    
    // Auto-center on first position
    static bool firstPosition = true;
    if (firstPosition) {
        zoomToPosition();
        firstPosition = false;
    }
    
    qDebug() << "Position updated:" << latitude << longitude << altitude;
}

void MapWidget::updatePositionMarker()
{
    if (!m_positionLayer || !m_hasPosition) {
        return;
    }
    
    // Clear existing features
    m_positionLayer->dataProvider()->truncate();
    
    // Create new feature
    QgsFeature feature;
    feature.setGeometry(QgsGeometry::fromPointXY(QgsPointXY(m_currentLongitude, m_currentLatitude)));
    
    QgsFields fields = m_positionLayer->fields();
    feature.setFields(fields);
    feature.setAttribute("id", 1);
    feature.setAttribute("name", "Current Position");
    
    // Add feature
    m_positionLayer->dataProvider()->addFeatures(QgsFeatureList() << feature);
    m_positionLayer->updateExtents();
    m_positionLayer->triggerRepaint();
    
    m_mapCanvas->refresh();
}

void MapWidget::addTrailPoint()
{
    if (!m_trailLayer || !m_hasPosition) {
        return;
    }
    
    QgsPointXY point(m_currentLongitude, m_currentLatitude);
    m_trailPoints.append(point);
    
    // Create new feature
    QgsFeature feature;
    feature.setGeometry(QgsGeometry::fromPointXY(point));
    
    QgsFields fields = m_trailLayer->fields();
    feature.setFields(fields);
    feature.setAttribute("id", m_trailPoints.size());
    feature.setAttribute("timestamp", QDateTime::currentDateTime().toString());
    
    // Add feature
    m_trailLayer->dataProvider()->addFeatures(QgsFeatureList() << feature);
    m_trailLayer->updateExtents();
    m_trailLayer->triggerRepaint();
    
    m_mapCanvas->refresh();
}

void MapWidget::zoomToPosition()
{
    if (!m_hasPosition) {
        return;
    }
    
    // Transform coordinates to map CRS
    QgsCoordinateTransform transform(QgsCoordinateReferenceSystem("EPSG:4326"), m_mapCrs, QgsProject::instance());
    QgsPointXY mapPoint = transform.transform(QgsPointXY(m_currentLongitude, m_currentLatitude));
    
    // Create extent around the point
    double buffer = 1000; // 1km buffer in map units
    QgsRectangle extent(mapPoint.x() - buffer, mapPoint.y() - buffer,
                       mapPoint.x() + buffer, mapPoint.y() + buffer);
    
    m_mapCanvas->setExtent(extent);
    m_mapCanvas->refresh();
}

void MapWidget::onZoomIn()
{
    m_mapCanvas->zoomIn();
}

void MapWidget::onZoomOut()
{
    m_mapCanvas->zoomOut();
}

void MapWidget::onZoomToFit()
{
    if (m_positionLayer) {
        m_mapCanvas->zoomToFeatureExtent(m_positionLayer->extent());
    }
}

void MapWidget::onCenterOnPosition()
{
    zoomToPosition();
}

void MapWidget::onBaseMapChanged(const QString &baseMapType)
{
    if (baseMapType == "OpenStreetMap") {
        addOpenStreetMapLayer();
    } else if (baseMapType == "Satellite") {
        addSatelliteLayer();
    } else if (baseMapType == "None") {
        if (m_baseMapLayer) {
            QgsProject::instance()->removeMapLayer(m_baseMapLayer);
            m_baseMapLayer = nullptr;
            updateMapLayers();
        }
    }
}

void MapWidget::onShowTrailToggled(bool show)
{
    m_showTrail = show;
    updateMapLayers();
}

void MapWidget::onClearTrail()
{
    if (m_trailLayer) {
        m_trailLayer->dataProvider()->truncate();
        m_trailLayer->updateExtents();
        m_trailLayer->triggerRepaint();
        m_trailPoints.clear();
        m_mapCanvas->refresh();
    }
}