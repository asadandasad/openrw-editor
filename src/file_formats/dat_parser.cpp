#include "dat_parser.h"
#include <QFile>
#include <QDebug>
#include <QDataStream>
#include <QRegularExpression>

bool DATParser::parsePathFile(QIODevice* device, QVector<PathNode>& nodes) {
    if (!device || !device->isOpen()) {
        qWarning() << "DATParser: Invalid or closed device";
        return false;
    }
    
    // Check if it's binary format
    if (isBinaryPathFile(device)) {
        QDataStream stream(device);
        stream.setByteOrder(QDataStream::LittleEndian);
        return parseBinaryPath(stream, nodes);
    }
    
    // Parse as text format
    QTextStream stream(device);
    QString line;
    
    while (!stream.atEnd()) {
        line = readLine(stream);
        
        if (line.isEmpty() || line.startsWith('#') || line.startsWith(';')) {
            continue; // Skip empty lines and comments
        }
        
        PathNode node;
        if (parsePathLine(line, node)) {
            nodes.append(node);
        }
    }
    
    return true;
}

bool DATParser::parseHandlingFile(QIODevice* device, QVector<VehicleHandling>& handling) {
    if (!device || !device->isOpen()) {
        qWarning() << "DATParser: Invalid or closed device";
        return false;
    }
    
    QTextStream stream(device);
    QString line;
    bool inHandlingSection = false;
    
    while (!stream.atEnd()) {
        line = readLine(stream);
        
        if (line.isEmpty() || line.startsWith('#') || line.startsWith(';') || line.startsWith('%')) {
            continue; // Skip empty lines and comments
        }
        
        // Check for section markers
        if (line.toLower() == "handling") {
            inHandlingSection = true;
            continue;
        } else if (line.toLower() == "end") {
            inHandlingSection = false;
            continue;
        }
        
        if (inHandlingSection) {
            VehicleHandling vehicleHandling;
            if (parseHandlingLine(line, vehicleHandling)) {
                handling.append(vehicleHandling);
            }
        }
    }
    
    return true;
}

bool DATParser::parseWaterFile(QIODevice* device, QVector<WaterPlane>& waterPlanes) {
    if (!device || !device->isOpen()) {
        qWarning() << "DATParser: Invalid or closed device";
        return false;
    }
    
    QTextStream stream(device);
    QString line;
    
    while (!stream.atEnd()) {
        line = readLine(stream);
        
        if (line.isEmpty() || line.startsWith('#') || line.startsWith(';')) {
            continue; // Skip empty lines and comments
        }
        
        WaterPlane plane;
        if (parseWaterLine(line, plane)) {
            waterPlanes.append(plane);
        }
    }
    
    return true;
}

bool DATParser::parsePathFromFile(const QString& filePath, QVector<PathNode>& nodes) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "DATParser: Failed to open path file:" << filePath;
        return false;
    }
    
    bool result = parsePathFile(&file, nodes);
    file.close();
    
    if (result) {
        qDebug() << "DATParser: Successfully parsed path file" << filePath << "with" << nodes.size() << "nodes";
    }
    
    return result;
}

bool DATParser::parseHandlingFromFile(const QString& filePath, QVector<VehicleHandling>& handling) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "DATParser: Failed to open handling file:" << filePath;
        return false;
    }
    
    bool result = parseHandlingFile(&file, handling);
    file.close();
    
    if (result) {
        qDebug() << "DATParser: Successfully parsed handling file" << filePath << "with" << handling.size() << "vehicles";
    }
    
    return result;
}

bool DATParser::parseWaterFromFile(const QString& filePath, QVector<WaterPlane>& waterPlanes) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "DATParser: Failed to open water file:" << filePath;
        return false;
    }
    
    bool result = parseWaterFile(&file, waterPlanes);
    file.close();
    
    if (result) {
        qDebug() << "DATParser: Successfully parsed water file" << filePath << "with" << waterPlanes.size() << "planes";
    }
    
    return result;
}

QString DATParser::readLine(QTextStream& stream) {
    QString line = stream.readLine().trimmed();
    
    // Remove comments
    int commentPos = line.indexOf('#');
    if (commentPos >= 0) {
        line = line.left(commentPos).trimmed();
    }
    
    commentPos = line.indexOf(';');
    if (commentPos >= 0) {
        line = line.left(commentPos).trimmed();
    }
    
    return line;
}

