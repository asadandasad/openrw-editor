#include "scene_manager.h"
#include "gta_loader.h"
#include "math_utils.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>

SceneManager& SceneManager::instance() {
    static SceneManager instance;
    return instance;
}

void SceneManager::newScene() {
    clearScene();
    m_sceneName = "Untitled Scene";
    m_sceneDescription = "";
    
    // Create default layer
    createLayer("Default");
    
    qDebug() << "SceneManager: Created new scene";
    emit sceneChanged();
}

bool SceneManager::loadScene(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "SceneManager: Failed to open scene file:" << filePath;
        return false;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "SceneManager: Invalid scene file format";
        return false;
    }
    
    clearScene();
    deserialize(doc.object().toVariantMap());
    
    qDebug() << "SceneManager: Loaded scene from" << filePath;
    emit sceneChanged();
    return true;
}

bool SceneManager::saveScene(const QString& filePath) {
    QVariantMap sceneData = serialize();
    QJsonDocument doc = QJsonDocument::fromVariant(sceneData);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "SceneManager: Failed to save scene file:" << filePath;
        return false;
    }
    
    file.write(doc.toJson());
    file.close();
    
    qDebug() << "SceneManager: Saved scene to" << filePath;
    return true;
}

void SceneManager::clearScene() {
    clearSelection();
    EntityManager::instance().clear();
    m_layers.clear();
    m_entityLayers.clear();
    m_triggerZones.clear();
    m_missionObjectives.clear();
    
    // Reset camera
    m_cameraPosition = QVector3D(0, 0, 10);
    m_cameraTarget = QVector3D(0, 0, 0);
    m_cameraUp = QVector3D(0, 1, 0);
    
    qDebug() << "SceneManager: Cleared scene";
    emit sceneChanged();
}

Entity* SceneManager::createEntity(const QString& name) {
    Entity* entity = EntityManager::instance().createEntity(name);
    if (entity) {
        connectEntitySignals(entity);
        
        // Add to default layer
        if (m_layers.contains("Default")) {
            setEntityLayer(entity->getId(), "Default");
        }
        
        emit entityCreated(entity);
    }
    return entity;
}

void SceneManager::destroyEntity(EntityId id) {
    Entity* entity = EntityManager::instance().getEntity(id);
    if (entity) {
        disconnectEntitySignals(entity);
        
        // Remove from selection
        deselectEntity(id);
        
        // Remove from layer
        QString layer = getEntityLayer(id);
        if (!layer.isEmpty() && m_layers.contains(layer)) {
            m_layers[layer].entities.removeAll(id);
        }
        m_entityLayers.remove(id);
        
        EntityManager::instance().destroyEntity(id);
        emit entityDestroyed(id);
    }
}

Entity* SceneManager::getEntity(EntityId id) const {
    return EntityManager::instance().getEntity(id);
}

QVector<Entity*> SceneManager::getAllEntities() const {
    return EntityManager::instance().getAllEntities();
}

void SceneManager::selectEntity(EntityId id) {
    if (!m_selectedEntities.contains(id)) {
        m_selectedEntities.append(id);
        emit selectionChanged(m_selectedEntities);
    }
}

void SceneManager::deselectEntity(EntityId id) {
    if (m_selectedEntities.removeAll(id) > 0) {
        emit selectionChanged(m_selectedEntities);
    }
}

void SceneManager::clearSelection() {
    if (!m_selectedEntities.isEmpty()) {
        m_selectedEntities.clear();
        emit selectionChanged(m_selectedEntities);
    }
}

void SceneManager::selectMultiple(const QVector<EntityId>& ids) {
    m_selectedEntities = ids;
    emit selectionChanged(m_selectedEntities);
}

QVector<EntityId> SceneManager::getSelectedEntities() const {
    return m_selectedEntities;
}

Entity* SceneManager::getPrimarySelection() const {
    if (m_selectedEntities.isEmpty()) {
        return nullptr;
    }
    return getEntity(m_selectedEntities.first());
}

QVector<Entity*> SceneManager::getEntitiesInRadius(const QVector3D& center, float radius) const {
    QVector<Entity*> result;
    float radiusSquared = radius * radius;
    
    for (Entity* entity : getAllEntities()) {
        if (entity) {
            float distanceSquared = MathUtils::distanceSquared(center, entity->getPosition());
            if (distanceSquared <= radiusSquared) {
                result.append(entity);
            }
        }
    }
    
    return result;
}

QVector<Entity*> SceneManager::getEntitiesInBox(const BoundingBox& box) const {
    QVector<Entity*> result;
    
    for (Entity* entity : getAllEntities()) {
        if (entity && box.contains(entity->getPosition())) {
            result.append(entity);
        }
    }
    
    return result;
}

