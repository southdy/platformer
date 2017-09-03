#include "SpriteSheet.h"

#include "Common.h"
#include "Properties.h"
#include "PropertiesRef.h"
#include "ResourceManager.h"
#include "Texture.h"

namespace game
{
    SpriteSheet::SpriteSheet()
        : _texture(nullptr)
    {
    }

    SpriteSheet::~SpriteSheet()
    {
        SAFE_RELEASE(_texture);
    }

    void SpriteSheet::initialize(std::string const & filePath)
    {
        gameplay::PropertiesRef * propertyRef = ResourceManager::getInstance().getProperties(filePath.c_str());
        gameplay::Properties * properties = propertyRef->get();

        GAME_ASSERT(properties, "Failed to load sprite sheet %s", filePath.c_str());

        while (gameplay::Properties * currentNamespace = properties->getNextNamespace())
        {
            if (strcmp(currentNamespace->getNamespace(), "frames") == 0)
            {
                while (gameplay::Properties * frameNamespace = currentNamespace->getNextNamespace())
                {
                    Sprite sprite;
                    sprite._isRotated = frameNamespace->getBool("rotated");
                    sprite._name = frameNamespace->getString("filename");
                    gameplay::Vector4 frame;
                    frameNamespace->getVector4("frame", &frame);
                    sprite._src.x = frame.x;
                    sprite._src.y = frame.y;
                    sprite._src.width = frame.z;
                    sprite._src.height = frame.w;

                    GAME_ASSERT(_sprites.find(sprite._name) == _sprites.end(),
                        "Duplicate sprite name '%s' in '%s'", sprite._name.c_str(), filePath.c_str());

                    _sprites[sprite._name] = sprite;
                }
            }
            else if (strcmp(currentNamespace->getNamespace(), "meta") == 0)
            {
                std::string const texturePath = currentNamespace->getString("image");
                _texture = gameplay::Texture::create(texturePath.c_str());
                gameplay::Vector2 size;
                currentNamespace->getVector2("size", &size);
                int const width = size.x;
                int const height = size.y;
                GAME_ASSERT(_texture && _texture->getWidth() == width && _texture->getHeight() == height,
                    "Spritesheet '%s' width/height meta texture meta data is incorrect for '%s'", filePath.c_str(), texturePath.c_str());
            }
        }

        SAFE_RELEASE(propertyRef);
    }

    Sprite * SpriteSheet::getSprite(std::string const & spriteName)
    {
        auto itr = _sprites.find(spriteName);
        return itr != _sprites.end() ? &itr->second : nullptr;
    }

    void SpriteSheet::getSprites(std::string const & spritePrefix, std::vector<Sprite*> & spritesOut)
    {
        auto itr = _sprites.lower_bound(spritePrefix);
        while (itr != _sprites.end())
        {
            spritesOut.push_back(&itr->second);
            ++itr;
        }
    }

    gameplay::Rectangle const & SpriteSheet::getSrc()
    {
        return _src;
    }

    gameplay::Texture * SpriteSheet::getTexture()
    {
        return _texture;
    }

    std::string const & SpriteSheet::getName() const
    {
        return _name;
    }

    void SpriteSheet::forEachSprite(std::function<void(Sprite const &)> func)
    {
        for(auto & spritePair : _sprites)
        {
            func(spritePair.second);
        }
    }
}
