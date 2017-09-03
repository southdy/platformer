#include "GameObjectController.h"

#include "Component.h"
#include "FileSystem.h"
#include "Scene.h"
#ifdef GP_USE_PROFILER
#include "ProfilerController.h"
#endif

#ifndef PROFILE
#define PROFILE()
#endif

namespace gameobjects
{
    #define GAMEOBJECT_CALLBACK_SCOPE ScopedGameObjectCallback scopedGameObjectCallback;

    GameObjectController::GameObjectController()
        : _gameObjectTypeDir("res/gameobjects")
        , _scene(nullptr)
        , _processingGameObjectCallbacks(false)
        , _callbackHandler(nullptr)
    {
    }

    GameObjectController:: ~GameObjectController()
    {
    }

    GameObjectController & GameObjectController::getInstance()
    {
        static GameObjectController instance;
        return instance;
    }

    void GameObjectController::initialize()
    {
        PROFILE();
        GAMEOBJECT_ASSERT(!_scene, "GameObjectController has already been initialized");
        _scene = gameplay::Scene::create();

        std::vector<std::string> files;
        gameplay::FileSystem::listFiles(_gameObjectTypeDir.c_str(), files);

        for (std::string const & file : files)
        {
            std::string const path = _gameObjectTypeDir + "/" + file;
            
            gameplay::Properties * gameObjectDef = nullptr;

            if(_callbackHandler)
            {
                gameObjectDef = _callbackHandler->getProperties(path.c_str());
            }
            else
            {
                gameObjectDef = gameplay::Properties::create(path.c_str());
            }

            GAMEOBJECT_ASSERT(_gameObjectTypes.find(file) == _gameObjectTypes.end(), "Duplicate gameObject definition file '%s' found", path.c_str());

            GameObjectTypeInfo & gameObjectTypeInfo = _gameObjectTypes[file];

            while (gameObjectDef->getNextProperty())
            {
                gameObjectTypeInfo._gameObjects.push_back(gameObjectDef->getString());
            }

            gameObjectDef->rewind();

            while (gameplay::Properties * componentDef = gameObjectDef->getNextNamespace())
            {
                bool componentTypeFound = false;

                for (auto & componentTypePair : _componentTypes)
                {
                    if (componentTypePair.second._name == componentDef->getNamespace())
                    {
                        gameObjectTypeInfo._components.push_back(std::make_pair(componentTypePair.first, componentDef));
                        componentTypeFound = true;
                        break;
                    }
                }

                GAMEOBJECT_ASSERT(componentTypeFound, "Unknown component type '%s'", componentDef->getNamespace());
            }

            gameObjectTypeInfo._definition = gameObjectDef;
        }
    }

    void GameObjectController::finalize()
    {
        if(_scene)
        {
            gameplay::Node * child = _scene->getFirstNode();
            while (child)
            {
                gameplay::Node * sibling = child->getNextSibling();
                removeGameObject(child);
                child = sibling;
            }
            SAFE_RELEASE(_scene);

            for (auto & gameObjectTypePair : _gameObjectTypes)
            {
                GameObjectTypeInfo & gameObjectTypeInfo = gameObjectTypePair.second;
                if(!_callbackHandler)
                {
                    SAFE_DELETE(gameObjectTypeInfo._definition);
                }
            }

            SAFE_RELEASE(_callbackHandler);
            _gameObjectTypes.clear();
            _componentTypes.clear();
        }
    }

    void GameObjectController::broadcastMessage(Message * message)
    {
        if (_scene)
        {
            GAMEOBJECT_CALLBACK_SCOPE
            _scene->visit(this, &GameObjectController::broadcastMessage, message);
        }
    }

#ifdef GP_SCENE_VISIT_EXTENSIONS
    void GameObjectController::broadcastMessage(Message * message, GameObject * gameObject)
    {
        gameObject->onMessageReceived(message);
        gameplay::Node * firstGameObjectChild = gameObject->getNode()->getFirstChild();
        if (_scene && firstGameObjectChild)
        {
            GAMEOBJECT_CALLBACK_SCOPE
            _scene->visit(this, &GameObjectController::broadcastMessage, message, firstGameObjectChild);
        }
    }
#endif

    GameObject * GameObjectController::createGameObject(GameObject * parent)
    {
        return createGameObject("", parent->getNode(), false);
    }

    GameObject * GameObjectController::createGameObject(std::string const & typeName)
    {
        gameplay::Node * nullNode = nullptr;
        return createGameObject(typeName, nullNode);
    }

    GameObject * GameObjectController::createGameObject(std::string const & typeName, GameObject * parent)
    {
        return createGameObject(typeName, parent->getNode());
    }

