#include "PlayerComponent.h"

#include "Common.h"
#include "PhysicsLoaderComponent.h"
#include "Game.h"
#include "GameObject.h"
#include "LevelCollision.h"
#include "Messages.h"
#include "PhysicsCharacter.h"
#include "Properties.h"
#include "PropertiesRef.h"
#include "ResourceManager.h"
#include "SpriteAnimationComponent.h"

namespace game
{
    PlayerComponent::PlayerComponent()
        : _flipFlags(Sprite::Flip::None)
        , _horizontalMovementDirection(MovementDirection::None)
        , _previousState(State::Idle)
        , _movementSpeed(5.0f)
        , _jumpHeight(1.0f)
        , _node(nullptr)
        , _horizontalMovementScale(0.0f)
        , _verticalMovementScale(0.0f)
        , _jumpMessage(nullptr)
        , _climbingEnabled(false)
        , _swimmingEnabled(false)
        , _swimSpeedScale(1.0f)
        , _kinematicNode(nullptr)
        , _character(nullptr)
    {
    }

    PlayerComponent::~PlayerComponent()
    {
    }

    void PlayerComponent::onStart()
    {
        _animations[State::Idle] = getParent()->findComponent<SpriteAnimationComponent>(_idleAnimComponentId);
        _animations[State::Walking] = getParent()->findComponent<SpriteAnimationComponent>(_walkAnimComponentId);
        _animations[State::Cowering] = getParent()->findComponent<SpriteAnimationComponent>(_cowerAnimComponentId);
        _animations[State::Jumping] = getParent()->findComponent<SpriteAnimationComponent>(_jumpAnimComponentId);
        _animations[State::Climbing] = getParent()->findComponent<SpriteAnimationComponent>(_climbingCharacterComponentId);
        _animations[State::Swimming] = getParent()->findComponent<SpriteAnimationComponent>(_swimmingCharacterComponentId);

        std::vector<PhysicsLoaderComponent*> physicsComponents;
        getParent()->getComponents<PhysicsLoaderComponent>(physicsComponents);
        for (PhysicsLoaderComponent * physics : physicsComponents)
        {
            gameplay::PropertiesRef * propertiesRef = ResourceManager::getInstance().getProperties(physics->getPhysicsPath());
            gameplay::Properties * physicsProperties = propertiesRef->get();
            SpriteAnimationComponent * animation = _animations[State::Idle];
            float const radius = ((animation->getCurrentSpriteSrc().width * animation->getScale()) / 2) * GAME_UNIT_SCALAR;
            float const height = animation->getCurrentSpriteSrc().height * animation->getScale() * GAME_UNIT_SCALAR;
            physicsProperties->setString("radius", toString(radius).c_str());
            physicsProperties->setString("height", toString(height).c_str());
            SAFE_RELEASE(propertiesRef);
            physics->createPhysics();
        }

        _node = getParent()->findComponent<PhysicsLoaderComponent>(_physicsComponentId)->getNode();
        _node->addRef();
        _character = static_cast<gameplay::PhysicsCharacter*>(_node->getCollisionObject());
        _character->setLinearFactor(gameplay::Vector3(1, 1, 0));
        _character->setClampFallToGravity(true);
        _character->setHorizontalImpulsesEnabled(false);

        _jumpMessage = PlayerJumpMessage::create();
        _state = State::Idle;
    }

    void PlayerComponent::finalize()
    {
        getParent()->getNode()->removeAllChildren();
        detachFromKinematic();
        SAFE_RELEASE(_node);
        gameobjects::Message::destroy(&_jumpMessage);
    }

    void PlayerComponent::readProperties(gameplay::Properties & properties)
    {
        _idleAnimComponentId = properties.getString("idle_anim");
        _walkAnimComponentId = properties.getString("walk_anim");
        _cowerAnimComponentId = properties.getString("cower_anim");
        _jumpAnimComponentId = properties.getString("jump_anim");
        _climbingCharacterComponentId = properties.getString("climb_anim");
        _swimmingCharacterComponentId = properties.getString("swim_anim");
        _movementSpeed = properties.getFloat("speed");
        _swimSpeedScale = properties.getFloat("swim_speed_scale");
        _jumpHeight = properties.getFloat("jump_height");
        _physicsComponentId = properties.getString("physics");
    }

