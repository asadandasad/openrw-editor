#ifndef IPL_PARSER_H
#define IPL_PARSER_H

#include "types.h"
#include <QTextStream>
#include <QIODevice>

// IPL (Item Placement List) file format parser
// Handles both text and binary IPL formats
class IPLParser {
public:
    static bool parse(QIODevice* device, QVector<IPLInstance>& instances);
    static bool parseFromFile(const QString& filePath, QVector<IPLInstance>& instances);
    
private:
    // IPL section types
    enum IPLSection {
        INST,      // Object instances
        ZONE,      // Zone definitions
        CULL,      // Culling zones
        PICK,      // Pickup instances
        PATH,      // Path nodes
        OCCL,      // Occlusion zones
        MULT,      // Multi-object instances
        GRGE,      // Garage definitions
        ENEX,      // Entrance/exit points
        CARS,      // Car generators
        JUMP,      // Stunt jumps
        TCYC,      // Time cycles
        AUZO,      // Audio zones
        UNKNOWN
    };
    
    static bool parseTextFormat(QTextStream& stream, QVector<IPLInstance>& instances);
    static bool parseBinaryFormat(QDataStream& stream, QVector<IPLInstance>& instances);
    static bool isBinaryFormat(QIODevice* device);
    
    static IPLSection parseSection(const QString& sectionName);
    static bool parseInstSection(QTextStream& stream, QVector<IPLInstance>& instances);
    static bool parseInstLine(const QString& line, IPLInstance& instance);
    
    static void skipSection(QTextStream& stream);
    static QString readLine(QTextStream& stream);
    static QStringList splitLine(const QString& line);
    
    // Binary format structures
    struct BinaryIPLHeader {
        char signature[4];
        uint32_t itemCount;
    };
    
    struct BinaryIPLItem {
        float posX, posY, posZ;
        float rotX, rotY, rotZ, rotW;
        uint32_t modelId;
        uint32_t interior;
        uint32_t lod;
    };
};

#endif // IPL_PARSER_H

