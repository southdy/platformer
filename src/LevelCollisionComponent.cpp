#include "LevelCollisionComponent.h"

#include "Common.h"
#include "EnemyComponent.h"
#include "GameObject.h"
#include "PlayerComponent.h"
#include "Messages.h"
#include "PhysicsCharacter.h"
#include "LevelCollision.h"

namespace game
{
    LevelCollisionComponent::LevelCollisionComponent()
        : _player(nullptr)
        , _playerClimbingTerrainRefCount(0)
        , _playerSwimmingRefCount(0)
        , _waitForPhysicsCleanup(false)
        , _framesSinceLevelReloaded(0)
        , _playerResetMessage(nullptr)
        , _enemyKilledMessage(nullptr)
        , _playerNode(nullptr)
        , _playerCollisionListener(nullptr)
        , _character(nullptr)
        , _level(nullptr)
    {
    }

    LevelCollisionComponent::~LevelCollisionComponent()
    {
    }

    bool LevelCollisionComponent::onMessageReceived(gameobjects::Message * message, int messageType)
    {
        switch (messageType)
        {
        case(Messages::Type::LevelLoaded):
            onLevelUnloaded();
            onLevelLoaded();
            break;
        case(Messages::Type::PreLevelUnloaded):
        case(Messages::Type::LevelUnloaded):
            onLevelUnloaded();
            break;
        case(Messages::Type::PostSimulationUpdate):
            onPostSimulationUpdate();
            break;
        }

        return true;
    }

    void addOrRemoveCollisionListener(collision::Type::Enum collisionType,
                                      gameplay::PhysicsRigidBody::CollisionListener * listener,
                                      LevelLoaderComponent * level,
                                      gameplay::PhysicsCollisionObject * collisionObject,
                                      bool add)
    {
        level->forEachCachedNode(collisionType, [&add, listener, &collisionObject](gameplay::Node * node)
        {
            if(add)
            {
                collisionObject->addCollisionListener(listener, node->getCollisionObject());
            }
            else
            {
                collisionObject->removeCollisionListener(listener, node->getCollisionObject());
            }

        });
    }

    void LevelCollisionComponent::onLevelLoaded()
    {
        _playerClimbingTerrainRefCount = 0;
        _playerSwimmingRefCount = 0;
        _waitForPhysicsCleanup = true;
        std::vector<EnemyComponent *> enemyComponents;
        getParent()->getComponentsInChildren(enemyComponents);

        _player = getParent()->getComponentInChildren<PlayerComponent>();
        _player->addRef();
        _playerNode = _player->getNode();
        _playerNode->addRef();
        _character = _player->getCharacter();
        _character->setGhostCollisionCallback([this](gameplay::PhysicsCollisionObject * object, gameplay::Vector3 collisionNormal)
        {
            onCharacterCollision(object->getNode(), collisionNormal);
        });
        _level = getParent()->getComponent<LevelLoaderComponent>();
        _level->addRef();
        _collectables.clear();
        _level->getCollectables(_collectables);
        addOrRemoveCollisionListener(collision::Type::LADDER, _playerCollisionListener, _level, _playerNode->getCollisionObject(), true);
        addOrRemoveCollisionListener(collision::Type::RESET, _playerCollisionListener, _level, _playerNode->getCollisionObject(), true);
        addOrRemoveCollisionListener(collision::Type::COLLECTABLE, _playerCollisionListener, _level, _playerNode->getCollisionObject(), true);
        addOrRemoveCollisionListener(collision::Type::WATER, _playerCollisionListener, _level, _playerNode->getCollisionObject(), true);
        addOrRemoveCollisionListener(collision::Type::KINEMATIC, _playerCollisionListener, _level, _playerNode->getCollisionObject(), true);
    }

    void LevelCollisionComponent::onLevelUnloaded()
    {
        if(_playerNode)
        {
            _character->setGhostCollisionCallback(nullptr);
            addOrRemoveCollisionListener(collision::Type::LADDER, _playerCollisionListener, _level, _playerNode->getCollisionObject(), false);
            addOrRemoveCollisionListener(collision::Type::RESET, _playerCollisionListener, _level, _playerNode->getCollisionObject(), false);
            addOrRemoveCollisionListener(collision::Type::COLLECTABLE, _playerCollisionListener, _level, _playerNode->getCollisionObject(), false);
            addOrRemoveCollisionListener(collision::Type::WATER, _playerCollisionListener, _level, _playerNode->getCollisionObject(), false);
            addOrRemoveCollisionListener(collision::Type::KINEMATIC, _playerCollisionListener, _level, _playerNode->getCollisionObject(), false);
        }

        _character = nullptr;
        SAFE_RELEASE(_playerNode);
        SAFE_RELEASE(_player);
        SAFE_RELEASE(_level);
    }