    PlayerComponent::State::Enum PlayerComponent::getState() const
    {
        return _state;
    }

    gameplay::Vector3 PlayerComponent::getPosition() const
    {
        return _node->getTranslation() + _node->getParent()->getTranslation();
    }

    gameplay::Vector3 PlayerComponent::getRenderPosition() const
    {
        return _character->getRenderPosition();
    }

    void PlayerComponent::forEachAnimation(std::function <bool(State::Enum, SpriteAnimationComponent *)> func) const
    {
        for (auto & pair : _animations)
        {
            if (func(pair.first, pair.second))
            {
                break;
            }
        }
    }

    bool PlayerComponent::onMessageReceived(gameobjects::Message * message, int messageType)
    {
        switch (messageType)
        {
        case Messages::Type::PostSimulationUpdate:
            onPostSimulationUpdate(PostSimulationUpdateMessage(message)._elapsedTime);
            break;
        case Messages::Type::SimulationUpdate:
            onSimulationUpdate(SimulationUpdateMessage(message)._elapsedTime);
            break;
        default:
            break;
        }

        return true;
    }

    void PlayerComponent::onSimulationUpdate(float elapsedTime)
    {
        float const minVerticalScaleToInitiateClimb = 0.35f;
        float const minDistToLadderCentre = _node->getScaleX() * 0.15f;
        gameplay::Vector3 const ladderVeritcallyAlignedPosition(_ladderPosition.x, getRenderPosition().y, 0.0f);
        bool const isClimbRequested = fabs(_verticalMovementScale) > minVerticalScaleToInitiateClimb;
        bool const isPlayerWithinLadderClimbingDistance = getRenderPosition().distance(ladderVeritcallyAlignedPosition) <= minDistToLadderCentre;

        // Initiate climbing if possible
        if(_climbingEnabled && isClimbRequested && isPlayerWithinLadderClimbingDistance)
        {
            if (_state != State::Climbing)
            {
                // Attach the player to the ladder at the render position rather than the physics position
                _node->setTranslation(gameplay::Vector3(ladderVeritcallyAlignedPosition.x, ladderVeritcallyAlignedPosition.y, 0));
            }

            _state = State::Climbing;

            // Physics will be disabled so that we can translate the player vertically along the ladder
            _node->setTranslationX(_ladderPosition.x);
            _character->resetVelocityState();
            _character->setPhysicsEnabled(false);
        }

        if(_state == State::Climbing)
        {
            // Move the player along the ladder using the input vertical movement scale
            float const elapsedTimeMs = elapsedTime / 1000.0f;
            float const verticalMovementSpeed = _movementSpeed / 2.0f;
            float const previousDistToLadder = _ladderPosition.distanceSquared(_node->getTranslation());
            gameplay::Vector3 const previousPosition = _node->getTranslation();
            _node->translateY((_verticalMovementScale * verticalMovementSpeed) * elapsedTimeMs);

            // If the player has moved away from the ladder but they are no longer intersecting it then restore their last position
            // and zero their movement, this will prevent them from climing beyond the top/bottom
            if(!_climbingEnabled && previousDistToLadder < _node->getTranslation().distanceSquared(_ladderPosition))
            {
                _node->setTranslation(previousPosition);
                _verticalMovementScale = 0.0f;
            }
        }
    }

    void PlayerComponent::onPostSimulationUpdate(float elapsedTime)
    {
        // Play animation if different
        if(_state != _previousState)
        {
            _animations[_previousState]->stop();
            getCurrentAnimation()->play();
        }

        // Scale the animation speed using the movement scale
        if(_state == State::Walking || _state == State::Climbing)
        {
            getCurrentAnimation()->setSpeed(fabs(_state == State::Climbing ? _verticalMovementScale : _horizontalMovementScale));
        }

        _previousState = _state;

        getCurrentAnimation()->update(elapsedTime);
    }

