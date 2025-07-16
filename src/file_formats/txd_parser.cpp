#include "txd_parser.h"
#include <QFile>
#include <QDebug>

bool TXDParser::parse(QIODevice* device, QVector<GTATexture>& textures) {
    if (!device || !device->isOpen()) {
        qWarning() << "TXDParser: Invalid or closed device";
        return false;
    }
    
    QDataStream stream(device);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    RWChunk rootChunk;
    if (!readChunk(stream, rootChunk)) {
        qWarning() << "TXDParser: Failed to read root chunk";
        return false;
    }
    
    if (rootChunk.type != rwTEXDICTIONARY) {
        qWarning() << "TXDParser: Root chunk is not a TEXTURE DICTIONARY, got type:" << Qt::hex << rootChunk.type;
        return false;
    }
    
    return parseTextureDictionary(stream, rootChunk, textures);
}

bool TXDParser::parseFromFile(const QString& filePath, QVector<GTATexture>& textures) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "TXDParser: Failed to open file:" << filePath;
        return false;
    }
    
    bool result = parse(&file, textures);
    file.close();
    
    if (result) {
        qDebug() << "TXDParser: Successfully parsed" << filePath << "with" << textures.size() << "textures";
    }
    
    return result;
}

bool TXDParser::readChunk(QDataStream& stream, RWChunk& chunk) {
    if (stream.atEnd()) {
        return false;
    }
    
    stream >> chunk.type >> chunk.size >> chunk.version;
    
    if (stream.status() != QDataStream::Ok) {
        qWarning() << "TXDParser: Failed to read chunk header";
        return false;
    }
    
    return true;
}

bool TXDParser::parseTextureDictionary(QDataStream& stream, const RWChunk& chunk, QVector<GTATexture>& textures) {
    qint64 chunkEnd = stream.device()->pos() + chunk.size - 12; // Subtract header size
    
    // Read texture dictionary data
    RWChunk dataChunk;
    if (!readChunk(stream, dataChunk) || dataChunk.type != rwDATA) {
        qWarning() << "TXDParser: Expected DATA chunk in TEXTURE DICTIONARY";
        return false;
    }
    
    uint16_t textureCount;
    stream >> textureCount;
    skipChunk(stream, dataChunk);
    
    qDebug() << "TXDParser: Texture dictionary contains" << textureCount << "textures";
    
    // Parse textures
    while (stream.device()->pos() < chunkEnd && !stream.atEnd()) {
        RWChunk texChunk;
        if (!readChunk(stream, texChunk)) {
            break;
        }
        
        if (texChunk.type == rwTEXNATIVE) {
            GTATexture texture;
            if (parseTextureNative(stream, texChunk, texture)) {
                textures.append(texture);
            }
        } else {
            skipChunk(stream, texChunk);
        }
    }
    
    return true;
}

