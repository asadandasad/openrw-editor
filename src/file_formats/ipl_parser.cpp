#include "ipl_parser.h"
#include "math_utils.h"
#include <QFile>
#include <QDebug>
#include <QDataStream>
#include <QTextStream>
#include <QRegularExpression>

bool IPLParser::parse(QIODevice* device, QVector<IPLInstance>& instances) {
    if (!device || !device->isOpen()) {
        qWarning() << "IPLParser: Invalid or closed device";
        return false;
    }
    
    // Check if it's binary or text format
    if (isBinaryFormat(device)) {
        QDataStream stream(device);
        stream.setByteOrder(QDataStream::LittleEndian);
        return parseBinaryFormat(stream, instances);
    } else {
        QTextStream stream(device);
        return parseTextFormat(stream, instances);
    }
}

bool IPLParser::parseFromFile(const QString& filePath, QVector<IPLInstance>& instances) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "IPLParser: Failed to open file:" << filePath;
        return false;
    }
    
    bool result = parse(&file, instances);
    file.close();
    
    if (result) {
        qDebug() << "IPLParser: Successfully parsed" << filePath << "with" << instances.size() << "instances";
    }
    
    return result;
}

bool IPLParser::parseTextFormat(QTextStream& stream, QVector<IPLInstance>& instances) {
    QString line;
    IPLSection currentSection = UNKNOWN;
    
    while (!stream.atEnd()) {
        line = readLine(stream);
        
        if (line.isEmpty() || line.startsWith('#')) {
            continue; // Skip empty lines and comments
        }
        
        // Check for section headers
        if (line == "inst") {
            currentSection = INST;
            qDebug() << "IPLParser: Entering INST section";
            continue;
        } else if (line == "zone") {
            currentSection = ZONE;
            continue;
        } else if (line == "cull") {
            currentSection = CULL;
            continue;
        } else if (line == "pick") {
            currentSection = PICK;
            continue;
        } else if (line == "path") {
            currentSection = PATH;
            continue;
        } else if (line == "occl") {
            currentSection = OCCL;
            continue;
        } else if (line == "mult") {
            currentSection = MULT;
            continue;
        } else if (line == "grge") {
            currentSection = GRGE;
            continue;
        } else if (line == "enex") {
            currentSection = ENEX;
            continue;
        } else if (line == "cars") {
            currentSection = CARS;
            continue;
        } else if (line == "jump") {
            currentSection = JUMP;
            continue;
        } else if (line == "tcyc") {
            currentSection = TCYC;
            continue;
        } else if (line == "auzo") {
            currentSection = AUZO;
            continue;
        } else if (line == "end") {
            currentSection = UNKNOWN;
            continue;
        }
        
        // Parse section content
        switch (currentSection) {
            case INST:
                if (!parseInstSection(stream, instances)) {
                    // If parsing fails, try to parse the current line as an instance
                    IPLInstance instance;
                    if (parseInstLine(line, instance)) {
                        instances.append(instance);
                    }
                }
                break;
            default:
                // Skip other sections for now
                break;
        }
    }
    
    return true;
}

bool IPLParser::parseBinaryFormat(QDataStream& stream, QVector<IPLInstance>& instances) {
    BinaryIPLHeader header;
    stream.readRawData(reinterpret_cast<char*>(&header), sizeof(header));
    
    if (stream.status() != QDataStream::Ok) {
        qWarning() << "IPLParser: Failed to read binary header";
        return false;
    }
    
    qDebug() << "IPLParser: Binary IPL with" << header.itemCount << "items";
    
    for (uint32_t i = 0; i < header.itemCount; ++i) {
        BinaryIPLItem item;
        stream.readRawData(reinterpret_cast<char*>(&item), sizeof(item));
        
        if (stream.status() != QDataStream::Ok) {
            qWarning() << "IPLParser: Failed to read binary item" << i;
            break;
        }
        
        IPLInstance instance;
        instance.id = item.modelId;
        instance.modelName = QString("Model_%1").arg(item.modelId);
        instance.transform.position = QVector3D(item.posX, item.posY, item.posZ);
        instance.transform.rotation = QQuaternion(item.rotW, item.rotX, item.rotY, item.rotZ);
        instance.interior = item.interior;
        instance.lod = item.lod;
        
        instances.append(instance);
    }
    
    return true;
}