    void PlayerComponent::onPlayerInputComplete()
    {
        gameplay::Vector3 velocity = _character->getCurrentVelocity();

        if(_character->isPhysicsEnabled())
        {
            // Zero velocity once the player has stopped falling and there isn't a desired movement direction
            if(_horizontalMovementDirection == MovementDirection::None)
            {
                if (velocity.y == 0.0f && velocity.x != 0.0f)
                {
                    velocity = gameplay::Vector3::zero();
                }
            }

            if (_state != State::Swimming)
            {
                if (velocity.isZero())
                {
                    // Player should swim on the spot when idle while in water
                    if(_horizontalMovementDirection == MovementDirection::None)
                    {
                        _state = _swimmingEnabled ? State::Swimming : State::Idle;
                    }
                }
                else
                {
                    // Handle when the player moves horizontally
                    if (velocity.y == 0.0f && velocity.x != 0.0f)
                    {
                        float moveScale = _horizontalMovementScale;

                        if (_swimmingEnabled && _state == State::Walking)
                        {
                            // Player has moved into a water volume, transition from walking to swimming and move at swim speed
                            _state = State::Swimming;
                            moveScale *= _swimSpeedScale;
                        }
                        else
                        {
                            // Player has transitioned to walking from swimming
                            _state = State::Walking;
                        }

                        velocity.x = ((_flipFlags & Sprite::Flip::Horizontal) ? -_movementSpeed : _movementSpeed) * moveScale;
                    }

                    // Make the player cower when they are pushed/walk off a ledge
                    float const minFallVelocity = gameplay::Game::getInstance()->getPhysicsController()->getGravity().y * 0.25f;

                    if (velocity.y < minFallVelocity && (_state == State::Idle || _state == State::Walking))
                    {
                        _state = State::Cowering;
                    }
                }
            }

            // Apply new velocity if different
            if(velocity != _character->getCurrentVelocity())
            {
                velocity.z = 0.0f;
                _character->setVelocity(velocity);
            }
        }
    }

    SpriteAnimationComponent * PlayerComponent::getCurrentAnimation()
    {
        return _animations[_state];
    }

    gameplay::Node * PlayerComponent::getNode() const
    {
        return _node;
    }

    gameplay::PhysicsCharacter * PlayerComponent::getCharacter() const
    {
        return _character;
    }

    void PlayerComponent::onPlayerDirectionalInput(MovementDirection::Enum direction, bool enabled, float scale /* = 1.0f */)
    {
        if(enabled)
        {
            if(direction & MovementDirection::Horizontal)
            {
                // Apply state based on horizontal movement immediatley

                _horizontalMovementDirection = direction;

                float const minHorizontalScaleToCancelClimb = 0.75f;

                if(_state == State::Climbing && scale > minHorizontalScaleToCancelClimb)
                {
                    // Player was climbing a ladder but they moved enough horizontally that climbing should be cancelled
                    _verticalMovementScale = 0.0f;
                    _character->setPhysicsEnabled(true);
                    _state = State::Cowering;
                }
                else if (_swimmingEnabled && _character->getCurrentVelocity().y == 0)
                {
                    // Player is colliding with a swimming surface
                    _state = State::Swimming;
                }

                // Apply the horizontal velocity and face the player in the direction of the input provided they aren't climbing
                if(_state != State::Climbing)
                {
                    _flipFlags = direction == MovementDirection::Left ? Sprite::Flip::Horizontal : Sprite::Flip::None;
                    float horizontalSpeed = (_flipFlags & Sprite::Flip::Horizontal) ? -_movementSpeed : _movementSpeed;
                    horizontalSpeed *= _state == State::Swimming ? scale * _swimSpeedScale : scale;
                    _character->setVelocity(horizontalSpeed, 0.0f, 0.0f);
                    _horizontalMovementScale = scale;
                }
            }
            else
            {
                // Cache the desired vertical movement, state will only be applied in update since we want the player
                // to be able to apply both horizontal and vertical movement at the same time
                _verticalMovementScale = scale * (direction == MovementDirection::Up ? 1.0f : -1.0f);
            }
        }
        else
        {
            // Zero mutually exclusive pairs left/right and up/down if input to disable matches current movement scale
            if(direction & MovementDirection::Vertical)
            {
                if((direction & MovementDirection::Up && _verticalMovementScale > 0) || (direction & MovementDirection::Down && _verticalMovementScale < 0))
                {
                    _verticalMovementScale = 0.0f;
                }
            }
            else if(_horizontalMovementDirection == direction)
            {
                _horizontalMovementDirection = MovementDirection::None;
                _horizontalMovementScale = 0.0f;
            }
        }
    }

