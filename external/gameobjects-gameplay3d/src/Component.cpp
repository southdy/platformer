#include "Component.h"

#include "GameObject.h"
#include "GameObjectController.h"

namespace gameobjects
{
    Component::Component()
        : _typeId(typeid(Component))
        , _parent(nullptr)
    {
    }

    GameObject * Component::getParent()
    {
        return _parent;
    }

    GameObject * Component::getRootParent()
    {
        gameplay::Node * node = _parent->getNode();

        while (true)
        {
            gameplay::Node * parent = node->getParent();

            if (parent)
            {
                node = parent;
            }
            else
            {
                break;
            }
        }

        return GameObject::getGameObject(node);
    }

    std::string const & Component::getId() const
    {
        return _id;
    }
}
