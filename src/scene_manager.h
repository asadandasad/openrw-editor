#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include "types.h"
#include "entity_system.h"
#include <QObject>
#include <QVector>
#include <QMap>

// Scene manager handles the 3D world and all entities within it
class SceneManager : public QObject {
    Q_OBJECT
    
public:
    static SceneManager& instance();
    
    // Scene management
    void newScene();
    bool loadScene(const QString& filePath);
    bool saveScene(const QString& filePath);
    void clearScene();
    
    // Entity management
    Entity* createEntity(const QString& name = "");
    void destroyEntity(EntityId id);
    Entity* getEntity(EntityId id) const;
    QVector<Entity*> getAllEntities() const;
    
    // Selection management
    void selectEntity(EntityId id);
    void deselectEntity(EntityId id);
    void clearSelection();
    void selectMultiple(const QVector<EntityId>& ids);
    QVector<EntityId> getSelectedEntities() const;
    Entity* getPrimarySelection() const;
    
    // Spatial queries
    QVector<Entity*> getEntitiesInRadius(const QVector3D& center, float radius) const;
    QVector<Entity*> getEntitiesInBox(const BoundingBox& box) const;
    Entity* raycast(const QVector3D& origin, const QVector3D& direction, float maxDistance = 1000.0f) const;
    
    // Layer management
    void createLayer(const QString& name);
    void deleteLayer(const QString& name);
    void setEntityLayer(EntityId id, const QString& layer);
    QString getEntityLayer(EntityId id) const;
    QStringList getAllLayers() const;
    void setLayerVisible(const QString& layer, bool visible);
    bool isLayerVisible(const QString& layer) const;
    void setLayerLocked(const QString& layer, bool locked);
    bool isLayerLocked(const QString& layer) const;
    
    // Grid and snapping
    void setGridSize(float size);
    float getGridSize() const;
    void setSnapToGrid(bool enabled);
    bool isSnapToGrid() const;
    QVector3D snapPosition(const QVector3D& position) const;
    
    // Camera management
    void setActiveCamera(const QVector3D& position, const QVector3D& target, const QVector3D& up);
    QVector3D getCameraPosition() const;
    QVector3D getCameraTarget() const;
    QVector3D getCameraUp() const;
    
    // Rendering settings
    void setWireframeMode(bool enabled);
    bool isWireframeMode() const;
    void setShowGrid(bool enabled);
    bool isShowGrid() const;
    void setShowBoundingBoxes(bool enabled);
    bool isShowBoundingBoxes() const;
    
    // Asset loading
    bool loadGTAMap(const QString& iplPath, const QString& idePath);
    bool loadDFFModel(const QString& dffPath, const QString& txdPath = "");
    
    // Mission data
    void addTriggerZone(const TriggerZone& zone);
    void removeTriggerZone(const QString& name);
    QVector<TriggerZone> getTriggerZones() const;
    TriggerZone* getTriggerZone(const QString& name);
    
    void addMissionObjective(const MissionObjective& objective);
    void removeMissionObjective(const QString& id);
    QVector<MissionObjective> getMissionObjectives() const;
    MissionObjective* getMissionObjective(const QString& id);
    
    // Serialization
    QVariantMap serialize() const;
    void deserialize(const QVariantMap& data);
    
signals:
    void entityCreated(Entity* entity);
    void entityDestroyed(EntityId id);
    void selectionChanged(const QVector<EntityId>& selectedIds);
    void layerCreated(const QString& name);
    void layerDeleted(const QString& name);
    void layerVisibilityChanged(const QString& name, bool visible);
    void layerLockChanged(const QString& name, bool locked);
    void sceneChanged();
    void cameraChanged();
    
private:
    SceneManager() = default;
    Q_DISABLE_COPY(SceneManager)
    
    void connectEntitySignals(Entity* entity);
    void disconnectEntitySignals(Entity* entity);
    
    // Selection state
    QVector<EntityId> m_selectedEntities;
    
    // Layer management
    struct LayerInfo {
        bool visible = true;
        bool locked = false;
        QVector<EntityId> entities;
    };
    QMap<QString, LayerInfo> m_layers;
    QMap<EntityId, QString> m_entityLayers;
    
    // Grid and snapping
    float m_gridSize = 1.0f;
    bool m_snapToGrid = false;
    
    // Camera state
    QVector3D m_cameraPosition{0, 0, 10};
    QVector3D m_cameraTarget{0, 0, 0};
    QVector3D m_cameraUp{0, 1, 0};
    
    // Rendering settings
    bool m_wireframeMode = false;
    bool m_showGrid = true;
    bool m_showBoundingBoxes = false;
    
    // Mission data
    QVector<TriggerZone> m_triggerZones;
    QVector<MissionObjective> m_missionObjectives;
    
    // Scene metadata
    QString m_sceneName;
    QString m_sceneDescription;
};

#endif // SCENE_MANAGER_H

