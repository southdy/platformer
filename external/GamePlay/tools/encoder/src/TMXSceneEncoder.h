#ifndef TMXSCENEEENCODER_H_
#define TMXSCENEEENCODER_H_

#include <fstream>
#include <unordered_map>
#include <tinyxml2.h>

#include "Base.h"
#include "Vector2.h"
#include "TMXTypes.h"
#include "EncoderArguments.h"
#include "Image.h"

/**
 * Class for encoding an TMX file.
 */
class TMXSceneEncoder
{
public:
    /**
     * Constructor.
     */
    TMXSceneEncoder();

    /**
     * Destructor.
     */
    ~TMXSceneEncoder();

    /**
     * Writes out encoded TMX file.
     */
    void write(const tools::EncoderArguments& arguments);

private:
    static std::vector<unsigned int> loadDataElement(const tinyxml2::XMLElement* data);
    static inline std::string buildFilePath(const std::string& directory, const std::string& file);
    static void copyImage(unsigned char* dst, const unsigned char* src,
        unsigned int srcWidth, unsigned int dstWidth, unsigned int bpp,
        unsigned int srcx, unsigned int srcy, unsigned int dstx, unsigned int dsty, unsigned int width, unsigned int height);

    // Parsing
    bool parseTmx(const tinyxml2::XMLDocument& xmlDoc, tools::TMXMap& map, const std::string& inputDirectory) const;
    void parseBaseLayerProperties(const tinyxml2::XMLElement* xmlBaseLayer, tools::TMXBaseLayer* layer) const;

    // Gutter
    void buildTileGutter(tools::TMXMap& map, const std::string& inputDirectory, const std::string& outputDirectory);
    bool buildTileGutterTileset(const tools::TMXTileSet& tileset, const std::string& inputFile, const std::string& outputFile);

    // Writing
    void writeScene(const tools::TMXMap& map, const std::string& outputFilepath, const std::string& sceneName);

    void writeTileset(const tools::TMXMap& map, const tools::TMXLayer* layer, std::ofstream& file);
    void writeSoloTileset(const tools::TMXMap& map, const tools::TMXTileSet& tmxTileset, const tools::TMXLayer& tileset, std::ofstream& file, unsigned int resultOnlyForTileset = TMX_INVALID_ID);

    void writeSprite(const tools::TMXImageLayer* imageLayer, std::ofstream& file);

    void writeNodeProperties(bool enabled, std::ofstream& file, bool seperatorLineWritten = false);
    void writeNodeProperties(bool enabled, const tools::TMXProperties& properties, std::ofstream& file, bool seperatorLineWritten = false);
    void writeNodeProperties(bool enabled, const tools::Vector2& pos, std::ofstream& file, bool seperatorLineWritten = false);
    void writeNodeProperties(bool enabled, const tools::Vector2& pos, const tools::TMXProperties& properties, std::ofstream& file, bool seperatorLineWritten = false);

    void writeLine(std::ofstream& file, const std::string& line) const;

    unsigned int _tabCount;
};

inline void TMXSceneEncoder::writeNodeProperties(bool enabled, std::ofstream& file, bool seperatorLineWritten)
{
    writeNodeProperties(enabled, tools::Vector2::zero(), file, seperatorLineWritten);
}
inline void TMXSceneEncoder::writeNodeProperties(bool enabled, const tools::TMXProperties& properties, std::ofstream& file, bool seperatorLineWritten)
{
    writeNodeProperties(enabled, tools::Vector2::zero(), properties, file, seperatorLineWritten);
}
inline void TMXSceneEncoder::writeNodeProperties(bool enabled, const tools::Vector2& pos, std::ofstream& file, bool seperatorLineWritten)
{
    tools::TMXProperties noOp;
    writeNodeProperties(enabled, pos, noOp, file, seperatorLineWritten);
}

#endif