    GameObject * GameObjectController::createGameObject(std::string const & typeName, gameplay::Node * parentNode, bool definitionExists)
    {
        auto gameObjectTypeItr = _gameObjectTypes.find(typeName + ".go");
		bool const definitionFound = gameObjectTypeItr != _gameObjectTypes.end();
        GAMEOBJECT_ASSERT(!definitionExists || definitionFound, "Unknown gameObject type '%s", typeName.c_str());
		GameObjectTypeInfo * gameObjectDef = nullptr;
		
		if (definitionFound)
		{
			gameObjectDef = &gameObjectTypeItr->second;
		}
        
        GameObject * gameObject = new GameObject();
        std::string name = definitionExists ? typeName : "annon";
        gameplay::Node * node = gameplay::Node::create(name.c_str());
        node->setUserObject(gameObject);
        parentNode ? parentNode->addChild(node) : _scene->addNode(node);
        node->release();
        gameObject->_node = node;

		if (gameObjectDef)
		{
			for (auto & gameObjectDefPair : gameObjectDef->_components)
			{
				auto componentTypeItr = _componentTypes.find(gameObjectDefPair.first);
				ComponentTypeInfo const & componentTypeInfo = componentTypeItr->second;
				Component * component = componentTypeInfo._generator();
				gameplay::Properties & properties = *gameObjectDefPair.second;
				std::string id = properties.getId();

				if (id.empty())
				{
					char buffer[UCHAR_MAX];
					sprintf(buffer, "annon_%d", static_cast<int>(gameObject->_components.size()));
					id = buffer;
				}

				component->_id = id;
				component->readProperties(*gameObjectDefPair.second);
				component->_typeId = gameObjectDefPair.first;
				component->_parent = gameObject;
				component->initialize();
				gameObject->_components[component->_typeId].push_back(component);
			}

			for (std::string & gameObjectTypeName : gameObjectDef->_gameObjects)
			{
				createGameObject(gameObjectTypeName, node);
			}

			gameObject->forEachComponent([](Component * component) -> bool
			{
				component->onStart();
				return false;
			});
		}

        return  gameObject;
    }

    void GameObjectController::destroyGameObject(GameObject * gameObject)
    {
        removeGameObject(gameObject);
    }

    bool GameObjectController::removeGameObject(gameplay::Node * node)
    {
        if (GameObject * gameObject = GameObject::getGameObject(node))
        {
            removeGameObject(gameObject);
        }
        return true;
    }

    void GameObjectController::removeGameObject(GameObject * gameObject)
    {
        if(!_processingGameObjectCallbacks)
        {
            gameplay::Node * node = gameObject->_node;
            if (gameplay::Node * child = node->getFirstChild())
            {
                while (child)
                {
                    gameplay::Node * sibling = child->getNextSibling();
                    if (GameObject * childGameObject = GameObject::getGameObject(child))
                    {
                        removeGameObject(childGameObject);
                    }
                    child = sibling;
                }
            }
            node->addRef();
            gameObject->finalize();
            node->setUserObject(nullptr);

            if (node->getParent())
            {
                node->getParent()->removeChild(node);
            }
            else
            {
                _scene->removeNode(node);
            }
            SAFE_RELEASE(gameObject);
        }
        else
        {
            _gameObjectsToRemove.insert(gameObject);
        }
    }

    void GameObjectController::preGameObjectCallbacks()
    {
        _processingGameObjectCallbacks = true;
    }

    void GameObjectController::postGameObjectCallbacks()
    {
        _processingGameObjectCallbacks = false;

        if(! _gameObjectsToRemove.empty())
        {
            for(auto gameObjectToRemoveItr = _gameObjectsToRemove.begin(); gameObjectToRemoveItr != _gameObjectsToRemove.end(); ++gameObjectToRemoveItr)
            {
                removeGameObject(*gameObjectToRemoveItr);
            }

            _gameObjectsToRemove.clear();
        }
    }

    bool GameObjectController::broadcastMessage(gameplay::Node * node, Message * message)
    {
        if (GameObject * gameObject = GameObject::getGameObject(node))
        {
            return gameObject->onMessageReceived(message);
        }

        return true;
    }

    gameplay::Scene * GameObjectController::getScene() const
    {
        return _scene;
    }

    GameObjectController::ScopedGameObjectCallback::ScopedGameObjectCallback()
    {
        GameObjectController::getInstance().preGameObjectCallbacks();
    }

    GameObjectController::ScopedGameObjectCallback::~ScopedGameObjectCallback()
    {
        GameObjectController::getInstance().postGameObjectCallbacks();
    }

    void GameObjectController::registerCallbackHandler(GameObjectCallbackHandler * handler)
    {
        if(_callbackHandler)
        {
            SAFE_RELEASE(_callbackHandler);
        }

        _callbackHandler = handler;
    }
}
