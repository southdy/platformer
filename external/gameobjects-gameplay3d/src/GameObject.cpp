#include "GameObject.h"
#include "GameObjectController.h"

#include "Component.h"

namespace gameobjects
{
    GameObject::GameObject()
        : _node(nullptr)
    {
    }

    GameObject::~GameObject(){}

    void GameObject::initialize()
    {
        for (auto & componentPair : _components)
        {
            for (Component * component : componentPair.second)
            {
                component->initialize();
            }
        }
    }

    void GameObject::finalize()
    {
        for (auto & componentPair : _components)
        {
            for (Component * component : componentPair.second)
            {
                component->finalize();
            }
        }

        for (auto & componentPair : _components)
        {
            for (Component * component : componentPair.second)
            {
                SAFE_RELEASE(component);
            }
        }

        SAFE_RELEASE(_node);
    }

    bool GameObject::onMessageReceived(Message * message)
    {
        for (auto & componentPair : _components)
        {
            for (Component * component : componentPair.second)
            {
                if(!component->onMessageReceived(message, message->getId()))
                {
                    return false;
                }
            }
        }

        return true;
    }

    void GameObject::forEachComponent(std::function <bool(Component *)> func)
    {
        for (auto & componentPair : _components)
        {
            for (Component * component : componentPair.second)
            {
                if (func(component))
                {
                    break;
                }
            }
        }
    }

    void GameObject::forEachComponent(std::function <bool(Component *)> func) const
    {
        for (auto & componentPair : _components)
        {
            for (Component * component : componentPair.second)
            {
                if (func(component))
                {
                    break;
                }
            }
        }
    }

#ifdef GP_SCENE_VISIT_EXTENSIONS
    void GameObject::broadcastMessage(Message * message)
    {
        GameObjectController::getInstance().broadcastMessage(message, this);
    }
#endif

    GameObject * GameObject::getParent() const
    {
        GameObject * parentGameObject = nullptr;
        gameplay::Node * parentNode = _node->getParent();

        if(parentNode)
        {
            parentGameObject = getGameObject(parentNode);
        }

        return parentGameObject;
    }

    gameplay::Node * GameObject::getNode()
    {
        return _node;
    }

    std::type_index const & GameObject::getComponentTypeId(Component * component)
    {
        return component->_typeId;
    }

    std::string const & GameObject::getComponentId(Component * component)
    {
        return component->_id;
    }

    int GameObject::getNodeUserDataId() const
    {
        return GAMEOBJECT_NODE_USER_DATA_ID;
    }

    GameObject * GameObject::getGameObject(gameplay::Node * node)
    {
        return getNodeUserData<GameObject>(node, GAMEOBJECT_NODE_USER_DATA_ID);
    }
}
