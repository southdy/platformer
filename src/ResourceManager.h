#ifndef GAME_RESOURCE_MANAGER
#define GAME_RESOURCE_MANAGER

#include <map>
#include <functional>
#include <set>
#include <string>
#include <vector>
#include "Ref.h"

namespace gameplay
{
    class Properties;
    class PropertiesRef;
    class Texture;
    class SpriteBatch;
#ifndef _FINAL
    class Font;
#endif
}

namespace game
{
    class SpriteSheet;

    /** @script{ignore} */
    class ResourceManager
    {
    public:
        static ResourceManager & getInstance();

        void initializeForBoot();
        void initialize();
        void finalize();

        gameplay::SpriteBatch * createSinglePixelSpritebatch();
        gameplay::PropertiesRef * getProperties(std::string const & url) const;
        SpriteSheet * getSpriteSheet(std::string const & url) const;

#ifndef _FINAL
        gameplay::Font * getDebugFront() const;
#endif
    private:
        explicit ResourceManager();
        ~ResourceManager();
        ResourceManager(ResourceManager const &);

        void cacheTexture(std::string const & texturePath);
        void cacheTexture(std::string const & texturePath, gameplay::Texture * texture);
        void cacheSpriteSheet(std::string const & spritesheetPath);
        void cacheProperties(std::string const & propertiesPath);
        void loadPixelSpritebatch();
        gameplay::PropertiesRef * loadProperties(std::string const & propertyPath);

        std::map<std::string, gameplay::Texture *> _cachedTextures;
        std::map<std::string, gameplay::PropertiesRef *> _cachedProperties;
        std::map<std::string, SpriteSheet *> _cachedSpriteSheets;
        std::set<std::string> _mipMappedTextures;
        std::vector<std::function<void()>> _slowTasks;

#ifndef _FINAL
        void loadDebugFont();
        gameplay::Font * _debugFont;
#endif
    };
}

#endif