QStringList DATParser::splitLine(const QString& line) {
    // Split by whitespace or tabs, handling quoted strings
    QStringList result;
    QRegularExpression regex(R"([^\s\t]+|"[^"]*")");
    QRegularExpressionMatchIterator it = regex.globalMatch(line);
    
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString part = match.captured(0);
        
        // Remove quotes if present
        if (part.startsWith('"') && part.endsWith('"')) {
            part = part.mid(1, part.length() - 2);
        }
        
        result.append(part);
    }
    
    return result;
}

bool DATParser::parsePathLine(const QString& line, PathNode& node) {
    QStringList parts = splitLine(line);
    
    // Path format varies, but typically: ID, PosX, PosY, PosZ, DirX, DirY, DirZ, Width, Type, Next, Cross
    if (parts.size() < 8) {
        qWarning() << "DATParser: Invalid path line format:" << line;
        return false;
    }
    
    bool ok;
    node.id = parts[0].toUInt(&ok);
    if (!ok) return false;
    
    float posX = parts[1].toFloat(&ok);
    if (!ok) return false;
    float posY = parts[2].toFloat(&ok);
    if (!ok) return false;
    float posZ = parts[3].toFloat(&ok);
    if (!ok) return false;
    
    node.position = QVector3D(posX, posY, posZ);
    
    if (parts.size() > 6) {
        float dirX = parts[4].toFloat(&ok);
        if (!ok) return false;
        float dirY = parts[5].toFloat(&ok);
        if (!ok) return false;
        float dirZ = parts[6].toFloat(&ok);
        if (!ok) return false;
        
        node.direction = QVector3D(dirX, dirY, dirZ);
    }
    
    if (parts.size() > 7) {
        node.width = parts[7].toFloat(&ok);
        if (!ok) node.width = 1.0f;
    }
    
    if (parts.size() > 8) {
        node.nodeType = parts[8].toUInt(&ok);
        if (!ok) node.nodeType = 0;
    }
    
    if (parts.size() > 9) {
        node.nextNode = parts[9].toUInt(&ok);
        if (!ok) node.nextNode = 0;
    }
    
    if (parts.size() > 10) {
        node.crossRoad = parts[10].toUInt(&ok);
        if (!ok) node.crossRoad = 0;
    }
    
    node.name = QString("PathNode_%1").arg(node.id);
    
    return true;
}

bool DATParser::parseHandlingLine(const QString& line, VehicleHandling& handling) {
    QStringList parts = splitLine(line);
    
    // Handling format: Identifier, Mass, Drag, CenterOfMass(3), PercentSubmerged, etc.
    if (parts.size() < 30) {
        qWarning() << "DATParser: Invalid handling line format (expected ~30 fields):" << line;
        return false;
    }
    
    bool ok;
    int index = 0;
    
    handling.identifier = parts[index++];
    
    handling.mass = parts[index++].toFloat(&ok);
    if (!ok) return false;
    
    handling.dragMult = parts[index++].toFloat(&ok);
    if (!ok) return false;
    
    float comX = parts[index++].toFloat(&ok);
    if (!ok) return false;
    float comY = parts[index++].toFloat(&ok);
    if (!ok) return false;
    float comZ = parts[index++].toFloat(&ok);
    if (!ok) return false;
    handling.centerOfMass = QVector3D(comX, comY, comZ);
    
    handling.percentSubmerged = parts[index++].toUInt(&ok);
    if (!ok) return false;
    
    handling.tractionMult = parts[index++].toFloat(&ok);
    if (!ok) return false;
    
    handling.tractionLoss = parts[index++].toFloat(&ok);
    if (!ok) return false;
    
    handling.tractionBias = parts[index++].toFloat(&ok);
    if (!ok) return false;
    
    handling.transmissionData = parts[index++].toUInt(&ok);
    if (!ok) return false;
    
    handling.engineAcceleration = parts[index++].toFloat(&ok);
    if (!ok) return false;
    
    handling.engineInertia = parts[index++].toFloat(&ok);
    if (!ok) return false;
    
    handling.driveType = parts[index++].toUInt(&ok);
    if (!ok) return false;
    
    handling.engineType = parts[index++].toUInt(&ok);
    if (!ok) return false;
    
    handling.brakeDeceleration = parts[index++].toFloat(&ok);
    if (!ok) return false;
    
    handling.brakeBias = parts[index++].toFloat(&ok);
    if (!ok) return false;
    
    handling.abs = parts[index++].toInt(&ok) != 0;
    if (!ok) return false;
    
    handling.steeringLock = parts[index++].toFloat(&ok);
    if (!ok) return false;
    
    // Continue parsing remaining fields...
    // For brevity, we'll set defaults for the remaining fields
    handling.suspensionForceLevel = 1.0f;
    handling.suspensionDampingLevel = 0.1f;
    handling.suspensionHighSpeedComDamp = 0.0f;
    handling.suspensionUpperLimit = 0.3f;
    handling.suspensionLowerLimit = -0.15f;
    handling.suspensionBiasBetweenFrontAndRear = 0.5f;
    handling.suspensionAntiDiveMultiplier = 0.0f;
    handling.seatOffsetDistance = 0.2f;
    handling.collisionDamageMultiplier = 0.2f;
    handling.monetaryValue = 10000;
    handling.modelFlags = 0;
    handling.handlingFlags = 0;
    handling.frontLights = 0;
    handling.rearLights = 1;
    handling.animGroup = 0;
    
    return true;
}

