#ifndef DFF_PARSER_H
#define DFF_PARSER_H

#include "types.h"
#include <QIODevice>
#include <QDataStream>

// DFF (RenderWare Model) file format parser
// Based on RenderWare Graphics SDK documentation
class DFFParser {
public:
    static bool parse(QIODevice* device, GTAModel& model);
    static bool parseFromFile(const QString& filePath, GTAModel& model);
    
private:
    // RenderWare chunk structure
    struct RWChunk {
        uint32_t type;
        uint32_t size;
        uint32_t version;
    };
    
    // RenderWare chunk types
    enum RWChunkType {
        rwCLUMP = 0x10,
        rwFRAMELIST = 0x0E,
        rwGEOMETRY = 0x0F,
        rwGEOMETRYLIST = 0x1A,
        rwATOMIC = 0x14,
        rwMATERIAL = 0x07,
        rwMATERIALLIST = 0x08,
        rwTEXTURE = 0x06,
        rwSTRING = 0x02,
        rwEXTENSION = 0x03,
        rwDATA = 0x01
    };
    
    // Geometry flags
    enum GeometryFlags {
        rpGEOMETRYTRISTRIP = 0x01,
        rpGEOMETRYPOSITIONS = 0x02,
        rpGEOMETRYTEXTURED = 0x04,
        rpGEOMETRYPRELIT = 0x08,
        rpGEOMETRYNORMALS = 0x10,
        rpGEOMETRYLIGHT = 0x20,
        rpGEOMETRYMODULATEMATERIALCOLOR = 0x40,
        rpGEOMETRYTEXTURED2 = 0x80
    };
    
    static bool readChunk(QDataStream& stream, RWChunk& chunk);
    static bool parseClump(QDataStream& stream, const RWChunk& chunk, GTAModel& model);
    static bool parseFrameList(QDataStream& stream, const RWChunk& chunk);
    static bool parseGeometryList(QDataStream& stream, const RWChunk& chunk, GTAModel& model);
    static bool parseGeometry(QDataStream& stream, const RWChunk& chunk, GTAMesh& mesh);
    static bool parseMaterialList(QDataStream& stream, const RWChunk& chunk, QVector<GTAMaterial>& materials);
    static bool parseMaterial(QDataStream& stream, const RWChunk& chunk, GTAMaterial& material);
    static bool parseTexture(QDataStream& stream, const RWChunk& chunk, QString& textureName);
    static bool parseString(QDataStream& stream, const RWChunk& chunk, QString& str);
    static bool parseAtomic(QDataStream& stream, const RWChunk& chunk);
    
    static void skipChunk(QDataStream& stream, const RWChunk& chunk);
    static BoundingBox calculateBoundingBox(const QVector<GTAVertex>& vertices);
};

#endif // DFF_PARSER_H