bool IPLParser::isBinaryFormat(QIODevice* device) {
    qint64 originalPos = device->pos();
    
    // Read first 4 bytes to check for binary signature
    char signature[4];
    qint64 bytesRead = device->read(signature, 4);
    device->seek(originalPos);
    
    if (bytesRead != 4) {
        return false;
    }
    
    // Check for common binary signatures or patterns
    // This is a simplified check - in practice, you'd need to know the specific binary format
    return (signature[0] == 'I' && signature[1] == 'P' && signature[2] == 'L' && signature[3] == 'B') ||
           (static_cast<unsigned char>(signature[0]) > 127); // Non-ASCII character suggests binary
}

IPLParser::IPLSection IPLParser::parseSection(const QString& sectionName) {
    QString lower = sectionName.toLower();
    if (lower == "inst") return INST;
    if (lower == "zone") return ZONE;
    if (lower == "cull") return CULL;
    if (lower == "pick") return PICK;
    if (lower == "path") return PATH;
    if (lower == "occl") return OCCL;
    if (lower == "mult") return MULT;
    if (lower == "grge") return GRGE;
    if (lower == "enex") return ENEX;
    if (lower == "cars") return CARS;
    if (lower == "jump") return JUMP;
    if (lower == "tcyc") return TCYC;
    if (lower == "auzo") return AUZO;
    return UNKNOWN;
}

bool IPLParser::parseInstSection(QTextStream& stream, QVector<IPLInstance>& instances) {
    QString line;
    while (!stream.atEnd()) {
        qint64 pos = stream.pos();
        line = readLine(stream);
        
        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }
        
        if (line == "end") {
            return true;
        }
        
        // Check if this is a new section
        if (parseSection(line) != UNKNOWN) {
            // Rewind to before this line
            stream.seek(pos);
            return true;
        }
        
        IPLInstance instance;
        if (parseInstLine(line, instance)) {
            instances.append(instance);
        }
    }
    
    return true;
}

bool IPLParser::parseInstLine(const QString& line, IPLInstance& instance) {
    QStringList parts = splitLine(line);
    
    // INST format: ID, ModelName, Interior, PosX, PosY, PosZ, RotX, RotY, RotZ, RotW, LOD
    if (parts.size() < 10) {
        qWarning() << "IPLParser: Invalid INST line format:" << line;
        return false;
    }
    
    bool ok;
    instance.id = parts[0].toUInt(&ok);
    if (!ok) {
        qWarning() << "IPLParser: Invalid ID in INST line:" << parts[0];
        return false;
    }
    
    instance.modelName = parts[1];
    instance.interior = parts[2].toUInt();
    
    float posX = parts[3].toFloat(&ok);
    if (!ok) return false;
    float posY = parts[4].toFloat(&ok);
    if (!ok) return false;
    float posZ = parts[5].toFloat(&ok);
    if (!ok) return false;
    
    instance.transform.position = QVector3D(posX, posY, posZ);
    
    float rotX = parts[6].toFloat(&ok);
    if (!ok) return false;
    float rotY = parts[7].toFloat(&ok);
    if (!ok) return false;
    float rotZ = parts[8].toFloat(&ok);
    if (!ok) return false;
    float rotW = parts[9].toFloat(&ok);
    if (!ok) return false;
    
    instance.transform.rotation = QQuaternion(rotW, rotX, rotY, rotZ);
    
    if (parts.size() > 10) {
        instance.lod = parts[10].toUInt();
    }
    
    return true;
}

void IPLParser::skipSection(QTextStream& stream) {
    QString line;
    while (!stream.atEnd()) {
        line = readLine(stream);
        if (line == "end") {
            break;
        }
    }
}

QString IPLParser::readLine(QTextStream& stream) {
    QString line = stream.readLine().trimmed();
    
    // Remove comments
    int commentPos = line.indexOf('#');
    if (commentPos >= 0) {
        line = line.left(commentPos).trimmed();
    }
    
    return line;
}

QStringList IPLParser::splitLine(const QString& line) {
    // Split by comma or whitespace, handling quoted strings
    QStringList result;
    QRegularExpression regex(R"([^\s,]+|"[^"]*")");
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

