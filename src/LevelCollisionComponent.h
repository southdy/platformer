#ifndef GAME_COLLISION_HANDLER_COMPONENT_H
#define GAME_COLLISION_HANDLER_COMPONENT_H

#include "Component.h"
#include "LevelLoaderComponent.h"
#include "PhysicsCollisionObject.h"

namespace gameplay
{
    class AIMessage;
}

namespace game
{
    class LevelLoaderComponent;
    class PlayerComponent;

    /**
     * Handles collision events between characters and the terrain
     *
     * @script{ignore}
    */
    class LevelCollisionComponent : public gameobjects::Component
    {
        friend class EnemyCollisionListener;
        friend class PlayerCollisionListener;

    public:
        explicit LevelCollisionComponent();
        ~LevelCollisionComponent();
    protected:
        virtual bool onMessageReceived(gameobjects::Message * message, int messageType) override;
        virtual void initialize() override;
        virtual void finalize() override;
    private:
        LevelCollisionComponent(LevelCollisionComponent const &);

        void onLevelLoaded();
        void onLevelUnloaded();
        void onPostSimulationUpdate();
        void onCharacterCollision(gameplay::Node * enemyNode, gameplay::Vector3 firstNormal);
        void onTerrainCollision(gameplay::PhysicsCollisionObject::CollisionListener::EventType type,
                            gameplay::Node * playerNode, gameplay::Node * terrainNode);

        struct PlayerCollisionListener : public gameplay::PhysicsCollisionObject::CollisionListener, gameplay::Ref
        {
            virtual void collisionEvent(gameplay::PhysicsCollisionObject::CollisionListener::EventType type,
                                gameplay::PhysicsCollisionObject::CollisionPair const & collisionPair,
                                gameplay::Vector3 const &, gameplay::Vector3 const &) override;
            LevelCollisionComponent * _collisionHandler;
        };

        PlayerCollisionListener * _playerCollisionListener;
        PlayerComponent * _player;
        LevelLoaderComponent * _level;
        gameplay::Node * _playerNode;
        gameplay::PhysicsCharacter * _character;
        gameobjects::Message * _playerResetMessage;
        gameobjects::Message * _enemyKilledMessage;
        std::vector<LevelLoaderComponent::Collectable *> _collectables;
        int _playerClimbingTerrainRefCount;
        int _playerSwimmingRefCount;
        int _framesSinceLevelReloaded;
        bool _waitForPhysicsCleanup;
    };
}

#endif
