#ifndef GAMEOBJECT_GAMEOBJECT_H
#define GAMEOBJECT_GAMEOBJECT_H

#include "GameObjectCommon.h"
#include <typeindex>

namespace gameobjects
{
    class Component;

    /**
     * A game object is a container of components that provides access to its components as well as the components
     * of any child game objects that belong to it.
     *
     * Components cannot be added or removed from a game object once it has been created, however, game objects themselves
     * can be added/removed post creation via GameObjectController
     * Game objects and their components are serialized from a file (*.go) and are created via the GameObjectController.
     *
     * An example game object file 'example_gameobject.go':
     *
     * example_component_type_A MyExampleComponentA
     * {
     *     example_numeric_property = 123
     * }
     *
     * example_component_type_B MyExampleComponentB
     * {
     *     example_boolean_property = true
     * }
     *
     * child = example_gameobjectA
     * child = example_gameobjectB
     * child = example_gameobjectC
     */
    class GameObject : public INodeUserData
    {
        friend class GameObjectController;

    public:
        explicit GameObject();
        virtual ~GameObject();

        /** @script{ignore} */
        static int const GAMEOBJECT_NODE_USER_DATA_ID = 1;

        /** @script{ignore} */
        static GameObject * getGameObject(gameplay::Node * node);

#ifdef GP_SCENE_VISIT_EXTENSIONS
        void broadcastMessage(Message * message);
#endif

        /** @script{ignore} */
        gameplay::Node * getNode();

        /** @script{ignore} */
        GameObject * getParent() const;

        template<typename ComponentType>
        /** @script{ignore} */
        ComponentType * getComponent() const;

        template<typename ComponentType>
        /** @script{ignore} */
        void getComponents(std::vector<ComponentType*> & componentsOut) const;

        template<typename ComponentType>
        /** @script{ignore} */
        ComponentType * getComponentInChildren() const;

        template<typename ComponentType>
        /** @script{ignore} */
        void getComponentsInChildren(std::vector<ComponentType*> & componentsOut) const;

        template<typename ComponentType>
        /** @script{ignore} */
        ComponentType * findComponent(std::string const & id);

        /** @script{ignore} */
        virtual int getNodeUserDataId() const override;

        template<typename ComponentType>
        /** @script{ignore} */
        ComponentType * addComponent(std::string const & id);
    private:
        bool onMessageReceived(Message * message);
        void initialize();
        void finalize();
        void forEachComponent(std::function <bool(Component *)> func);
        void forEachComponent(std::function <bool(Component *)> func) const;
        static std::type_index const & getComponentTypeId(Component * component);
        static std::string const & getComponentId(Component * component);

        std::map<std::type_index, std::vector<Component*>> _components;
        gameplay::Node * _node;
    };
}

#include "GameObject.inl"

#endif
