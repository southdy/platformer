#include "PlayerResetComponent.h"

#include "CameraComponent.h"
#include "Common.h"
#include "GameObject.h"
#include "LevelLoaderComponent.h"
#include "Messages.h"
#include "PlayerComponent.h"
#include "ScreenOverlay.h"

namespace game
{
    PlayerResetComponent::PlayerResetComponent()
        : _levelLoaded(false)
        , _forceNextReset(false)
    {
    }

    PlayerResetComponent::~PlayerResetComponent()
    {
    }

    void PlayerResetComponent::onStart()
    {
        _player = getParent()->getComponent<PlayerComponent>();
        _player->addRef();
        _camera = getRootParent()->getComponent<CameraComponent>();
        _camera->addRef();
    }

    void PlayerResetComponent::finalize()
    {
        SAFE_RELEASE(_player);
        SAFE_RELEASE(_camera);
    }

    void PlayerResetComponent::onPostSimulationUpdate()
    {
        if(_levelLoaded && (_forceNextReset || !_boundary.intersects(_player->getPosition().x, _player->getPosition().y, 1, 1)))
        {
            _forceNextReset = false;

            if (_player)
            {
                float const fadeOutDuration = 1.15f;
                ScreenOverlay::getInstance().queueFadeToBlack(0.0f);
                ScreenOverlay::getInstance().queueFadeOut(fadeOutDuration);
                _player->reset(_resetPosition);
            }
        }
    }

    bool PlayerResetComponent::onMessageReceived(gameobjects::Message * message, int messageType)
    {
        switch (messageType)
        {
        case(Messages::Type::LevelLoaded):
        {
            LevelLoaderComponent * level = getRootParent()->getComponentInChildren<LevelLoaderComponent>();
            _resetPosition = level->getPlayerSpawnPosition();
            float const height = ((level->getHeight() + 1) * level->getTileHeight()) * GAME_UNIT_SCALAR;
            _boundary.width = _camera->getTargetBoundary().width;
            _boundary.height = height;
            _boundary.x = _camera->getTargetBoundary().x;
            _boundary.y = -height + (height - _boundary.height) / 2;
            _boundary.height = std::numeric_limits<float>::max();
            _levelLoaded = true;
        }
            break;
        case(Messages::Type::LevelUnloaded):
            _levelLoaded = false;
            break;
        case(Messages::Type::PlayerReset):
            _forceNextReset = true;
            break;
        case(Messages::Type::PostSimulationUpdate):
            onPostSimulationUpdate();
            break;
        }

        return true;
    }
}
