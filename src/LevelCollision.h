#ifndef GAME_TERRAIN_INFO_H
#define GAME_TERRAIN_INFO_H

#include "GameObjectCommon.h"

namespace game
{
    namespace collision
    {
        /**
         * The unique tile types that can be set per tile in a level.
         *
         * @script{ignore}
        */
        struct Type
        {
            enum Enum
            {
                STATIC = 1 << 0,
                DYNAMIC = 1 << 1,
                LADDER = 1 << 2,
                RESET = 1 << 3,
                COLLECTABLE = 1 << 4,
                WATER = 1 << 5,
                BRIDGE = 1 << 6,
                KINEMATIC = 1 << 7,
                PLAYER_PHYSICS = 1 << 8,
                ENEMY = 1 << 9,

                ALL = -1
            };
        };

        Type::Enum fromString(std::string const & value);

        /** @script{ignore} */
        class NodeData : public gameobjects::INodeUserData
        {
        public:
            static NodeData * get(gameplay::Node * node);

            int getNodeUserDataId() const override;

            Type::Enum _type;
        };
    }
}

#endif