bool TXDParser::parseTextureNative(QDataStream& stream, const RWChunk& chunk, GTATexture& texture) {
    qint64 chunkEnd = stream.device()->pos() + chunk.size - 12;
    
    // Read texture native data
    RWChunk dataChunk;
    if (!readChunk(stream, dataChunk) || dataChunk.type != rwDATA) {
        qWarning() << "TXDParser: Expected DATA chunk in TEXTURE NATIVE";
        return false;
    }
    
    // Read platform ID (PC = 8, PS2 = 9, Xbox = 5)
    uint32_t platformId;
    stream >> platformId;
    
    if (platformId != 8 && platformId != 9 && platformId != 5) {
        qWarning() << "TXDParser: Unsupported platform ID:" << platformId;
        skipChunk(stream, dataChunk);
        return false;
    }
    
    // Read texture info
    uint32_t filterFlags, uAddressing, vAddressing;
    stream >> filterFlags >> uAddressing >> vAddressing;
    
    // Read texture name
    char nameBuffer[32];
    stream.readRawData(nameBuffer, 32);
    texture.name = QString::fromLatin1(nameBuffer).trimmed();
    
    // Read mask name
    char maskBuffer[32];
    stream.readRawData(maskBuffer, 32);
    texture.maskName = QString::fromLatin1(maskBuffer).trimmed();
    
    // Read raster format info
    uint32_t rasterFormat, d3dFormat;
    uint16_t width, height;
    uint8_t depth, mipmapCount, rasterType, compression;
    
    stream >> rasterFormat >> d3dFormat >> width >> height >> depth >> mipmapCount >> rasterType >> compression;
    
    texture.width = width;
    texture.height = height;
    texture.depth = depth;
    texture.format = rasterFormat;
    texture.mipmapCount = mipmapCount;
    texture.hasAlpha = (rasterFormat & 0x0500) != 0; // Check for alpha formats
    
    qDebug() << "TXDParser: Texture" << texture.name << "size:" << width << "x" << height 
             << "format:" << Qt::hex << rasterFormat << "mipmaps:" << mipmapCount;
    
    // Calculate data size
    uint32_t dataSize = width * height;
    if (depth == 32) {
        dataSize *= 4;
    } else if (depth == 24) {
        dataSize *= 3;
    } else if (depth == 16) {
        dataSize *= 2;
    } else if (depth == 8) {
        dataSize *= 1;
    } else if (depth == 4) {
        dataSize /= 2;
    }
    
    // Handle DXT compression
    if (compression == 1) { // DXT1
        dataSize = qMax(1U, (width + 3) / 4) * qMax(1U, (height + 3) / 4) * 8;
    } else if (compression == 3) { // DXT3
        dataSize = qMax(1U, (width + 3) / 4) * qMax(1U, (height + 3) / 4) * 16;
    } else if (compression == 5) { // DXT5
        dataSize = qMax(1U, (width + 3) / 4) * qMax(1U, (height + 3) / 4) * 16;
    }
    
    // Read texture data
    QByteArray textureData(dataSize, 0);
    stream.readRawData(textureData.data(), dataSize);
    
    // Convert to QImage based on format
    if (compression == 1) {
        texture.image = decompressDXT1(textureData, width, height);
    } else if (compression == 3) {
        texture.image = decompressDXT3(textureData, width, height);
    } else if (compression == 5) {
        texture.image = decompressDXT5(textureData, width, height);
    } else {
        // Handle uncompressed formats
        texture.image = convertRGBATexture(textureData, width, height, rasterFormat);
    }
    
    skipChunk(stream, dataChunk);
    
    // Parse child chunks (extensions)
    while (stream.device()->pos() < chunkEnd && !stream.atEnd()) {
        RWChunk childChunk;
        if (!readChunk(stream, childChunk)) {
            break;
        }
        skipChunk(stream, childChunk);
    }
    
    return !texture.image.isNull();
}

