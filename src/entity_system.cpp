#include "entity_system.h"
#include <QDebug>

// Entity implementation
Entity::Entity(EntityId id, const QString& name, QObject* parent)
    : QObject(parent), m_id(id), m_name(name.isEmpty() ? QString("Entity_%1").arg(id) : name)
{
    // Every entity starts with a transform component
    addComponent<TransformComponent>();
}

Entity::~Entity() = default;

void Entity::setName(const QString& name) {
    if (m_name != name) {
        m_name = name;
        emit nameChanged(m_name);
    }
}

QVector<Component*> Entity::getAllComponents() const {
    QVector<Component*> components;
    for (const auto& pair : m_components) {
        components.append(pair.second.get());
    }
    return components;
}

QVariantMap Entity::serialize() const {
    QVariantMap data;
    data["id"] = m_id;
    data["name"] = m_name;
    
    QVariantMap componentsData;
    for (const auto& pair : m_components) {
        Component* component = pair.second.get();
        componentsData[component->getTypeName()] = component->serialize();
    }
    data["components"] = componentsData;
    
    return data;
}

void Entity::deserialize(const QVariantMap& data) {
    m_id = data.value("id").toUInt();
    setName(data.value("name").toString());
    
    QVariantMap componentsData = data.value("components").toMap();
    
    // Clear existing components (except transform)
    auto transformIt = m_components.find(std::type_index(typeid(TransformComponent)));
    Ref<Component> transformComponent;
    if (transformIt != m_components.end()) {
        transformComponent = transformIt->second;
    }
    m_components.clear();
    if (transformComponent) {
        m_components[std::type_index(typeid(TransformComponent))] = transformComponent;
    }
    
    // Deserialize components
    for (auto it = componentsData.begin(); it != componentsData.end(); ++it) {
        const QString& typeName = it.key();
        const QVariantMap& componentData = it.value().toMap();
        
        if (typeName == "Transform") {
            auto* transform = getComponent<TransformComponent>();
            if (transform) {
                transform->deserialize(componentData);
            }
        } else if (typeName == "Mesh") {
            auto* mesh = addComponent<MeshComponent>();
            mesh->deserialize(componentData);
        } else if (typeName == "Light") {
            auto* light = addComponent<LightComponent>();
            light->deserialize(componentData);
        } else if (typeName == "Script") {
            auto* script = addComponent<ScriptComponent>();
            script->deserialize(componentData);
        }
        // Add more component types as needed
    }
}

Transform* Entity::getTransform() const {
    return getComponent<TransformComponent>() ? &getComponent<TransformComponent>()->transform : nullptr;
}

void Entity::setPosition(const QVector3D& position) {
    if (auto* transform = getTransform()) {
        transform->position = position;
    }
}

void Entity::setRotation(const QQuaternion& rotation) {
    if (auto* transform = getTransform()) {
        transform->rotation = rotation;
    }
}

void Entity::setScale(const QVector3D& scale) {
    if (auto* transform = getTransform()) {
        transform->scale = scale;
    }
}

QVector3D Entity::getPosition() const {
    if (auto* transform = getTransform()) {
        return transform->position;
    }
    return QVector3D();
}

QQuaternion Entity::getRotation() const {
    if (auto* transform = getTransform()) {
        return transform->rotation;
    }
    return QQuaternion();
}

QVector3D Entity::getScale() const {
    if (auto* transform = getTransform()) {
        return transform->scale;
    }
    return QVector3D(1, 1, 1);
}

// EntityManager implementation
EntityManager& EntityManager::instance() {
    static EntityManager instance;
    return instance;
}

Entity* EntityManager::createEntity(const QString& name) {
    EntityId id = m_nextId++;
    auto entity = CreateRef<Entity>(id, name);
    m_entities[id] = entity;
    
    qDebug() << "Created entity:" << entity->getName() << "with ID:" << id;
    emit entityCreated(entity.get());
    
    return entity.get();
}

void EntityManager::destroyEntity(EntityId id) {
    auto it = m_entities.find(id);
    if (it != m_entities.end()) {
        qDebug() << "Destroying entity with ID:" << id;
        emit entityDestroyed(id);
        m_entities.erase(it);
    }
}

Entity* EntityManager::getEntity(EntityId id) const {
    auto it = m_entities.find(id);
    return it != m_entities.end() ? it->second.get() : nullptr;
}

QVector<Entity*> EntityManager::getAllEntities() const {
    QVector<Entity*> entities;
    for (const auto& pair : m_entities) {
        entities.append(pair.second.get());
    }
    return entities;
}

void EntityManager::clear() {
    qDebug() << "Clearing all entities";
    m_entities.clear();
    m_nextId = 1;
}

QVariantMap EntityManager::serialize() const {
    QVariantMap data;
    data["nextId"] = m_nextId;
    
    QVariantList entitiesData;
    for (const auto& pair : m_entities) {
        entitiesData.append(pair.second->serialize());
    }
    data["entities"] = entitiesData;
    
    return data;
}

void EntityManager::deserialize(const QVariantMap& data) {
    clear();
    
    m_nextId = data.value("nextId", 1).toUInt();
    
    QVariantList entitiesData = data.value("entities").toList();
    for (const QVariant& entityVariant : entitiesData) {
        QVariantMap entityData = entityVariant.toMap();
        EntityId id = entityData.value("id").toUInt();
        
        auto entity = CreateRef<Entity>(id);
        entity->deserialize(entityData);
        m_entities[id] = entity;
        
        emit entityCreated(entity.get());
    }
}

#include "entity_system.moc"

