#include "dff_parser.h"
#include <QFile>
#include <QDebug>
#include <QVector2D>

bool DFFParser::parse(QIODevice* device, GTAModel& model) {
    if (!device || !device->isOpen()) {
        qWarning() << "DFFParser: Invalid or closed device";
        return false;
    }
    
    QDataStream stream(device);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    RWChunk rootChunk;
    if (!readChunk(stream, rootChunk)) {
        qWarning() << "DFFParser: Failed to read root chunk";
        return false;
    }
    
    if (rootChunk.type != rwCLUMP) {
        qWarning() << "DFFParser: Root chunk is not a CLUMP, got type:" << Qt::hex << rootChunk.type;
        return false;
    }
    
    return parseClump(stream, rootChunk, model);
}

bool DFFParser::parseFromFile(const QString& filePath, GTAModel& model) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "DFFParser: Failed to open file:" << filePath;
        return false;
    }
    
    model.name = QFileInfo(filePath).baseName();
    bool result = parse(&file, model);
    file.close();
    
    if (result) {
        qDebug() << "DFFParser: Successfully parsed" << filePath << "with" << model.meshes.size() << "meshes";
    }
    
    return result;
}

bool DFFParser::readChunk(QDataStream& stream, RWChunk& chunk) {
    if (stream.atEnd()) {
        return false;
    }
    
    stream >> chunk.type >> chunk.size >> chunk.version;
    
    if (stream.status() != QDataStream::Ok) {
        qWarning() << "DFFParser: Failed to read chunk header";
        return false;
    }
    
    return true;
}

bool DFFParser::parseClump(QDataStream& stream, const RWChunk& chunk, GTAModel& model) {
    qint64 chunkEnd = stream.device()->pos() + chunk.size - 12; // Subtract header size
    
    // Read clump data
    RWChunk dataChunk;
    if (!readChunk(stream, dataChunk) || dataChunk.type != rwDATA) {
        qWarning() << "DFFParser: Expected DATA chunk in CLUMP";
        return false;
    }
    
    uint32_t atomicCount;
    stream >> atomicCount;
    skipChunk(stream, dataChunk);
    
    qDebug() << "DFFParser: Clump contains" << atomicCount << "atomics";
    
    // Parse child chunks
    while (stream.device()->pos() < chunkEnd && !stream.atEnd()) {
        RWChunk childChunk;
        if (!readChunk(stream, childChunk)) {
            break;
        }
        
        switch (childChunk.type) {
            case rwFRAMELIST:
                parseFrameList(stream, childChunk);
                break;
            case rwGEOMETRYLIST:
                parseGeometryList(stream, childChunk, model);
                break;
            case rwATOMIC:
                parseAtomic(stream, childChunk);
                break;
            default:
                skipChunk(stream, childChunk);
                break;
        }
    }
    
    // Calculate overall bounding box
    if (!model.meshes.isEmpty()) {
        model.boundingBox = model.meshes.first().boundingBox;
        for (const auto& mesh : model.meshes) {
            model.boundingBox.min = QVector3D(
                qMin(model.boundingBox.min.x(), mesh.boundingBox.min.x()),
                qMin(model.boundingBox.min.y(), mesh.boundingBox.min.y()),
                qMin(model.boundingBox.min.z(), mesh.boundingBox.min.z())
            );
            model.boundingBox.max = QVector3D(
                qMax(model.boundingBox.max.x(), mesh.boundingBox.max.x()),
                qMax(model.boundingBox.max.y(), mesh.boundingBox.max.y()),
                qMax(model.boundingBox.max.z(), mesh.boundingBox.max.z())
            );
        }
    }
    
    return true;
}

bool DFFParser::parseFrameList(QDataStream& stream, const RWChunk& chunk) {
    // For now, we'll skip frame parsing as it's mainly for hierarchy
    // In a full implementation, this would parse the bone/frame structure
    skipChunk(stream, chunk);
    return true;
}