bool TXDParser::parseString(QDataStream& stream, const RWChunk& chunk, QString& str) {
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

void TXDParser::skipChunk(QDataStream& stream, const RWChunk& chunk) {
    qint64 bytesToSkip = chunk.size - 12; // Subtract header size
    if (bytesToSkip > 0) {
        stream.skipRawData(bytesToSkip);
    }
}

QImage TXDParser::decompressDXT1(const QByteArray& data, uint32_t width, uint32_t height) {
    QImage image(width, height, QImage::Format_RGBA8888);
    
    uint32_t blockWidth = (width + 3) / 4;
    uint32_t blockHeight = (height + 3) / 4;
    
    const uint8_t* blockData = reinterpret_cast<const uint8_t*>(data.constData());
    
    for (uint32_t by = 0; by < blockHeight; ++by) {
        for (uint32_t bx = 0; bx < blockWidth; ++bx) {
            uint32_t blockPixels[16];
            decompressDXTBlock(blockData + (by * blockWidth + bx) * 8, blockPixels, true);
            
            // Copy block to image
            for (int py = 0; py < 4 && (by * 4 + py) < height; ++py) {
                for (int px = 0; px < 4 && (bx * 4 + px) < width; ++px) {
                    uint32_t pixel = blockPixels[py * 4 + px];
                    image.setPixel(bx * 4 + px, by * 4 + py, pixel);
                }
            }
        }
    }
    
    return image;
}

QImage TXDParser::decompressDXT3(const QByteArray& data, uint32_t width, uint32_t height) {
    // Simplified DXT3 decompression - in practice, this would handle alpha properly
    return decompressDXT1(data, width, height);
}

QImage TXDParser::decompressDXT5(const QByteArray& data, uint32_t width, uint32_t height) {
    // Simplified DXT5 decompression - in practice, this would handle alpha properly
    return decompressDXT1(data, width, height);
}

QImage TXDParser::convertRGBATexture(const QByteArray& data, uint32_t width, uint32_t height, uint32_t format) {
    QImage image(width, height, QImage::Format_RGBA8888);
    
    const uint8_t* pixelData = reinterpret_cast<const uint8_t*>(data.constData());
    
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            uint32_t pixel = 0xFF000000; // Default alpha
            
            switch (format & 0x0F00) {
                case RASTER_8888:
                    if (data.size() >= static_cast<int>((y * width + x + 1) * 4)) {
                        uint32_t offset = (y * width + x) * 4;
                        pixel = (pixelData[offset + 3] << 24) |  // A
                               (pixelData[offset + 0] << 16) |  // R
                               (pixelData[offset + 1] << 8) |   // G
                               (pixelData[offset + 2]);        // B
                    }
                    break;
                case RASTER_888:
                    if (data.size() >= static_cast<int>((y * width + x + 1) * 3)) {
                        uint32_t offset = (y * width + x) * 3;
                        pixel = 0xFF000000 |                   // A
                               (pixelData[offset + 0] << 16) |  // R
                               (pixelData[offset + 1] << 8) |   // G
                               (pixelData[offset + 2]);        // B
                    }
                    break;
                case RASTER_565:
                    if (data.size() >= static_cast<int>((y * width + x + 1) * 2)) {
                        uint32_t offset = (y * width + x) * 2;
                        uint16_t pixel565 = (pixelData[offset + 1] << 8) | pixelData[offset + 0];
                        uint8_t r = ((pixel565 >> 11) & 0x1F) << 3;
                        uint8_t g = ((pixel565 >> 5) & 0x3F) << 2;
                        uint8_t b = (pixel565 & 0x1F) << 3;
                        pixel = 0xFF000000 | (r << 16) | (g << 8) | b;
                    }
                    break;
                default:
                    // Default to white for unsupported formats
                    pixel = 0xFFFFFFFF;
                    break;
            }
            
            image.setPixel(x, y, pixel);
        }
    }
    
    return image;
}

void TXDParser::decompressDXTBlock(const uint8_t* block, uint32_t* output, bool isDXT1) {
    // Read color endpoints
    uint16_t c0 = (block[1] << 8) | block[0];
    uint16_t c1 = (block[3] << 8) | block[2];
    
    // Convert to RGB
    uint8_t r0 = ((c0 >> 11) & 0x1F) << 3;
    uint8_t g0 = ((c0 >> 5) & 0x3F) << 2;
    uint8_t b0 = (c0 & 0x1F) << 3;
    
    uint8_t r1 = ((c1 >> 11) & 0x1F) << 3;
    uint8_t g1 = ((c1 >> 5) & 0x3F) << 2;
    uint8_t b1 = (c1 & 0x1F) << 3;
    
    // Create color palette
    uint32_t colors[4];
    colors[0] = 0xFF000000 | (r0 << 16) | (g0 << 8) | b0;
    colors[1] = 0xFF000000 | (r1 << 16) | (g1 << 8) | b1;
    
    if (c0 > c1 || !isDXT1) {
        // Interpolate colors
        uint8_t r2 = (2 * r0 + r1) / 3;
        uint8_t g2 = (2 * g0 + g1) / 3;
        uint8_t b2 = (2 * b0 + b1) / 3;
        colors[2] = 0xFF000000 | (r2 << 16) | (g2 << 8) | b2;
        
        uint8_t r3 = (r0 + 2 * r1) / 3;
        uint8_t g3 = (g0 + 2 * g1) / 3;
        uint8_t b3 = (b0 + 2 * b1) / 3;
        colors[3] = 0xFF000000 | (r3 << 16) | (g3 << 8) | b3;
    } else {
        // 1-bit alpha
        uint8_t r2 = (r0 + r1) / 2;
        uint8_t g2 = (g0 + g1) / 2;
        uint8_t b2 = (b0 + b1) / 2;
        colors[2] = 0xFF000000 | (r2 << 16) | (g2 << 8) | b2;
        colors[3] = 0x00000000; // Transparent
    }
    
    // Read indices
    uint32_t indices = (block[7] << 24) | (block[6] << 16) | (block[5] << 8) | block[4];
    
    // Decode pixels
    for (int i = 0; i < 16; ++i) {
        uint32_t index = (indices >> (i * 2)) & 0x3;
        output[i] = colors[index];
    }
}