    void LevelCollisionComponent::onPostSimulationUpdate()
    {
        if(_waitForPhysicsCleanup)
        {
            ++_framesSinceLevelReloaded;

            if(_framesSinceLevelReloaded > 1)
            {
                _framesSinceLevelReloaded = 0;
                _waitForPhysicsCleanup = false;
            }
        }

        if(_level)
        {
            for(LevelLoaderComponent::Collectable * collectable : _collectables)
            {
                if(!collectable->_active && collectable->_node->getCollisionObject()->isEnabled())
                {
                    collectable->_node->getCollisionObject()->setEnabled(false);
                }
            }
        }
    }

    void LevelCollisionComponent::initialize()
    {
        _playerResetMessage = PlayerResetMessage::create();
        _enemyKilledMessage = EnemyKilledMessage::create();
        _playerCollisionListener = new PlayerCollisionListener();
        _playerCollisionListener->_collisionHandler = this;
    }

    void LevelCollisionComponent::finalize()
    {
        onLevelUnloaded();
        gameobjects::Message::destroy(&_playerResetMessage);
        gameobjects::Message::destroy(&_enemyKilledMessage);
        SAFE_RELEASE(_playerCollisionListener);
    }

    void LevelCollisionComponent::PlayerCollisionListener::collisionEvent(gameplay::PhysicsCollisionObject::CollisionListener::EventType type,
                        gameplay::PhysicsCollisionObject::CollisionPair const & collisionPair,
                        gameplay::Vector3 const &, gameplay::Vector3 const &)
    {
        if(!_collisionHandler->_waitForPhysicsCleanup)
        {
            _collisionHandler->onTerrainCollision(type, collisionPair.objectA->getNode(), collisionPair.objectB->getNode());
        }
    }

    void LevelCollisionComponent::onCharacterCollision(gameplay::Node * enemyNode, gameplay::Vector3 collisionNormal)
    {
        EnemyComponent * enemy = gameobjects::GameObject::getGameObject(enemyNode->getParent())->getComponent<EnemyComponent>();
        if(enemy->getState() != EnemyComponent::State::Dead)
        {
            const bool killedEnemy = collisionNormal.y > 0.5f;
            if(killedEnemy)
            {
                enemy->kill();
                getRootParent()->broadcastMessage(_enemyKilledMessage);
                _player->jump(PlayerComponent::JumpSource::EnemyCollision);
            }
            else
            {
                getRootParent()->broadcastMessage(_playerResetMessage);
            }
        }
    }

    void LevelCollisionComponent::onTerrainCollision(gameplay::PhysicsCollisionObject::CollisionListener::EventType type,
                                                      gameplay::Node * playerNode, gameplay::Node * terrainNode)
    {
        if(collision::NodeData * NodeCollisionInfo = collision::NodeData::get(terrainNode))
        {
            bool const isColliding = type == gameplay::PhysicsCollisionObject::CollisionListener::EventType::COLLIDING;

            switch (NodeCollisionInfo->_type)
            {
                case collision::Type::LADDER:
                {
                    if (isColliding)
                    {
                        _player->setLadderPosition(terrainNode->getTranslation());
                        ++_playerClimbingTerrainRefCount;
                    }
                    else
                    {
                        --_playerClimbingTerrainRefCount;
                    }

                    GAME_ASSERT(_playerClimbingTerrainRefCount == MATH_CLAMP(_playerClimbingTerrainRefCount, 0, std::numeric_limits<int>::max()),
                        "_playerClimbingTerrainRefCount invalid %d", _playerClimbingTerrainRefCount);
                    _player->setClimbingEnabled(_playerClimbingTerrainRefCount > 0);
                    break;
                }
                case collision::Type::RESET:
                {
                    if (isColliding)
                    {
                        getRootParent()->broadcastMessage(_playerResetMessage);
                    }
                    break;
                }
                case collision::Type::COLLECTABLE:
                {
                    auto * collectable = _level->findCollectable(terrainNode);
                    if(collectable)
                    {
                        collectable->_active = false;
                    }
                    break;
                }
                case collision::Type::WATER:
                {
                    isColliding ? ++_playerSwimmingRefCount : --_playerSwimmingRefCount;
                    _player->setSwimmingEnabled(_playerSwimmingRefCount > 0);
                    GAME_ASSERT(_playerSwimmingRefCount == MATH_CLAMP(_playerSwimmingRefCount, 0, std::numeric_limits<int>::max()),
                        "_playerSwimmingRefCount invalid %d", _playerSwimmingRefCount);
                    break;
                }
                case collision::Type::KINEMATIC:
                {
                    _player->setIntersectingKinematic(isColliding ? terrainNode : nullptr);
                    break;
                }
                default:
                    GAME_ASSERTFAIL("Unhandled terrain collision type %d", NodeCollisionInfo->_type);
            }
        }
    }
}
