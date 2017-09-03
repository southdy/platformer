namespace gameobjects
{
    template<typename ComponentType>
    ComponentType * GameObject::getComponent() const
    {
        ComponentType * component = nullptr;

        auto componentItr = _components.find(typeid(ComponentType));

        if (componentItr != _components.end())
        {
            component = static_cast<ComponentType*>((*componentItr).second.front());
        }
            
        return component;
    }

    template<typename ComponentType>
    void GameObject::getComponents(std::vector<ComponentType*> & componentsOut) const
    {
        std::type_index id = typeid(ComponentType);

        forEachComponent([&componentsOut, &id](Component * component) -> bool
        {
            if (getComponentTypeId(component) == id)
            {
                componentsOut.push_back(static_cast<ComponentType*>(component));
            }
            
            return false;
        });
    }

    template<typename ComponentType>
    ComponentType * GameObject::getComponentInChildren() const
    {
        ComponentType * component = getComponent<ComponentType>();

        if (!component)
        {
            gameplay::Node * childNode = _node->getFirstChild();

            while (component == nullptr && childNode != nullptr)
            {
                if (GameObject * childGameObject = getGameObject(childNode))
                {
                    component = childGameObject->getComponentInChildren<ComponentType>();
                }

                childNode = childNode->getNextSibling();
            }
        }

        return component;
    }

    template<typename ComponentType>
    void GameObject::getComponentsInChildren(std::vector<ComponentType*> & componentsOut) const
    {
        getComponents<ComponentType>(componentsOut);

        gameplay::Node * childNode = _node->getFirstChild();

        while (childNode != nullptr)
        {
            if (GameObject * childGameObject = getGameObject(childNode))
            {
                childGameObject->getComponents<ComponentType>(componentsOut);
            }

            childNode = childNode->getNextSibling();
        }
    }

    template<typename ComponentType>
    ComponentType * GameObject::findComponent(std::string const & id)
    {
        ComponentType * componentToFind = nullptr;

        forEachComponent([&id, &componentToFind](Component * component) -> bool
        {
            if (getComponentId(component) == id)
            {
                componentToFind = static_cast<ComponentType*>(component);
                return true;
            }

            return false;
        });

        return componentToFind;
    }

    template<typename ComponentType>
    ComponentType * GameObject::addComponent(std::string const & id)
    {
        ComponentType * component = new ComponentType();
        component->_id = id;
        component->_typeId = typeid(ComponentType);
        component->_parent = this;
        _components[component->_typeId].push_back(component);
        component->initialize();
        component->onStart();
        return component;
    }
}
