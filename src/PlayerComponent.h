#ifndef GAME_PLAYER_COMPONENT_H
#define GAME_PLAYER_COMPONENT_H

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
     * Creates a players physics and animation defined in sibling components and updates
     * its local state upon receiving user input.
     *
     * @script{ignore}
    */
    class PlayerComponent : public gameobjects::Component
    {
    public:
        /** @script{ignore} */
        struct State
        {
            enum Enum
            {
                Idle,
                Walking,
                Cowering,
                Jumping,
                Climbing,
                Swimming
            };
        };

        /** @script{ignore} */
        struct MovementDirection
        {
            enum Enum
            {
                None        = 0,
                Left        = 1 << 0,
                Right       = 1 << 1,
                Down        = 1 << 2,
                Up          = 1 << 3,

                EnumCount,

                Horizontal = Left | Right,
                Vertical = Up | Down
            };
        };

        /** @script{ignore} */
        struct JumpSource
        {
            enum Enum
            {
                Input,
                EnemyCollision,
            };
        };

        explicit PlayerComponent();
        ~PlayerComponent();

        void forEachAnimation(std::function <bool(State::Enum, SpriteAnimationComponent *)> func) const;
        State::Enum getState() const;
        gameplay::Vector3 getPosition() const;
        gameplay::Vector3 getRenderPosition() const;
        SpriteAnimationComponent * getCurrentAnimation();
        gameplay::Node * getNode() const;
        gameplay::PhysicsCharacter * getCharacter() const;
        Sprite::Flip::Enum getFlipFlags() const;

        void setIntersectingKinematic(gameplay::Node * node);
        void setSwimmingEnabled(bool enabled);
        void setClimbingEnabled(bool enabled);
        void setLadderPosition(gameplay::Vector3 const & pos);
        void jump(JumpSource::Enum source, float scale = 1.0f);
        void reset(gameplay::Vector3 const & position);
        void onPlayerDirectionalInput(MovementDirection::Enum direction, bool enabled, float scale = 1.0f);
        void onPlayerInputComplete();
    protected:
        virtual void onStart() override;
        virtual void finalize() override;
        virtual void readProperties(gameplay::Properties & properties) override;
        virtual bool onMessageReceived(gameobjects::Message * message, int messageType) override;
    private:
        PlayerComponent(PlayerComponent const &);

        void attachToKinematic();
        void detachFromKinematic();
        void onSimulationUpdate(float elapsedTime);
        void onPostSimulationUpdate(float elapsedTime);

        State::Enum _state;
        State::Enum _previousState;
        MovementDirection::Enum _horizontalMovementDirection;
        gameplay::Node * _node;
        gameplay::Node * _kinematicNode;
        gameplay::PhysicsCharacter * _character;
        std::map<State::Enum, SpriteAnimationComponent*> _animations;

        gameplay::Vector3 _ladderPosition;
        gameplay::Vector3 _previousClimbPosition;
        int _flipFlags;

        bool _climbingEnabled;
        bool _swimmingEnabled;

        float _horizontalMovementScale;
        float _verticalMovementScale;
        float _movementSpeed;
        float _swimSpeedScale;
        float _jumpHeight;

        std::string _idleAnimComponentId;
        std::string _walkAnimComponentId;
        std::string _cowerAnimComponentId;
        std::string _jumpAnimComponentId;
        std::string _physicsComponentId;
        std::string _climbingCharacterComponentId;
        std::string _swimmingCharacterComponentId;

        gameobjects::Message * _jumpMessage;
    };
}

#endif
