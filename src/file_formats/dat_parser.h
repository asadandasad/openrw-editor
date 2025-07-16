#ifndef DAT_PARSER_H
#define DAT_PARSER_H

#include "types.h"
#include <QTextStream>
#include <QIODevice>

// DAT file format parser
// Handles various data files like paths, handling, etc.
class DATParser {
public:
    // Path node structure
    struct PathNode {
        uint32_t id;
        QVector3D position;
        QVector3D direction;
        float width;
        uint32_t nodeType;
        uint32_t nextNode;
        uint32_t crossRoad;
        QString name;
    };
    
    // Vehicle handling data
    struct VehicleHandling {
        QString identifier;
        float mass;
        float dragMult;
        QVector3D centerOfMass;
        uint32_t percentSubmerged;
        float tractionMult;
        float tractionLoss;
        float tractionBias;
        uint32_t transmissionData;
        float engineAcceleration;
        float engineInertia;
        uint32_t driveType;
        uint32_t engineType;
        float brakeDeceleration;
        float brakeBias;
        bool abs;
        float steeringLock;
        float suspensionForceLevel;
        float suspensionDampingLevel;
        float suspensionHighSpeedComDamp;
        float suspensionUpperLimit;
        float suspensionLowerLimit;
        float suspensionBiasBetweenFrontAndRear;
        float suspensionAntiDiveMultiplier;
        float seatOffsetDistance;
        float collisionDamageMultiplier;
        uint32_t monetaryValue;
        uint32_t modelFlags;
        uint32_t handlingFlags;
        uint32_t frontLights;
        uint32_t rearLights;
        uint32_t animGroup;
    };
    
    // Water plane structure
    struct WaterPlane {
        QVector3D corner1;
        QVector3D corner2;
        QVector3D corner3;
        QVector3D corner4;
        float level;
        uint32_t type;
    };
    
    static bool parsePathFile(QIODevice* device, QVector<PathNode>& nodes);
    static bool parseHandlingFile(QIODevice* device, QVector<VehicleHandling>& handling);
    static bool parseWaterFile(QIODevice* device, QVector<WaterPlane>& waterPlanes);
    
    static bool parsePathFromFile(const QString& filePath, QVector<PathNode>& nodes);
    static bool parseHandlingFromFile(const QString& filePath, QVector<VehicleHandling>& handling);
    static bool parseWaterFromFile(const QString& filePath, QVector<WaterPlane>& waterPlanes);
    
private:
    static QString readLine(QTextStream& stream);
    static QStringList splitLine(const QString& line);
    static bool parsePathLine(const QString& line, PathNode& node);
    static bool parseHandlingLine(const QString& line, VehicleHandling& handling);
    static bool parseWaterLine(const QString& line, WaterPlane& plane);
    
    // Binary path file parsing (for GTA3/VC)
    static bool parseBinaryPath(QDataStream& stream, QVector<PathNode>& nodes);
    static bool isBinaryPathFile(QIODevice* device);
    
    struct BinaryPathHeader {
        uint32_t numNodes;
        uint32_t numVehicleNodes;
        uint32_t numPedNodes;
        uint32_t numCarNodes;
    };
    
    struct BinaryPathNode {
        uint16_t memoryAddress;
        uint16_t unknown1;
        QVector3D position;
        uint16_t linkId;
        uint16_t areaId;
        uint16_t nodeId;
        uint8_t pathWidth;
        uint8_t nodeType;
        uint32_t flags;
    };
};

#endif // DAT_PARSER_H

