#ifndef TXD_PARSER_H
#define TXD_PARSER_H

#include "types.h"
#include <QIODevice>
#include <QDataStream>
#include <QImage>

// TXD (Texture Dictionary) file format parser
// Based on RenderWare Graphics SDK documentation
class TXDParser {
public:
    struct GTATexture {
        QString name;
        QString maskName;
        QImage image;
        uint32_t width;
        uint32_t height;
        uint32_t depth;
        uint32_t format;
        uint32_t mipmapCount;
        bool hasAlpha;
    };
    
    static bool parse(QIODevice* device, QVector<GTATexture>& textures);
    static bool parseFromFile(const QString& filePath, QVector<GTATexture>& textures);
    
private:
    // RenderWare chunk structure (same as DFF)
    struct RWChunk {
        uint32_t type;
        uint32_t size;
        uint32_t version;
    };
    
    // RenderWare chunk types
    enum RWChunkType {
        rwTEXDICTIONARY = 0x16,
        rwTEXNATIVE = 0x15,
        rwSTRING = 0x02,
        rwEXTENSION = 0x03,
        rwDATA = 0x01
    };
    
    // Texture formats
    enum TextureFormat {
        RASTER_DEFAULT = 0x0000,
        RASTER_1555 = 0x0100,
        RASTER_565 = 0x0200,
        RASTER_4444 = 0x0300,
        RASTER_LUM8 = 0x0400,
        RASTER_8888 = 0x0500,
        RASTER_888 = 0x0600,
        RASTER_16 = 0x0700,
        RASTER_24 = 0x0800,
        RASTER_32 = 0x0900,
        RASTER_555 = 0x0A00,
        RASTER_DXT1 = 0x0B00,
        RASTER_DXT3 = 0x0C00,
        RASTER_DXT5 = 0x0D00
    };
    
    static bool readChunk(QDataStream& stream, RWChunk& chunk);
    static bool parseTextureDictionary(QDataStream& stream, const RWChunk& chunk, QVector<GTATexture>& textures);
    static bool parseTextureNative(QDataStream& stream, const RWChunk& chunk, GTATexture& texture);
    static bool parseString(QDataStream& stream, const RWChunk& chunk, QString& str);
    
    static void skipChunk(QDataStream& stream, const RWChunk& chunk);
    
    // Texture decompression functions
    static QImage decompressDXT1(const QByteArray& data, uint32_t width, uint32_t height);
    static QImage decompressDXT3(const QByteArray& data, uint32_t width, uint32_t height);
    static QImage decompressDXT5(const QByteArray& data, uint32_t width, uint32_t height);
    static QImage convertPalettedTexture(const QByteArray& data, const QByteArray& palette, uint32_t width, uint32_t height);
    static QImage convertRGBATexture(const QByteArray& data, uint32_t width, uint32_t height, uint32_t format);
    
    // Helper functions for DXT decompression
    static void decompressDXTBlock(const uint8_t* block, uint32_t* output, bool isDXT1);
    static uint32_t interpolateColor(uint32_t c0, uint32_t c1, uint32_t t);
};

#endif // TXD_PARSER_H

