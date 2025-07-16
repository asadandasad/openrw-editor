#include "ide_parser.h"
#include <QFile>
#include <QDebug>
#include <QRegularExpression>

bool IDEParser::parse(QIODevice* device, QVector<IDEObject>& objects) {
    if (!device || !device->isOpen()) {
        qWarning() << "IDEParser: Invalid or closed device";
        return false;
    }
    
    QTextStream stream(device);
    QString line;
    IDESection currentSection = UNKNOWN;
    
    while (!stream.atEnd()) {
        line = readLine(stream);
        
        if (line.isEmpty() || line.startsWith('#') || line.startsWith('%')) {
            continue; // Skip empty lines and comments
        }
        
        // Check for section headers
        if (line == "objs") {
            currentSection = OBJS;
            qDebug() << "IDEParser: Entering OBJS section";
            continue;
        } else if (line == "tobj") {
            currentSection = TOBJ;
            continue;
        } else if (line == "weap") {
            currentSection = WEAP;
            continue;
        } else if (line == "hier") {
            currentSection = HIER;
            continue;
        } else if (line == "cars") {
            currentSection = CARS;
            continue;
        } else if (line == "peds") {
            currentSection = PEDS;
            continue;
        } else if (line == "path") {
            currentSection = PATH;
            continue;
        } else if (line == "txdp") {
            currentSection = TXDP;
            continue;
        } else if (line == "anim") {
            currentSection = ANIM;
            continue;
        } else if (line == "end") {
            currentSection = UNKNOWN;
            continue;
        }
        
        // Parse section content
        switch (currentSection) {
            case OBJS:
                if (!parseObjsSection(stream, objects)) {
                    // If parsing fails, try to parse the current line as an object
                    IDEObject object;
                    if (parseObjLine(line, object)) {
                        objects.append(object);
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

bool IDEParser::parseFromFile(const QString& filePath, QVector<IDEObject>& objects) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "IDEParser: Failed to open file:" << filePath;
        return false;
    }
    
    bool result = parse(&file, objects);
    file.close();
    
    if (result) {
        qDebug() << "IDEParser: Successfully parsed" << filePath << "with" << objects.size() << "objects";
    }
    
    return result;
}

IDEParser::IDESection IDEParser::parseSection(const QString& sectionName) {
    QString lower = sectionName.toLower();
    if (lower == "objs") return OBJS;
    if (lower == "tobj") return TOBJ;
    if (lower == "weap") return WEAP;
    if (lower == "hier") return HIER;
    if (lower == "cars") return CARS;
    if (lower == "peds") return PEDS;
    if (lower == "path") return PATH;
    if (lower == "txdp") return TXDP;
    if (lower == "anim") return ANIM;
    return UNKNOWN;
}

bool IDEParser::parseObjsSection(QTextStream& stream, QVector<IDEObject>& objects) {
    QString line;
    while (!stream.atEnd()) {
        qint64 pos = stream.pos();
        line = readLine(stream);
        
        if (line.isEmpty() || line.startsWith('#') || line.startsWith('%')) {
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
        
        IDEObject object;
        if (parseObjLine(line, object)) {
            objects.append(object);
        }
    }
    
    return true;
}

bool IDEParser::parseObjLine(const QString& line, IDEObject& object) {
    QStringList parts = splitLine(line);
    
    // OBJS format: ID, ModelName, TxdName, MeshCount, DrawDist, Flags
    if (parts.size() < 5) {
        qWarning() << "IDEParser: Invalid OBJS line format:" << line;
        return false;
    }
    
    bool ok;
    object.id = parts[0].toUInt(&ok);
    if (!ok) {
        qWarning() << "IDEParser: Invalid ID in OBJS line:" << parts[0];
        return false;
    }
    
    object.modelName = parts[1];
    object.textureName = parts[2];
    
    object.meshCount = parts[3].toUInt(&ok);
    if (!ok) {
        qWarning() << "IDEParser: Invalid mesh count in OBJS line:" << parts[3];
        return false;
    }
    
    object.drawDistance = parts[4].toFloat(&ok);
    if (!ok) {
        qWarning() << "IDEParser: Invalid draw distance in OBJS line:" << parts[4];
        return false;
    }
    
    // Parse flags if present
    if (parts.size() > 5) {
        object.flags = parseFlags(parts[5]);
    } else {
        object.flags = 0;
    }
    
    return true;
}

void IDEParser::skipSection(QTextStream& stream) {
    QString line;
    while (!stream.atEnd()) {
        line = readLine(stream);
        if (line == "end") {
            break;
        }
    }
}

QString IDEParser::readLine(QTextStream& stream) {
    QString line = stream.readLine().trimmed();
    
    // Remove comments
    int commentPos = line.indexOf('#');
    if (commentPos >= 0) {
        line = line.left(commentPos).trimmed();
    }
    
    commentPos = line.indexOf('%');
    if (commentPos >= 0) {
        line = line.left(commentPos).trimmed();
    }
    
    return line;
}

QStringList IDEParser::splitLine(const QString& line) {
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

uint32_t IDEParser::parseFlags(const QString& flagsStr) {
    bool ok;
    uint32_t flags = flagsStr.toUInt(&ok);
    
    if (!ok) {
        // Try parsing as hex
        flags = flagsStr.toUInt(&ok, 16);
    }
    
    if (!ok) {
        qWarning() << "IDEParser: Invalid flags format:" << flagsStr;
        return 0;
    }
    
    return flags;
}