    void PlayerComponent::attachToKinematic()
    {
        gameplay::Node * kinematicParent = _kinematicNode->getParent();
        _node->setTranslation(_node->getTranslation() - kinematicParent->getTranslation());
        kinematicParent->addChild(_node);
        _state = State::Idle;
    }

    void PlayerComponent::detachFromKinematic()
    {
        if(_kinematicNode)
        {
            gameplay::Node * kinematicParent = _kinematicNode->getParent();
            _node->setTranslation(_node->getTranslation() + kinematicParent->getTranslation());
            kinematicParent->removeChild(_node);
            getParent()->getNode()->addChild(_node);
            SAFE_RELEASE(kinematicParent);
            SAFE_RELEASE(_kinematicNode);
        }
    }

    void PlayerComponent::setIntersectingKinematic(gameplay::Node * node)
    {
        detachFromKinematic();

        if (node)
        {
            node->getParent()->addRef();
            node->addRef();
            _kinematicNode = node;
            attachToKinematic();
        }
    }

    void PlayerComponent::setClimbingEnabled(bool enabled)
    {
        if(_climbingEnabled && !enabled && _state == State::Climbing)
        {
            _verticalMovementScale = 0.0f;
        }

        _climbingEnabled = enabled;
    }

    void PlayerComponent::setSwimmingEnabled(bool enabled)
    {
        if(!enabled && _swimmingEnabled && _state == State::Swimming)
        {
            _state = State::Idle;
        }

        _swimmingEnabled = enabled;
    }

    void PlayerComponent::setLadderPosition(gameplay::Vector3 const & pos)
    {
        _ladderPosition = pos;
    }

    void PlayerComponent::jump(JumpSource::Enum source, float scale)
    {
        gameplay::Vector3 const characterOriginalVelocity = _character->getCurrentVelocity();
        gameplay::Vector3 preJumpVelocity;
        bool jumpAllowed = characterOriginalVelocity.y == 0;
        float jumpHeight = (_jumpHeight * scale);
        bool resetVelocityState = true;

        switch(source)
        {
            case JumpSource::Input:
            {
                preJumpVelocity.x = _state != State::Swimming || characterOriginalVelocity.x == 0 ? characterOriginalVelocity.x :
                    (_movementSpeed * _swimSpeedScale) * (!(_flipFlags & Sprite::Flip::Horizontal) ? 1.0f : -1.0f);
#ifndef _FINAL
                if(getConfig()->getBool("multi_jump"))
                {
                    if(characterOriginalVelocity.y < 0)
                    {
                        jumpHeight += -characterOriginalVelocity.y / 2.0f;
                    }

                    resetVelocityState = false;
                    jumpAllowed = true;
                }
#endif
                break;
            }
            case JumpSource::EnemyCollision:
            {
                preJumpVelocity.x = characterOriginalVelocity.x;
                float const verticalModifier = 1.5f;
                jumpHeight *= verticalModifier;
                jumpAllowed = true;
                break;
            }
            default:
                GAME_ASSERTFAIL("Unhandled JumpSource %d", source);
                break;
        }

        if(jumpAllowed)
        {
            if(_kinematicNode && _character->getCurrentVelocity().isZero())
            {
                gameplay::PhysicsRigidBody * kinematic = static_cast<gameplay::PhysicsRigidBody *>(_kinematicNode->getCollisionObject());
                preJumpVelocity.y = kinematic->getLinearVelocity().y;
            }

            _state = State::Jumping;
            _character->setPhysicsEnabled(true);

            if(resetVelocityState)
            {
                _character->resetVelocityState();
            }

            _character->setVelocity(preJumpVelocity);
            _character->jump(jumpHeight, !resetVelocityState);
            if(source != JumpSource::EnemyCollision)
            {
                getRootParent()->broadcastMessage(_jumpMessage);
            }
            _verticalMovementScale = 0.0f;
        }
    }

    Sprite::Flip::Enum PlayerComponent::getFlipFlags() const
    {
        return static_cast<Sprite::Flip::Enum>(_flipFlags);
    }

    void PlayerComponent::reset(gameplay::Vector3 const & position)
    {
        _state = State::Idle;
        _flipFlags = Sprite::Flip::None;
        _node->setTranslation(position);
        _character->resetVelocityState();
        _character->setPhysicsEnabled(true);
    }
}