bool DFFParser::parseGeometryList(QDataStream& stream, const RWChunk& chunk, GTAModel& model) {
    qint64 chunkEnd = stream.device()->pos() + chunk.size - 12;
    
    // Read geometry list data
    RWChunk dataChunk;
    if (!readChunk(stream, dataChunk) || dataChunk.type != rwDATA) {
        qWarning() << "DFFParser: Expected DATA chunk in GEOMETRYLIST";
        return false;
    }
    
    uint32_t geometryCount;
    stream >> geometryCount;
    skipChunk(stream, dataChunk);
    
    qDebug() << "DFFParser: GeometryList contains" << geometryCount << "geometries";
    
    // Parse geometries
    while (stream.device()->pos() < chunkEnd && !stream.atEnd()) {
        RWChunk geomChunk;
        if (!readChunk(stream, geomChunk)) {
            break;
        }
        
        if (geomChunk.type == rwGEOMETRY) {
            GTAMesh mesh;
            mesh.name = QString("Mesh_%1").arg(model.meshes.size());
            if (parseGeometry(stream, geomChunk, mesh)) {
                model.meshes.append(mesh);
            }
        } else {
            skipChunk(stream, geomChunk);
        }
    }
    
    return true;
}

bool DFFParser::parseGeometry(QDataStream& stream, const RWChunk& chunk, GTAMesh& mesh) {
    qint64 chunkEnd = stream.device()->pos() + chunk.size - 12;
    
    // Read geometry data
    RWChunk dataChunk;
    if (!readChunk(stream, dataChunk) || dataChunk.type != rwDATA) {
        qWarning() << "DFFParser: Expected DATA chunk in GEOMETRY";
        return false;
    }
    
    uint32_t flags, triangleCount, vertexCount, morphTargetCount;
    stream >> flags >> triangleCount >> vertexCount >> morphTargetCount;
    
    qDebug() << "DFFParser: Geometry - Flags:" << Qt::hex << flags 
             << "Triangles:" << triangleCount << "Vertices:" << vertexCount;
    
    // Read vertices
    mesh.vertices.resize(vertexCount);
    
    // Read positions
    if (flags & rpGEOMETRYPOSITIONS) {
        for (uint32_t i = 0; i < vertexCount; ++i) {
            float x, y, z;
            stream >> x >> y >> z;
            mesh.vertices[i].position = QVector3D(x, y, z);
        }
    }
    
    // Read normals
    if (flags & rpGEOMETRYNORMALS) {
        for (uint32_t i = 0; i < vertexCount; ++i) {
            float x, y, z;
            stream >> x >> y >> z;
            mesh.vertices[i].normal = QVector3D(x, y, z);
        }
    }
    
    // Read vertex colors
    if (flags & rpGEOMETRYPRELIT) {
        for (uint32_t i = 0; i < vertexCount; ++i) {
            uint32_t color;
            stream >> color;
            mesh.vertices[i].color = color;
        }
    }
    
    // Read texture coordinates
    if (flags & rpGEOMETRYTEXTURED) {
        for (uint32_t i = 0; i < vertexCount; ++i) {
            float u, v;
            stream >> u >> v;
            mesh.vertices[i].texCoord = QVector2D(u, v);
        }
    }
    
    // Read triangles
    mesh.indices.resize(triangleCount * 3);
    for (uint32_t i = 0; i < triangleCount; ++i) {
        uint16_t v1, v2;
        uint32_t materialId, v3;
        stream >> v1 >> v2 >> materialId >> v3;
        
        mesh.indices[i * 3] = v1;
        mesh.indices[i * 3 + 1] = v2;
        mesh.indices[i * 3 + 2] = v3;
    }
    
    skipChunk(stream, dataChunk);
    
    // Parse child chunks (materials, etc.)
    QVector<GTAMaterial> materials;
    while (stream.device()->pos() < chunkEnd && !stream.atEnd()) {
        RWChunk childChunk;
        if (!readChunk(stream, childChunk)) {
            break;
        }
        
        switch (childChunk.type) {
            case rwMATERIALLIST:
                parseMaterialList(stream, childChunk, materials);
                break;
            default:
                skipChunk(stream, childChunk);
                break;
        }
    }
    
    // Assign first material if available
    if (!materials.isEmpty()) {
        mesh.material = materials.first();
    }
    
    // Calculate bounding box
    mesh.boundingBox = calculateBoundingBox(mesh.vertices);
    
    return true;
}

bool DFFParser::parseMaterialList(QDataStream& stream, const RWChunk& chunk, QVector<GTAMaterial>& materials) {
    qint64 chunkEnd = stream.device()->pos() + chunk.size - 12;
    
    // Read material list data
    RWChunk dataChunk;
    if (!readChunk(stream, dataChunk) || dataChunk.type != rwDATA) {
        qWarning() << "DFFParser: Expected DATA chunk in MATERIALLIST";
        return false;
    }
    
    uint32_t materialCount;
    stream >> materialCount;
    
    // Skip material indices
    for (uint32_t i = 0; i < materialCount; ++i) {
        uint32_t materialIndex;
        stream >> materialIndex;
    }
    
    skipChunk(stream, dataChunk);
    
    // Parse materials
    while (stream.device()->pos() < chunkEnd && !stream.atEnd()) {
        RWChunk matChunk;
        if (!readChunk(stream, matChunk)) {
            break;
        }
        
        if (matChunk.type == rwMATERIAL) {
            GTAMaterial material;
            if (parseMaterial(stream, matChunk, material)) {
                materials.append(material);
            }
        } else {
            skipChunk(stream, matChunk);
        }
    }
    
    return true;
}

