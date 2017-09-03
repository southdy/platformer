#ifndef GAMEOBJECT_GAMEOBJECTCONTROLLER_H
#define GAMEOBJECT_GAMEOBJECTCONTROLLER_H

#include "GameObjectCommon.h"
#include <typeindex>

#include "GameObject.h"

namespace gameobjects
{
    /**
     * Defines a class for creating, destroying and broadcasting messages to game objects. All component
     * types must be registered with this controller in order to be serializable from game object files.
     *
     * The Game class should be responsible for forwarding platform level events to game objects as well as
     * invoking the various update and render methods.
     */
    class GameObjectController
    {
        friend class ScopedGameObjectCallback;

    public:
        /**
         * @return GameObjectController
         */
        static GameObjectController & getInstance();

        /** @script{ignore} */
        void initialize();

        /** @script{ignore} */
        void finalize();

        /** @script{ignore} */
        void broadcastMessage(Message * message);

#ifdef GP_SCENE_VISIT_EXTENSIONS
        /** @script{ignore} */
        void broadcastMessage(Message * message, GameObject * gameObject);
#endif

        /** @script{ignore} */
        void update(float elapsedTime);

        /**
         * @param gameObject
         */
        void destroyGameObject(GameObject * gameObject);

        /**
         * @param parent
         */
        GameObject * createGameObject(GameObject * parent);

        /**
         * @param typeName
         */
        GameObject * createGameObject(std::string const & typeName);

        /**
         * @param typeName
         * @param parent
         * @return GameObject
         */
        GameObject * createGameObject(std::string const & typeName, GameObject * parent);

        template<typename ComponentType>
        /** @script{ignore} */
        void registerComponent(std::string const & name);

        /** @script{ignore} */
        gameplay::Scene * getScene() const;

        /** @script{ignore} */
        void registerCallbackHandler(GameObjectCallbackHandler * handler);
    private:
        struct ScopedGameObjectCallback
        {
            ScopedGameObjectCallback();
            ~ScopedGameObjectCallback();
        };

        explicit GameObjectController();
        ~GameObjectController();
        GameObjectController(GameObjectController const &);

        bool removeGameObject(gameplay::Node * node);
        void removeGameObject(GameObject * gameObject);
        bool broadcastMessage(gameplay::Node * node, Message * message);
        void preGameObjectCallbacks();
        void postGameObjectCallbacks();
        GameObject * createGameObject(std::string const & typeName, gameplay::Node * parentNode, bool definitionExists = true);

        struct ComponentTypeInfo
        {
            std::string _name;
            std::function<Component*()> _generator;
        };

        struct GameObjectTypeInfo
        {
            std::vector<std::pair<std::type_index, gameplay::Properties *>> _components;
            std::vector<std::string> _gameObjects;
            gameplay::Properties * _definition;
        };

        gameplay::Scene * _scene;
        std::string _gameObjectTypeDir;
        std::map<std::type_index, ComponentTypeInfo> _componentTypes;
        std::map<std::string, GameObjectTypeInfo> _gameObjectTypes;
        std::set<GameObject *> _gameObjectsToRemove;
        bool _processingGameObjectCallbacks;
        GameObjectCallbackHandler * _callbackHandler;
    };
}

#include "GameObjectController.inl"

#endif
