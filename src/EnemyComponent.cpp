#include "EnemyComponent.h"

#include "Common.h"
#include "PhysicsLoaderComponent.h"
#include "Game.h"
#include "GameObject.h"
#include "Messages.h"
#include "PropertiesRef.h"
#include "ResourceManager.h"
#include "SpriteAnimationComponent.h"

namespace game
{
    EnemyComponent::EnemyComponent()
        : _flipFlags(MATH_RANDOM_0_1() > 0.5f ? Sprite::Flip::Horizontal : Sprite::Flip::None)
        , _movementSpeed(5.0f)
        , _node(nullptr)
        , _state(State::Walking)
        , _minX(std::numeric_limits<float>::min())
        , _maxX(std::numeric_limits<float>::max())
        , _alpha(1.0f)
        , _snapToCollisionY(true)
        , _respawnTimeRangeSeconds(2.0f, 3.0f)
        , _respawnTimeSeconds(0.0f)
        , _respawnElapsed(0.0f)
        , _velocityX(0.0f)
    {
    }

    EnemyComponent::~EnemyComponent()
    {
    }

    void EnemyComponent::onStart()
    {
        _animations[State::Walking] = getParent()->findComponent<SpriteAnimationComponent>(_walkAnimComponentId);
        _animations[State::Dead] = getParent()->findComponent<SpriteAnimationComponent>(_deathAnimComponentId);

        PhysicsLoaderComponent * physics = getParent()->findComponent<PhysicsLoaderComponent>(_triggerComponentId);
        gameplay::PropertiesRef * propertiesRef = ResourceManager::getInstance().getProperties(physics->getPhysicsPath());
        gameplay::Properties * physicsProperties = propertiesRef->get();
        SpriteAnimationComponent * animation = _animations[State::Walking];
        float width = animation->getCurrentSpriteSrc().width * animation->getScale() * GAME_UNIT_SCALAR;
        float height = animation->getCurrentSpriteSrc().height * animation->getScale() * GAME_UNIT_SCALAR;
        physicsProperties->setString("extents", (toString(width) + ", " + toString(height) + ", 1").c_str());
        _respawnTimeSeconds = getRandomRange(_respawnTimeRangeSeconds.x, _respawnTimeRangeSeconds.y);
        SAFE_RELEASE(propertiesRef);
        physics->createPhysics();
        _node = physics->getNode();
        _node->addRef();

        getCurrentAnimation()->play();
    }

    void EnemyComponent::finalize()
    {
        SAFE_RELEASE(_node);
    }

    bool EnemyComponent::onMessageReceived(gameobjects::Message * message, int messageType)
    {
        switch(messageType)
        {
            case Messages::Type::SimulationUpdate:
            {
                SimulationUpdateMessage msg(message);
                onSimulationUpdate(msg._elapsedTime);
                break;
            }
            case Messages::Type::PostSimulationUpdate:
            {
                PostSimulationUpdateMessage msg(message);
                onPostSimulationUpdate(msg._elapsedTime);
                break;
            }
            default:
                break;
        }

        return true;
    }

    void EnemyComponent::readProperties(gameplay::Properties & properties)
    {
        _walkAnimComponentId = properties.getString("walk_anim");
        _deathAnimComponentId = properties.getString("death_anim");
        _triggerComponentId = properties.getString("collision_trigger");
        _movementSpeed = properties.getFloat("speed");
        gameobjects::setIfExists(properties, "snap_to_collision_y", _snapToCollisionY);
        gameobjects::setIfExists(properties, "respawn_range_seconds", _respawnTimeRangeSeconds);
    }

    EnemyComponent::State::Enum EnemyComponent::getState() const
    {
        return _state;
    }

    gameplay::Vector3 EnemyComponent::getRenderPosition() const
    {
        return gameplay::Game::getInstance()->getPhysicsController()->interpolate(_previousPosition, _node->getTranslation());
    }

    gameplay::Vector3 EnemyComponent::getVelocity() const
    {
        return gameplay::Vector3(0.0f, _velocityX, 0.0f);
    }

    void EnemyComponent::forEachAnimation(std::function <bool(State::Enum, SpriteAnimationComponent *)> func)
    {
        for (auto & pair : _animations)
        {
            if (func(pair.first, pair.second))
            {
                break;
            }
        }
    }

    void EnemyComponent::onSimulationUpdate(float elapsedTime)
    {
        _previousPosition = _node->getTranslation();

        if(_state != State::Dead)
        {
            // Calculate the next step
            float const dt = elapsedTime / 1000.0f;
            _velocityX = _movementSpeed *((_flipFlags & Sprite::Flip::Horizontal) ? -1.0f : 1.0f);
            float translationX = _velocityX * dt;
            float const originalTranslationX = _node->getTranslationX();
            float const nextTranslationX = originalTranslationX + translationX;
            float const offset = _node->getScaleX();
            float const clampedTranslationX = MATH_CLAMP(nextTranslationX, _minX + offset, _maxX - offset);

            // If it exceeds the constraints along x then use the clamped translation and turn around
            if(nextTranslationX != clampedTranslationX)
            {
                _flipFlags = _flipFlags & Sprite::Flip::Horizontal ? Sprite::Flip::None : Sprite::Flip::Horizontal;
                _node->setTranslationX(clampedTranslationX);
            }
            else
            {
                _node->translateX(translationX);
            }
        }
    }

    void EnemyComponent::onPostSimulationUpdate(float elapsedTime)
    {
        float const dt = elapsedTime / 1000.0f;
        bool const isAlive = _state != State::Dead;
        bool const isEnabled = _node->getCollisionObject()->isEnabled();
        _respawnElapsed = isAlive && isEnabled ? 0.0f : _respawnElapsed;
        _respawnElapsed = MATH_CLAMP(_respawnElapsed + dt, 0.0f, _respawnTimeRangeSeconds.y);
        bool const isRespawnRequired = _respawnElapsed == _respawnTimeRangeSeconds.y;
        _state = isRespawnRequired ? State::Walking : _state;
        getCurrentAnimation()->update(elapsedTime);
        float const fadeSpeed = 0.5f;
        float fadeDirection = isAlive ? 1.0f : -1.0f;
        _alpha = MATH_CLAMP(_alpha + ((dt * fadeSpeed) * fadeDirection), 0, 1.0f);
        bool const isVisibleTarget = _alpha > 0.35f;
        _node->getCollisionObject()->setEnabled(isVisibleTarget && (isAlive || isRespawnRequired));
    }

    SpriteAnimationComponent * EnemyComponent::getCurrentAnimation()
    {
        return _animations[_state];
    }

    gameplay::Node * EnemyComponent::getNode() const
    {
        return _node;
    }

    Sprite::Flip::Enum EnemyComponent::getFlipFlags() const
    {
        return static_cast<Sprite::Flip::Enum>(_flipFlags);
    }

    float EnemyComponent::getAlpha() const
    {
        return _alpha;
    }

    void EnemyComponent::setHorizontalConstraints(float minX, float maxX)
    {
        _minX = minX;
        _maxX = maxX;
    }

    void EnemyComponent::kill()
    {
        _state = State::Dead;
        getCurrentAnimation()->play();
    }

    bool EnemyComponent::isSnappedToCollisionY() const
    {
        return _snapToCollisionY;
    }
}