bool DFFParser::parseMaterial(QDataStream& stream, const RWChunk& chunk, GTAMaterial& material) {
    qint64 chunkEnd = stream.device()->pos() + chunk.size - 12;
    
    // Read material data
    RWChunk dataChunk;
    if (!readChunk(stream, dataChunk) || dataChunk.type != rwDATA) {
        qWarning() << "DFFParser: Expected DATA chunk in MATERIAL";
        return false;
    }
    
    uint32_t flags;
    float r, g, b, a;
    uint32_t unknown;
    uint32_t textured;
    
    stream >> flags >> r >> g >> b >> a >> unknown >> textured;
    
    material.diffuse = QVector3D(r, g, b);
    material.name = QString("Material_%1").arg(materials.size());
    
    skipChunk(stream, dataChunk);
    
    // Parse child chunks (texture)
    while (stream.device()->pos() < chunkEnd && !stream.atEnd()) {
        RWChunk childChunk;
        if (!readChunk(stream, childChunk)) {
            break;
        }
        
        switch (childChunk.type) {
            case rwTEXTURE:
                parseTexture(stream, childChunk, material.textureName);
                break;
            default:
                skipChunk(stream, childChunk);
                break;
        }
    }
    
    return true;
}

bool DFFParser::parseTexture(QDataStream& stream, const RWChunk& chunk, QString& textureName) {
    qint64 chunkEnd = stream.device()->pos() + chunk.size - 12;
    
    // Skip texture data chunk
    RWChunk dataChunk;
    if (readChunk(stream, dataChunk) && dataChunk.type == rwDATA) {
        skipChunk(stream, dataChunk);
    }
    
    // Parse texture name
    while (stream.device()->pos() < chunkEnd && !stream.atEnd()) {
        RWChunk childChunk;
        if (!readChunk(stream, childChunk)) {
            break;
        }
        
        if (childChunk.type == rwSTRING) {
            parseString(stream, childChunk, textureName);
            break; // Usually the first string is the texture name
        } else {
            skipChunk(stream, childChunk);
        }
    }
    
    // Skip remaining chunks
    while (stream.device()->pos() < chunkEnd && !stream.atEnd()) {
        RWChunk childChunk;
        if (!readChunk(stream, childChunk)) {
            break;
        }
        skipChunk(stream, childChunk);
    }
    
    return true;
}

bool DFFParser::parseString(QDataStream& stream, const RWChunk& chunk, QString& str) {
    QByteArray data(chunk.size, 0);
    stream.readRawData(data.data(), chunk.size);
    
    // Remove null terminator if present
    int nullIndex = data.indexOf('\0');
    if (nullIndex >= 0) {
        data = data.left(nullIndex);
    }
    
    str = QString::fromLatin1(data);
    return true;
}

bool DFFParser::parseAtomic(QDataStream& stream, const RWChunk& chunk) {
    // For now, we'll skip atomic parsing
    // In a full implementation, this would link geometries to frames
    skipChunk(stream, chunk);
    return true;
}

void DFFParser::skipChunk(QDataStream& stream, const RWChunk& chunk) {
    qint64 bytesToSkip = chunk.size - 12; // Subtract header size
    if (bytesToSkip > 0) {
        stream.skipRawData(bytesToSkip);
    }
}

BoundingBox DFFParser::calculateBoundingBox(const QVector<GTAVertex>& vertices) {
    if (vertices.isEmpty()) {
        return BoundingBox();
    }
    
    QVector3D min = vertices.first().position;
    QVector3D max = vertices.first().position;
    
    for (const auto& vertex : vertices) {
        const QVector3D& pos = vertex.position;
        min = QVector3D(qMin(min.x(), pos.x()), qMin(min.y(), pos.y()), qMin(min.z(), pos.z()));
        max = QVector3D(qMax(max.x(), pos.x()), qMax(max.y(), pos.y()), qMax(max.z(), pos.z()));
    }
    
    return BoundingBox(min, max);
}

