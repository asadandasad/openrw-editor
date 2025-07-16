#ifndef TYPES_H
#define TYPES_H

#include <QString>
#include <QVector3D>
#include <QQuaternion>
#include <QMatrix4x4>
#include <QVector>
#include <QMap>
#include <memory>

// Forward declarations
class Entity;
class Component;

// Type aliases for clarity
using EntityId = uint32_t;
using ComponentId = uint32_t;

// Common data structures
struct Transform {
    QVector3D position{0.0f, 0.0f, 0.0f};
    QQuaternion rotation{1.0f, 0.0f, 0.0f, 0.0f}; // w, x, y, z
    QVector3D scale{1.0f, 1.0f, 1.0f};
    
    QMatrix4x4 getMatrix() const {
        QMatrix4x4 matrix;
        matrix.translate(position);
        matrix.rotate(rotation);
        matrix.scale(scale);
        return matrix;
    }
};

struct BoundingBox {
    QVector3D min;
    QVector3D max;
    
    BoundingBox() : min(0, 0, 0), max(0, 0, 0) {}
    BoundingBox(const QVector3D& minPoint, const QVector3D& maxPoint) 
        : min(minPoint), max(maxPoint) {}
    
    QVector3D center() const { return (min + max) * 0.5f; }
    QVector3D size() const { return max - min; }
    bool contains(const QVector3D& point) const {
        return point.x() >= min.x() && point.x() <= max.x() &&
               point.y() >= min.y() && point.y() <= max.y() &&
               point.z() >= min.z() && point.z() <= max.z();
    }
};

// GTA-specific data structures
struct GTAVertex {
    QVector3D position;
    QVector3D normal;
    QVector2D texCoord;
    uint32_t color;
};

struct GTAMaterial {
    QString name;
    QString textureName;
    QVector3D ambient{0.5f, 0.5f, 0.5f};
    QVector3D diffuse{1.0f, 1.0f, 1.0f};
    QVector3D specular{0.0f, 0.0f, 0.0f};
    float shininess{0.0f};
};

struct GTAMesh {
    QString name;
    QVector<GTAVertex> vertices;
    QVector<uint32_t> indices;
    GTAMaterial material;
    BoundingBox boundingBox;
};

struct GTAModel {
    QString name;
    QVector<GTAMesh> meshes;
    BoundingBox boundingBox;
};

// IDE file structures
struct IDEObject {
    uint32_t id;
    QString modelName;
    QString textureName;
    uint32_t meshCount;
    float drawDistance;
    uint32_t flags;
};

// IPL file structures
struct IPLInstance {
    uint32_t id;
    QString modelName;
    Transform transform;
    uint32_t interior;
    uint32_t lod;
};

// Mission-related structures
struct TriggerZone {
    enum Type { Box, Sphere, Cylinder };
    Type type;
    Transform transform;
    QVector3D size; // For box: width, height, depth; For sphere: radius in x component
    QString name;
    bool isActive;
};

struct MissionObjective {
    QString id;
    QString description;
    bool isCompleted;
    bool isOptional;
    QVector<QString> dependencies; // IDs of objectives that must be completed first
};

// Component types
enum class ComponentType {
    Transform,
    Mesh,
    Light,
    Script,
    Trigger,
    Vehicle,
    NPC,
    Prop,
    Sound
};

// Smart pointer aliases
template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T>
using WeakRef = std::weak_ptr<T>;

template<typename T, typename... Args>
Ref<T> CreateRef(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

#endif // TYPES_H