Entity* SceneManager::raycast(const QVector3D& origin, const QVector3D& direction, float maxDistance) const {
    Entity* closestEntity = nullptr;
    float closestDistance = maxDistance;
    
    for (Entity* entity : getAllEntities()) {
        if (!entity) continue;
        
        // Get entity's mesh component for bounding box
        MeshComponent* meshComp = entity->getComponent<MeshComponent>();
        if (!meshComp) continue;
        
        // Transform bounding box to world space
        Transform* transform = entity->getTransform();
        if (!transform) continue;
        
        QMatrix4x4 worldMatrix = transform->getMatrix();
        QVector3D worldMin = worldMatrix * meshComp->boundingBox.min;
        QVector3D worldMax = worldMatrix * meshComp->boundingBox.max;
        
        // Ensure min/max are correct
        BoundingBox worldBox(
            QVector3D(qMin(worldMin.x(), worldMax.x()), qMin(worldMin.y(), worldMax.y()), qMin(worldMin.z(), worldMax.z())),
            QVector3D(qMax(worldMin.x(), worldMax.x()), qMax(worldMin.y(), worldMax.y()), qMax(worldMin.z(), worldMax.z()))
        );
        
        float distance;
        if (MathUtils::rayIntersectsBox(origin, direction, worldBox.min, worldBox.max, distance)) {
            if (distance < closestDistance) {
                closestDistance = distance;
                closestEntity = entity;
            }
        }
    }
    
    return closestEntity;
}

void SceneManager::createLayer(const QString& name) {
    if (!m_layers.contains(name)) {
        m_layers[name] = LayerInfo();
        qDebug() << "SceneManager: Created layer:" << name;
        emit layerCreated(name);
    }
}

void SceneManager::deleteLayer(const QString& name) {
    if (name == "Default") {
        qWarning() << "SceneManager: Cannot delete default layer";
        return;
    }
    
    if (m_layers.contains(name)) {
        // Move entities to default layer
        LayerInfo& layerInfo = m_layers[name];
        for (EntityId id : layerInfo.entities) {
            setEntityLayer(id, "Default");
        }
        
        m_layers.remove(name);
        qDebug() << "SceneManager: Deleted layer:" << name;
        emit layerDeleted(name);
    }
}

void SceneManager::setEntityLayer(EntityId id, const QString& layer) {
    // Remove from old layer
    QString oldLayer = getEntityLayer(id);
    if (!oldLayer.isEmpty() && m_layers.contains(oldLayer)) {
        m_layers[oldLayer].entities.removeAll(id);
    }
    
    // Add to new layer
    if (m_layers.contains(layer)) {
        m_layers[layer].entities.append(id);
        m_entityLayers[id] = layer;
    }
}

QString SceneManager::getEntityLayer(EntityId id) const {
    return m_entityLayers.value(id, "");
}

QStringList SceneManager::getAllLayers() const {
    return m_layers.keys();
}

void SceneManager::setLayerVisible(const QString& layer, bool visible) {
    if (m_layers.contains(layer)) {
        m_layers[layer].visible = visible;
        emit layerVisibilityChanged(layer, visible);
    }
}

bool SceneManager::isLayerVisible(const QString& layer) const {
    return m_layers.value(layer).visible;
}

void SceneManager::setLayerLocked(const QString& layer, bool locked) {
    if (m_layers.contains(layer)) {
        m_layers[layer].locked = locked;
        emit layerLockChanged(layer, locked);
    }
}

bool SceneManager::isLayerLocked(const QString& layer) const {
    return m_layers.value(layer).locked;
}

void SceneManager::setGridSize(float size) {
    if (size > 0 && m_gridSize != size) {
        m_gridSize = size;
        emit sceneChanged();
    }
}

float SceneManager::getGridSize() const {
    return m_gridSize;
}

void SceneManager::setSnapToGrid(bool enabled) {
    if (m_snapToGrid != enabled) {
        m_snapToGrid = enabled;
        emit sceneChanged();
    }
}

bool SceneManager::isSnapToGrid() const {
    return m_snapToGrid;
}

QVector3D SceneManager::snapPosition(const QVector3D& position) const {
    if (m_snapToGrid) {
        return MathUtils::snapToGrid(position, m_gridSize);
    }
    return position;
}

void SceneManager::setActiveCamera(const QVector3D& position, const QVector3D& target, const QVector3D& up) {
    m_cameraPosition = position;
    m_cameraTarget = target;
    m_cameraUp = up;
    emit cameraChanged();
}

