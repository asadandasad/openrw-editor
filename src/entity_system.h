#ifndef ENTITY_SYSTEM_H
#define ENTITY_SYSTEM_H

#include "types.h"
#include <QObject>
#include <QVariant>
#include <typeinfo>
#include <typeindex>

// Base component class
class Component {
public:
    Component() = default;
    virtual ~Component() = default;
    virtual ComponentType getType() const = 0;
    virtual QString getTypeName() const = 0;
    virtual QVariantMap serialize() const = 0;
    virtual void deserialize(const QVariantMap& data) = 0;
};

// Transform component
class TransformComponent : public Component {
public:
    Transform transform;
    
    TransformComponent() = default;
    TransformComponent(const Transform& t) : transform(t) {}
    
    ComponentType getType() const override { return ComponentType::Transform; }
    QString getTypeName() const override { return "Transform"; }
    
    QVariantMap serialize() const override {
        QVariantMap data;
        data["position"] = QVariant::fromValue(transform.position);
        data["rotation"] = QVariant::fromValue(transform.rotation);
        data["scale"] = QVariant::fromValue(transform.scale);
        return data;
    }
    
    void deserialize(const QVariantMap& data) override {
        if (data.contains("position"))
            transform.position = data["position"].value<QVector3D>();
        if (data.contains("rotation"))
            transform.rotation = data["rotation"].value<QQuaternion>();
        if (data.contains("scale"))
            transform.scale = data["scale"].value<QVector3D>();
    }
};

// Mesh component
class MeshComponent : public Component {
public:
    QString meshPath;
    QString materialPath;
    bool isVisible = true;
    BoundingBox boundingBox;
    
    ComponentType getType() const override { return ComponentType::Mesh; }
    QString getTypeName() const override { return "Mesh"; }
    
    QVariantMap serialize() const override {
        QVariantMap data;
        data["meshPath"] = meshPath;
        data["materialPath"] = materialPath;
        data["isVisible"] = isVisible;
        data["boundingBox_min"] = QVariant::fromValue(boundingBox.min);
        data["boundingBox_max"] = QVariant::fromValue(boundingBox.max);
        return data;
    }
    
    void deserialize(const QVariantMap& data) override {
        meshPath = data.value("meshPath").toString();
        materialPath = data.value("materialPath").toString();
        isVisible = data.value("isVisible", true).toBool();
        boundingBox.min = data.value("boundingBox_min").value<QVector3D>();
        boundingBox.max = data.value("boundingBox_max").value<QVector3D>();
    }
};

// Light component
class LightComponent : public Component {
public:
    enum LightType { Directional, Point, Spot };
    
    LightType lightType = Point;
    QVector3D color{1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;
    float range = 10.0f;
    float spotAngle = 45.0f; // For spot lights
    bool castShadows = true;
    
    ComponentType getType() const override { return ComponentType::Light; }
    QString getTypeName() const override { return "Light"; }
    
    QVariantMap serialize() const override {
        QVariantMap data;
        data["lightType"] = static_cast<int>(lightType);
        data["color"] = QVariant::fromValue(color);
        data["intensity"] = intensity;
        data["range"] = range;
        data["spotAngle"] = spotAngle;
        data["castShadows"] = castShadows;
        return data;
    }
    
    void deserialize(const QVariantMap& data) override {
        lightType = static_cast<LightType>(data.value("lightType", 0).toInt());
        color = data.value("color").value<QVector3D>();
        intensity = data.value("intensity", 1.0f).toFloat();
        range = data.value("range", 10.0f).toFloat();
        spotAngle = data.value("spotAngle", 45.0f).toFloat();
        castShadows = data.value("castShadows", true).toBool();
    }
};

// Script component
class ScriptComponent : public Component {
public:
    QString scriptPath;
    QVariantMap scriptProperties;
    
    ComponentType getType() const override { return ComponentType::Script; }
    QString getTypeName() const override { return "Script"; }
    
    QVariantMap serialize() const override {
        QVariantMap data;
        data["scriptPath"] = scriptPath;
        data["scriptProperties"] = scriptProperties;
        return data;
    }
    
    void deserialize(const QVariantMap& data) override {
        scriptPath = data.value("scriptPath").toString();
        scriptProperties = data.value("scriptProperties").toMap();
    }
};

// Entity class
class Entity : public QObject {
    Q_OBJECT
    
public:
    explicit Entity(EntityId id, const QString& name = "", QObject* parent = nullptr);
    ~Entity();
    
    EntityId getId() const { return m_id; }
    QString getName() const { return m_name; }
    void setName(const QString& name);
    
    // Component management
    template<typename T>
    T* addComponent() {
        static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
        auto component = CreateRef<T>();
        m_components[std::type_index(typeid(T))] = component;
        emit componentAdded(component.get());
        return component.get();
    }
    
    template<typename T>
    T* getComponent() const {
        static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
        auto it = m_components.find(std::type_index(typeid(T)));
        if (it != m_components.end()) {
            return static_cast<T*>(it->second.get());
        }
        return nullptr;
    }
    
    template<typename T>
    bool hasComponent() const {
        static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
        return m_components.find(std::type_index(typeid(T))) != m_components.end();
    }
    
    template<typename T>
    void removeComponent() {
        static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
        auto it = m_components.find(std::type_index(typeid(T)));
        if (it != m_components.end()) {
            emit componentRemoved(it->second.get());
            m_components.erase(it);
        }
    }
    
    QVector<Component*> getAllComponents() const;
    
    // Serialization
    QVariantMap serialize() const;
    void deserialize(const QVariantMap& data);
    
    // Transform convenience methods
    Transform* getTransform() const;
    void setPosition(const QVector3D& position);
    void setRotation(const QQuaternion& rotation);
    void setScale(const QVector3D& scale);
    QVector3D getPosition() const;
    QQuaternion getRotation() const;
    QVector3D getScale() const;
    
signals:
    void componentAdded(Component* component);
    void componentRemoved(Component* component);
    void nameChanged(const QString& newName);
    
private:
    EntityId m_id;
    QString m_name;
    std::map<std::type_index, Ref<Component>> m_components;
};

// Entity manager
class EntityManager : public QObject {
    Q_OBJECT
    
public:
    static EntityManager& instance();
    
    Entity* createEntity(const QString& name = "");
    void destroyEntity(EntityId id);
    Entity* getEntity(EntityId id) const;
    QVector<Entity*> getAllEntities() const;
    
    void clear();
    
    // Serialization
    QVariantMap serialize() const;
    void deserialize(const QVariantMap& data);
    
signals:
    void entityCreated(Entity* entity);
    void entityDestroyed(EntityId id);
    
private:
    EntityManager() = default;
    Q_DISABLE_COPY(EntityManager)
    
    EntityId m_nextId = 1;
    QMap<EntityId, Ref<Entity>> m_entities;
};

#endif // ENTITY_SYSTEM_H

