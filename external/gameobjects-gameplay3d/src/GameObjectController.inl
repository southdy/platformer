namespace gameobjects
{
    template<typename ComponentType>
    void GameObjectController::registerComponent(std::string const & name)
    {
        std::type_index const id = typeid(ComponentType);

        GAMEOBJECT_ASSERT(_componentTypes.find(id) == _componentTypes.end(), "Component type '%s' has already been registered", name.c_str());

        _componentTypes[id]._generator = []()
        {
            return new ComponentType();
        };

        _componentTypes[id]._name = name;
    }
}