QVector3D SceneManager::getCameraPosition() const {
    return m_cameraPosition;
}

QVector3D SceneManager::getCameraTarget() const {
    return m_cameraTarget;
}

QVector3D SceneManager::getCameraUp() const {
    return m_cameraUp;
}

void SceneManager::setWireframeMode(bool enabled) {
    if (m_wireframeMode != enabled) {
        m_wireframeMode = enabled;
        emit sceneChanged();
    }
}

bool SceneManager::isWireframeMode() const {
    return m_wireframeMode;
}

void SceneManager::setShowGrid(bool enabled) {
    if (m_showGrid != enabled) {
        m_showGrid = enabled;
        emit sceneChanged();
    }
}

bool SceneManager::isShowGrid() const {
    return m_showGrid;
}

void SceneManager::setShowBoundingBoxes(bool enabled) {
    if (m_showBoundingBoxes != enabled) {
        m_showBoundingBoxes = enabled;
        emit sceneChanged();
    }
}

bool SceneManager::isShowBoundingBoxes() const {
    return m_showBoundingBoxes;
}

bool SceneManager::loadGTAMap(const QString& iplPath, const QString& idePath) {
    // This would use the file parsers to load GTA map data
    // For now, we'll create a placeholder implementation
    qDebug() << "SceneManager: Loading GTA map from" << iplPath << "and" << idePath;
    
    // TODO: Implement actual GTA map loading using IPL and IDE parsers
    // This would involve:
    // 1. Parse IDE file to get object definitions
    // 2. Parse IPL file to get object placements
    // 3. Create entities for each placed object
    // 4. Load associated DFF and TXD files
    
    return true;
}

bool SceneManager::loadDFFModel(const QString& dffPath, const QString& txdPath) {
    // This would use the DFF parser to load a model
    qDebug() << "SceneManager: Loading DFF model from" << dffPath;
    
    // TODO: Implement actual DFF model loading
    // This would involve:
    // 1. Parse DFF file to get model data
    // 2. Parse TXD file to get textures (if provided)
    // 3. Create entity with mesh component
    
    return true;
}

void SceneManager::addTriggerZone(const TriggerZone& zone) {
    m_triggerZones.append(zone);
    emit sceneChanged();
}

void SceneManager::removeTriggerZone(const QString& name) {
    for (int i = 0; i < m_triggerZones.size(); ++i) {
        if (m_triggerZones[i].name == name) {
            m_triggerZones.removeAt(i);
            emit sceneChanged();
            break;
        }
    }
}

QVector<TriggerZone> SceneManager::getTriggerZones() const {
    return m_triggerZones;
}

TriggerZone* SceneManager::getTriggerZone(const QString& name) {
    for (auto& zone : m_triggerZones) {
        if (zone.name == name) {
            return &zone;
        }
    }
    return nullptr;
}

void SceneManager::addMissionObjective(const MissionObjective& objective) {
    m_missionObjectives.append(objective);
    emit sceneChanged();
}

void SceneManager::removeMissionObjective(const QString& id) {
    for (int i = 0; i < m_missionObjectives.size(); ++i) {
        if (m_missionObjectives[i].id == id) {
            m_missionObjectives.removeAt(i);
            emit sceneChanged();
            break;
        }
    }
}

QVector<MissionObjective> SceneManager::getMissionObjectives() const {
    return m_missionObjectives;
}

MissionObjective* SceneManager::getMissionObjective(const QString& id) {
    for (auto& objective : m_missionObjectives) {
        if (objective.id == id) {
            return &objective;
        }
    }
    return nullptr;
}

