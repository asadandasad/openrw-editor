#ifndef IDE_PARSER_H
#define IDE_PARSER_H

#include "types.h"
#include <QTextStream>
#include <QIODevice>

// IDE (Item Definition) file format parser
// Defines object properties and characteristics
class IDEParser {
public:
    static bool parse(QIODevice* device, QVector<IDEObject>& objects);
    static bool parseFromFile(const QString& filePath, QVector<IDEObject>& objects);
    
private:
    // IDE section types
    enum IDESection {
        OBJS,      // Static objects
        TOBJ,      // Timed objects
        WEAP,      // Weapons
        HIER,      // Hierarchical objects
        CARS,      // Vehicles
        PEDS,      // Pedestrians
        PATH,      // Path objects
        TXDP,      // Texture dictionary parent
        ANIM,      // Animated objects
        UNKNOWN
    };
    
    // Object flags
    enum ObjectFlags {
        DRAW_LAST = 0x01,
        ADDITIVE = 0x02,
        WORK_IN_RAIN = 0x04,
        DONT_LIGHT = 0x08,
        NO_ZBUFFER_WRITE = 0x10,
        DONT_RECEIVE_SHADOWS = 0x20,
        IGNORE_DRAW_DIST = 0x40,
        IS_GLASS_TYPE1 = 0x80,
        IS_GLASS_TYPE2 = 0x100,
        IS_GARAGE_DOOR = 0x200,
        IS_DAMAGEABLE = 0x400,
        IS_TREE = 0x800,
        IS_PALM = 0x1000,
        DOES_NOT_COLLIDE_WITH_FLYER = 0x2000,
        USE_AMBIENT_SCALE = 0x4000,
        IS_HAND_OBJECT = 0x8000,
        EXPLOSIVE = 0x10000,
        WET_EFFECT = 0x20000,
        DRY_EFFECT = 0x40000,
        UNKNOWN_FLAG = 0x80000
    };
    
    static IDESection parseSection(const QString& sectionName);
    static bool parseObjsSection(QTextStream& stream, QVector<IDEObject>& objects);
    static bool parseObjLine(const QString& line, IDEObject& object);
    
    static void skipSection(QTextStream& stream);
    static QString readLine(QTextStream& stream);
    static QStringList splitLine(const QString& line);
    static uint32_t parseFlags(const QString& flagsStr);
};

#endif // IDE_PARSER_H

