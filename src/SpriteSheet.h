#ifndef GAME_SPRITESHEET_H
#define GAME_SPRITESHEET_H

#include <functional>
#include <map>
#include "Rectangle.h"
#include "Ref.h"
#include "Sprite.h"
#include <string>
#include <vector>

namespace gameplay
{
    class Texture;
}

namespace game
{
    /**
     * Loads a sprite sheet from a file (*.ss)
     *
     * Spritesheets are created in [TexturePacker], exported as JSON and then converted to the
     * gameplay property format using [Json2gp3d]
     *
     * [TexturePacker]  Download @ http://www.codeandweb.com/texturepacker/download
     * [Json2gp3d]      Download @ https://github.com/louis-mclaughlin/json-to-gameplay3d
     *
     * @script{ignore}
    */
    class SpriteSheet : public gameplay::Ref
    {
        friend class ResourceManager;

    public:
        ~SpriteSheet();

        Sprite * getSprite(std::string const & spriteName);
        void getSprites(std::string const & spritePrefix, std::vector<Sprite*> & spritesOut);
        gameplay::Rectangle const & getSrc();
        gameplay::Texture * getTexture();
        std::string const & getName() const;
        void forEachSprite(std::function<void(Sprite const &)> func);
    private:
        explicit SpriteSheet();
        SpriteSheet(SpriteSheet const &);

        void initialize(std::string const & filePath);

        gameplay::Rectangle _src;
        gameplay::Texture * _texture;
        std::string _name;
        std::map<std::string, Sprite> _sprites;
    };
}

#endif
