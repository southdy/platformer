#ifndef GAME_SPRITE_H
#define GAME_SPRITE_H

#include "Rectangle.h"
#include <string>

namespace game
{
    /** @script{ignore} */
    struct Sprite
    {
        struct Flip
        {
            enum Enum
            {
                None = 1 << 0,
                Vertical = 1 << 1,
                Horizontal = 1 << 2
            };
        };

        std::string _name;
        gameplay::Rectangle _src;
        bool _isRotated;
    };
}

#endif
