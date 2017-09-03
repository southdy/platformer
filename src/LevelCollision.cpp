#include "Common.h"
#include "LevelCollision.h"
#include "GameObject.h"

namespace game
{
    namespace collision
    {
        static const int COLLISION_TYPE_ID = gameobjects::GameObject::GAMEOBJECT_NODE_USER_DATA_ID + 1;

        NodeData * NodeData::get(gameplay::Node * node)
        {
            return gameobjects::getNodeUserData<NodeData>(node, COLLISION_TYPE_ID);
        }

        int NodeData::getNodeUserDataId() const
        {
            return COLLISION_TYPE_ID;
        }

        Type::Enum fromString(std::string const & value)
        {
            Type::Enum type = Type::ALL;

            static std::map<std::string, collision::Type::Enum> lookup
            {
                {"STATIC", Type::STATIC},
                {"DYNAMIC", Type::DYNAMIC},
                {"LADDER", Type::LADDER},
                {"RESET", Type::RESET},
                {"COLLECTABLE", Type::COLLECTABLE},
                {"WATER", Type::WATER},
                {"BRIDGE", Type::BRIDGE},
                {"KINEMATIC", Type::KINEMATIC},
                {"PLAYER_PHYSICS", Type::PLAYER_PHYSICS},
                {"ENEMY", Type::ENEMY},
                {"ALL", Type::ALL}
            };

            auto itr = lookup.find(value);

            if(itr != lookup.end())
            {
                type = itr->second;
            }
            else
            {
                GAME_ASSERTFAIL("Unknown type %s", value.c_str());
            }

            return type;
        }
    }
}
