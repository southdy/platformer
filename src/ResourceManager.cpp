#include "ResourceManager.h"

#include "Common.h"
#include "ProfilerController.h"
#include "FileSystem.h"
#include "Font.h"
#include "Game.h"
#include "GameObjectController.h"
#include "LevelCollision.h"
#include "PropertiesRef.h"
#include "SpriteBatch.h"
#include "SpriteSheet.h"
#include "Texture.h"

namespace game
{
    static char const * PIXEL_TEXTURE_PATH = "pixel";

    ResourceManager::ResourceManager()
#ifndef _FINAL
        : _debugFont(nullptr)
#endif
    {
    }

    ResourceManager::~ResourceManager()
    {
    }

    ResourceManager::ResourceManager(ResourceManager const &)
    {
    }

    ResourceManager & ResourceManager::getInstance()
    {
        static ResourceManager instance;
        return instance;
    }

    void ResourceManager::initializeForBoot()
    {
        PROFILE();
#ifndef _FINAL
        if(gameplay::Properties * defaultUserConfig = gameplay::Properties::create("default.config"))
        {
            while(char const * propertyName = defaultUserConfig->getNextProperty())
            {
                getConfig()->setString(propertyName, defaultUserConfig->getString());
            }

#ifdef WIN32
            getConfig()->setString("os", "windows");
#else
            getConfig()->setString("os", "linux");
#endif

            SAFE_DELETE(defaultUserConfig);
        }

        if(gameplay::Properties * userConfig = gameplay::Properties::create("user.config"))
        {
            while(char const * propertyName = userConfig->getNextProperty())
            {
                getConfig()->setString(propertyName, userConfig->getString());
            }

            SAFE_DELETE(userConfig);
        }

        loadDebugFont();
#endif
        if(gameplay::Properties * mipMapNs = getConfig()->getNamespace("mip_maps", true))
        {
            while(char const * texturePath = mipMapNs->getNextProperty())
            {
                _mipMappedTextures.insert(texturePath);
            }
        }

        loadPixelSpritebatch();

        if(gameplay::Properties * bootNs = getConfig()->getNamespace("boot", true))
        {
            if(gameplay::Properties * bootTexturesNs = bootNs->getNamespace("textures", true))
            {
                while(char const * texturePath = bootTexturesNs->getNextProperty())
                {
                    cacheTexture(texturePath);
                }
            }

            if(gameplay::Properties * bootSpritesheetsNs = bootNs->getNamespace("spritesheets", true))
            {
                while(char const * spritesheetPath = bootSpritesheetsNs->getNextProperty())
                {
                    cacheProperties(spritesheetPath);
                    cacheSpriteSheet(spritesheetPath);
                }
            }
        }
    }

#ifndef _FINAL
    void ResourceManager::loadDebugFont()
    {
        PROFILE();
        std::string const fontPath = getConfig()->getString("font");
        _debugFont = gameplay::Font::create(fontPath.c_str());
        _debugFont->addRef();
    }
#endif

    void ResourceManager::loadPixelSpritebatch()
    {
        PROFILE();
        std::array<unsigned char, 4> rgba;
        rgba.fill(std::numeric_limits<unsigned char>::max());
        cacheTexture(PIXEL_TEXTURE_PATH, gameplay::Texture::create(gameplay::Texture::Format::RGBA, 1, 1, &rgba.front()));
    }

    gameplay::PropertiesRef * ResourceManager::loadProperties(std::string const & propertyPath)
    {
        return gameplay::PropertiesRef::create(gameplay::Properties::create(propertyPath.c_str()));
    }

    class GameObjectCallback : public gameobjects::GameObjectCallbackHandler
    {
    public:
        virtual gameplay::Properties * getProperties(const char * url) override
        {
            return ResourceManager::getInstance().getProperties(url)->get();
        }
    };