QVariantMap SceneManager::serialize() const {
    QVariantMap data;
    
    data["sceneName"] = m_sceneName;
    data["sceneDescription"] = m_sceneDescription;
    data["gridSize"] = m_gridSize;
    data["snapToGrid"] = m_snapToGrid;
    
    // Serialize camera
    QVariantMap cameraData;
    cameraData["position"] = QVariant::fromValue(m_cameraPosition);
    cameraData["target"] = QVariant::fromValue(m_cameraTarget);
    cameraData["up"] = QVariant::fromValue(m_cameraUp);
    data["camera"] = cameraData;
    
    // Serialize entities
    data["entities"] = EntityManager::instance().serialize();
    
    // Serialize layers
    QVariantMap layersData;
    for (auto it = m_layers.begin(); it != m_layers.end(); ++it) {
        QVariantMap layerData;
        layerData["visible"] = it->visible;
        layerData["locked"] = it->locked;
        QVariantList entityIds;
        for (EntityId id : it->entities) {
            entityIds.append(id);
        }
        layerData["entities"] = entityIds;
        layersData[it.key()] = layerData;
    }
    data["layers"] = layersData;
    
    // Serialize trigger zones
    QVariantList triggerZonesData;
    for (const auto& zone : m_triggerZones) {
        QVariantMap zoneData;
        zoneData["name"] = zone.name;
        zoneData["type"] = static_cast<int>(zone.type);
        zoneData["transform"] = QVariant::fromValue(zone.transform);
        zoneData["size"] = QVariant::fromValue(zone.size);
        zoneData["isActive"] = zone.isActive;
        triggerZonesData.append(zoneData);
    }
    data["triggerZones"] = triggerZonesData;
    
    // Serialize mission objectives
    QVariantList objectivesData;
    for (const auto& objective : m_missionObjectives) {
        QVariantMap objData;
        objData["id"] = objective.id;
        objData["description"] = objective.description;
        objData["isCompleted"] = objective.isCompleted;
        objData["isOptional"] = objective.isOptional;
        objData["dependencies"] = QVariant::fromValue(objective.dependencies);
        objectivesData.append(objData);
    }
    data["missionObjectives"] = objectivesData;
    
    return data;
}

void SceneManager::deserialize(const QVariantMap& data) {
    m_sceneName = data.value("sceneName", "Untitled Scene").toString();
    m_sceneDescription = data.value("sceneDescription", "").toString();
    m_gridSize = data.value("gridSize", 1.0f).toFloat();
    m_snapToGrid = data.value("snapToGrid", false).toBool();
    
    // Deserialize camera
    QVariantMap cameraData = data.value("camera").toMap();
    m_cameraPosition = cameraData.value("position").value<QVector3D>();
    m_cameraTarget = cameraData.value("target").value<QVector3D>();
    m_cameraUp = cameraData.value("up").value<QVector3D>();
    
    // Deserialize entities
    EntityManager::instance().deserialize(data.value("entities").toMap());
    
    // Connect signals for all entities
    for (Entity* entity : getAllEntities()) {
        connectEntitySignals(entity);
    }
    
    // Deserialize layers
    QVariantMap layersData = data.value("layers").toMap();
    for (auto it = layersData.begin(); it != layersData.end(); ++it) {
        QVariantMap layerData = it.value().toMap();
        LayerInfo layerInfo;
        layerInfo.visible = layerData.value("visible", true).toBool();
        layerInfo.locked = layerData.value("locked", false).toBool();
        
        QVariantList entityIds = layerData.value("entities").toList();
        for (const QVariant& idVariant : entityIds) {
            EntityId id = idVariant.toUInt();
            layerInfo.entities.append(id);
            m_entityLayers[id] = it.key();
        }
        
        m_layers[it.key()] = layerInfo;
    }
    
    // Deserialize trigger zones
    QVariantList triggerZonesData = data.value("triggerZones").toList();
    for (const QVariant& zoneVariant : triggerZonesData) {
        QVariantMap zoneData = zoneVariant.toMap();
        TriggerZone zone;
        zone.name = zoneData.value("name").toString();
        zone.type = static_cast<TriggerZone::Type>(zoneData.value("type").toInt());
        zone.transform = zoneData.value("transform").value<Transform>();
        zone.size = zoneData.value("size").value<QVector3D>();
        zone.isActive = zoneData.value("isActive", true).toBool();
        m_triggerZones.append(zone);
    }
    
    // Deserialize mission objectives
    QVariantList objectivesData = data.value("missionObjectives").toList();
    for (const QVariant& objVariant : objectivesData) {
        QVariantMap objData = objVariant.toMap();
        MissionObjective objective;
        objective.id = objData.value("id").toString();
        objective.description = objData.value("description").toString();
        objective.isCompleted = objData.value("isCompleted", false).toBool();
        objective.isOptional = objData.value("isOptional", false).toBool();
        objective.dependencies = objData.value("dependencies").toStringList().toVector();
        m_missionObjectives.append(objective);
    }
}

void SceneManager::connectEntitySignals(Entity* entity) {
    if (entity) {
        connect(entity, &Entity::nameChanged, this, &SceneManager::sceneChanged);
        connect(entity, &Entity::componentAdded, this, &SceneManager::sceneChanged);
        connect(entity, &Entity::componentRemoved, this, &SceneManager::sceneChanged);
    }
}

void SceneManager::disconnectEntitySignals(Entity* entity) {
    if (entity) {
        disconnect(entity, nullptr, this, nullptr);
    }
}

#include "scene_manager.moc"