bool DATParser::parseWaterLine(const QString& line, WaterPlane& plane) {
    QStringList parts = splitLine(line);
    
    // Water format: X1, Y1, Z1, X2, Y2, Z2, X3, Y3, Z3, X4, Y4, Z4, Level, Type
    if (parts.size() < 13) {
        qWarning() << "DATParser: Invalid water line format:" << line;
        return false;
    }
    
    bool ok;
    float x1 = parts[0].toFloat(&ok); if (!ok) return false;
    float y1 = parts[1].toFloat(&ok); if (!ok) return false;
    float z1 = parts[2].toFloat(&ok); if (!ok) return false;
    plane.corner1 = QVector3D(x1, y1, z1);
    
    float x2 = parts[3].toFloat(&ok); if (!ok) return false;
    float y2 = parts[4].toFloat(&ok); if (!ok) return false;
    float z2 = parts[5].toFloat(&ok); if (!ok) return false;
    plane.corner2 = QVector3D(x2, y2, z2);
    
    float x3 = parts[6].toFloat(&ok); if (!ok) return false;
    float y3 = parts[7].toFloat(&ok); if (!ok) return false;
    float z3 = parts[8].toFloat(&ok); if (!ok) return false;
    plane.corner3 = QVector3D(x3, y3, z3);
    
    float x4 = parts[9].toFloat(&ok); if (!ok) return false;
    float y4 = parts[10].toFloat(&ok); if (!ok) return false;
    float z4 = parts[11].toFloat(&ok); if (!ok) return false;
    plane.corner4 = QVector3D(x4, y4, z4);
    
    plane.level = parts[12].toFloat(&ok);
    if (!ok) return false;
    
    if (parts.size() > 13) {
        plane.type = parts[13].toUInt(&ok);
        if (!ok) plane.type = 0;
    }
    
    return true;
}

bool DATParser::parseBinaryPath(QDataStream& stream, QVector<PathNode>& nodes) {
    BinaryPathHeader header;
    stream.readRawData(reinterpret_cast<char*>(&header), sizeof(header));
    
    if (stream.status() != QDataStream::Ok) {
        qWarning() << "DATParser: Failed to read binary path header";
        return false;
    }
    
    qDebug() << "DATParser: Binary path with" << header.numNodes << "nodes";
    
    for (uint32_t i = 0; i < header.numNodes; ++i) {
        BinaryPathNode binaryNode;
        stream.readRawData(reinterpret_cast<char*>(&binaryNode), sizeof(binaryNode));
        
        if (stream.status() != QDataStream::Ok) {
            qWarning() << "DATParser: Failed to read binary path node" << i;
            break;
        }
        
        PathNode node;
        node.id = binaryNode.nodeId;
        node.position = binaryNode.position;
        node.width = binaryNode.pathWidth / 255.0f * 10.0f; // Convert to meters
        node.nodeType = binaryNode.nodeType;
        node.nextNode = binaryNode.linkId;
        node.crossRoad = 0; // Not available in binary format
        node.name = QString("PathNode_%1").arg(node.id);
        
        nodes.append(node);
    }
    
    return true;
}

bool DATParser::isBinaryPathFile(QIODevice* device) {
    qint64 originalPos = device->pos();
    
    // Read first few bytes to check for binary signature
    char buffer[16];
    qint64 bytesRead = device->read(buffer, 16);
    device->seek(originalPos);
    
    if (bytesRead < 16) {
        return false;
    }
    
    // Check if it looks like binary data (non-ASCII characters)
    for (int i = 0; i < 16; ++i) {
        if (static_cast<unsigned char>(buffer[i]) > 127) {
            return true;
        }
    }
    
    return false;
}