    void ResourceManager::initialize()
    {
        PROFILE();

        if (gameplay::Properties * aliases = getConfig()->getNamespace("aliases", true))
        {
            while(char const * path = aliases->getNextProperty())
            {
                if(strstr(aliases->getString(), "."))
                {
                    STALL_SCOPE();
                    cacheTexture("@" + std::string(path));
                }
            }

            aliases->rewind();
        }

        std::vector<std::string> fileList;
        if(gameplay::Properties * propertyDirNamespace = getConfig()->getNamespace("properties_directories", true))
        {
            while(char const * dir = propertyDirNamespace->getNextProperty())
            {
                fileList.clear();
                gameplay::FileSystem::listFiles(dir, fileList);

                for(std::string & propertyUrl : fileList)
                {
                    std::string const propertyPath = std::string(dir) + std::string("/") + propertyUrl;
                    gameplay::PropertiesRef * propertiesRef = getProperties(propertyPath);
                    bool const usesTopLevelNamespaceUrls = propertyDirNamespace->getBool();

                    if (!propertiesRef)
                    {
                        STALL_SCOPE();
                        propertiesRef = loadProperties(propertyPath);

                        if (!usesTopLevelNamespaceUrls)
                        {
                            propertiesRef->addRef();
                        }
                    }
                    else
                    {
                        propertiesRef->release();
                    }

                    if(usesTopLevelNamespaceUrls)
                    {
                        gameplay::Properties * properties = propertiesRef->get();

                        while(gameplay::Properties * topLevelChildNS = properties->getNextNamespace())
                        {
                            STALL_SCOPE();
                            cacheProperties(propertyPath + std::string("#") + topLevelChildNS->getId());
                        }

                        SAFE_RELEASE(propertiesRef);
                    }
                    else
                    {
                        _cachedProperties[propertyPath] = propertiesRef;
                    }
                }
            }
        }

        fileList.clear();
        std::string const spriteSheetDirectory = "res/spritesheets";
        gameplay::FileSystem::listFiles(spriteSheetDirectory.c_str(), fileList);

        for (std::string & fileName : fileList)
        {
            STALL_SCOPE();
            cacheSpriteSheet(spriteSheetDirectory + "/" + fileName);
        }

        for(auto & propertyCachePair : _cachedProperties)
        {
            if(propertyCachePair.first.find("/physics/") != std::string::npos)
            {
                gameplay::Properties * ns = propertyCachePair.second->get();

                while(char const * propertyName = ns->getNextProperty())
                {
                    if(strcmp(propertyName, "group") == 0 || strcmp(propertyName, "mask") == 0 && ns->getType(propertyName) != gameplay::Properties::Type::NUMBER)
                    {
                        std::stringstream ss(ns->getString());
                        std::string value;
                        int collisionValue = 0;
                        while (std::getline(ss, value, '|'))
                        {
                            collisionValue |= collision::fromString(value);
                        }
                        ns->setString(propertyName, toString(collisionValue).c_str());
                    }
                }

                ns->rewind();
            }
        }

        gameobjects::GameObjectController::getInstance().registerCallbackHandler(new GameObjectCallback());
    }

    void ResourceManager::cacheTexture(std::string const & texturePath)
    {
        if(_cachedTextures.find(texturePath) == _cachedTextures.end())
        {
            PROFILE();
            gameplay::Texture * texture = gameplay::Texture::create(texturePath.c_str(),
                _mipMappedTextures.find(texturePath) != _mipMappedTextures.end());
            texture->addRef();
            _cachedTextures[texturePath] = texture;
        }
    }

    void ResourceManager::cacheTexture(std::string const & texturePath, gameplay::Texture * texture)
    {
        texture->addRef();
        _cachedTextures[texturePath] = texture;
    }

    void ResourceManager::cacheSpriteSheet(std::string const & spritesheetPath)
    {
        if(_cachedSpriteSheets.find(spritesheetPath) == _cachedSpriteSheets.end())
        {
            PROFILE();
            SpriteSheet * spriteSheet = new SpriteSheet();
            spriteSheet->initialize(spritesheetPath);
            spriteSheet->addRef();
            _cachedSpriteSheets[spritesheetPath] = spriteSheet;
        }
    }

    void ResourceManager::cacheProperties(std::string const & propertiesPath)
    {
        if(_cachedProperties.find(propertiesPath) == _cachedProperties.end())
        {
            PROFILE();
            gameplay::PropertiesRef * childPropertiesRef = gameplay::PropertiesRef::create(gameplay::Properties::create(propertiesPath.c_str()));
            childPropertiesRef->addRef();
            _cachedProperties[propertiesPath] = childPropertiesRef;
        }
    }

    void releaseCacheRefs(gameplay::Ref * ref)
    {
        if (ref)
        {
            for(int i = 0; i < 2; ++i)
            {
                ref->release();
            }
        }
    }

    void ResourceManager::finalize()
    {
        for(auto pair : _cachedProperties)
        {
            releaseCacheRefs(pair.second);

            if(pair.first.find("/gameobjects/") != std::string::npos)
            {
                pair.second->release();
            }
        }

        for(auto pair : _cachedSpriteSheets)
        {
            releaseCacheRefs(pair.second);
        }

        for(auto pair : _cachedTextures)
        {
            releaseCacheRefs(pair.second);
        }

        _cachedProperties.clear();
        _cachedSpriteSheets.clear();
        _cachedTextures.clear();

#ifndef _FINAL
        releaseCacheRefs(_debugFont);
#endif
    }

    gameplay::PropertiesRef * ResourceManager::getProperties(std::string const & url) const
    {
        auto itr = _cachedProperties.find(url);

        if(itr != _cachedProperties.end())
        {
            itr->second->addRef();
            return itr->second;
        }

        return nullptr;
    }

    SpriteSheet * ResourceManager::getSpriteSheet(std::string const & url) const
    {
        auto itr = _cachedSpriteSheets.find(url);

        if(itr != _cachedSpriteSheets.end())
        {
            itr->second->addRef();
            return itr->second;
        }

        return nullptr;
    }

    gameplay::SpriteBatch * ResourceManager::createSinglePixelSpritebatch()
    {
        return gameplay::SpriteBatch::create(_cachedTextures[PIXEL_TEXTURE_PATH]);
    }

#ifndef _FINAL
    gameplay::Font * ResourceManager::getDebugFront() const
    {
        return _debugFont;
    }
#endif
}
