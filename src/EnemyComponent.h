#ifndef GAME_ENEMY_COMPONENT_H
#define GAME_ENEMY_COMPONENT_H

#include "Component.h"
#include "Sprite.h"

namespace gameplay
{
    class Properties;
}

namespace game
{
    class SpriteAnimationComponent;

    /**
     * A simple enemy behaviour that travels horizontally back and forth
     *
     * @script{ignore}
    */
    class EnemyComponent : public gameobjects::Component
    {
    public:
        /** @script{ignore} */
        struct State
        {
            enum Enum
            {
                Walking,
                Dead
             };
        };

        explicit EnemyComponent();
        ~EnemyComponent();

        void forEachAnimation(std::function <bool(State::Enum, SpriteAnimationComponent *)> func);
        void setHorizontalConstraints(float minX, float maxX);
        void kill();
        State::Enum getState() const;
        gameplay::Vector3 getRenderPosition() const;
        gameplay::Vector3 getVelocity() const;
        SpriteAnimationComponent * getCurrentAnimation();
        gameplay::Node * getNode() const;
        Sprite::Flip::Enum getFlipFlags() const;
        bool isSnappedToCollisionY() const;
        float getAlpha() const;
    protected:
        void onStart() override;
        void finalize() override;
        bool onMessageReceived(gameobjects::Message * message, int messageType) override;
        void onSimulationUpdate(float elapsedTime);
        void onPostSimulationUpdate(float elapsedTime);
        void readProperties(gameplay::Properties & properties) override;
    private:
        EnemyComponent(EnemyComponent const &);

        gameplay::Node * _node;
        std::map<State::Enum, SpriteAnimationComponent*> _animations;
        int _flipFlags;
        State::Enum _state;
        float _movementSpeed;
        float _minX;
        float _maxX;
        float _alpha;
        float _respawnTimeSeconds;
        float _respawnElapsed;
        float _velocityX;
        bool _snapToCollisionY;
        std::string _walkAnimComponentId;
        std::string _deathAnimComponentId;
        std::string _triggerComponentId;
        std::string _respawnTimeRangeId;
        gameplay::Vector3 _previousPosition;
        gameplay::Vector2 _respawnTimeRangeSeconds;
    };
}

#endif
