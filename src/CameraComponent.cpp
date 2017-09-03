#include "CameraComponent.h"

#include "Common.h"
#include "GameObject.h"
#include "GameObjectController.h"
#include "Messages.h"
#include "PlayerComponent.h"
#include "Scene.h"

namespace game
{
    CameraComponent::CameraComponent()
        : _camera(nullptr)
        , _player(nullptr)
        , _previousZoom(0.0f)
        , _zoomSpeedScale(0.003f)
        , _smoothSpeedScale(0.1f)
        , _targetBoundaryScale(0.25f, 0.5)
        , _boundary(std::numeric_limits<float>::max(), std::numeric_limits<float>::max())
    {
        _currentZoom = getMaxZoom();
        _targetZoom = getDefaultZoom();
    }

    CameraComponent::~CameraComponent()
    {
    }

    void CameraComponent::onStart()
    {
        _camera = gameobjects::GameObjectController::getInstance().getScene()->getActiveCamera();
        _camera->addRef();
        _initialCurrentZoom = _currentZoom;
        _initialTargetZoom = _targetZoom;
    }

    void CameraComponent::readProperties(gameplay::Properties & properties)
    {
        gameobjects::setIfExists(properties, "target_boundary_screen_scale", _targetBoundaryScale);
        gameobjects::setIfExists(properties, "zoom_speed_scale", _zoomSpeedScale);
        gameobjects::setIfExists(properties, "smooth_speed_scale", _smoothSpeedScale);
    }

    void CameraComponent::finalize()
    {
        SAFE_RELEASE(_camera);
        SAFE_RELEASE(_player);
    }

    bool CameraComponent::onMessageReceived(gameobjects::Message *message, int messageType)
    {
        switch(messageType)
        {
        case Messages::Type::PostSimulationUpdate:
            if(_player)
            {
                onPostSimulationUpdate(_player->getRenderPosition(), PostSimulationUpdateMessage(message)._elapsedTime);
            }
            break;
        case Messages::Type::LevelLoaded:
            _player = getParent()->getComponentInChildren<PlayerComponent>();
            _player->addRef();
            break;
        case Messages::Type::LevelUnloaded:
            SAFE_RELEASE(_player);
            break;
        }
        return true;
    }

    void CameraComponent::onPostSimulationUpdate(gameplay::Vector3 const & target, float elapsedTime)
    {
        bool clamp = true;
#ifndef _FINAL
        clamp = getConfig()->getBool("clamp_camera");
#endif

        if(_currentZoom != _targetZoom)
        {
            _currentZoom = gameplay::Curve::lerp(elapsedTime * _zoomSpeedScale, _currentZoom, _targetZoom);
            if(clamp)
            {
                _currentZoom = MATH_CLAMP(_currentZoom, getMinZoom(), getMaxZoom());
            }

            if(_currentZoom != _previousZoom)
            {
                _camera->setZoomX(gameplay::Game::getInstance()->getWidth() * _currentZoom);
                _camera->setZoomY(gameplay::Game::getInstance()->getHeight() * _currentZoom);
            }

            _previousZoom = _currentZoom;
        }

        _targetPosition.smooth(target, elapsedTime / 1000.0f, _smoothSpeedScale);
        gameplay::Game::getInstance()->getAudioListener()->setPosition(_targetPosition.x, _targetPosition.y, 0.0);
        if(clamp)
        {
            float const offsetX = (gameplay::Game::getInstance()->getWidth() / 2) *  _currentZoom;
            _targetPosition.x = MATH_CLAMP(_targetPosition.x, _boundary.x + offsetX, _boundary.x + _boundary.width - offsetX);
            float const offsetY = (gameplay::Game::getInstance()->getHeight() / 2) *  _currentZoom;
            _targetPosition.y = MATH_CLAMP(_targetPosition.y, _boundary.y + offsetY, std::numeric_limits<float>::max());
        }
        _camera->getNode()->setTranslation(gameplay::Vector3(_targetPosition.x, _targetPosition.y, 0));
    }

    void CameraComponent::setBoundary(gameplay::Rectangle boundary)
    {
        _boundary = boundary;
    }

    void CameraComponent::setPosition(gameplay::Vector3 const & position)
    {
        _camera->getNode()->setTranslation(position);
    }

    float CameraComponent::getMinZoom()
    {
        return getMaxZoom() / 4;
    }

    float CameraComponent::getMaxZoom()
    {
        float const viewportWidth = 1280.0f;
        float const screenWidth =  gameplay::Game::getInstance()->getWidth();
        return GAME_UNIT_SCALAR * ((1.0f / screenWidth) * viewportWidth);
    }

    float CameraComponent::getDefaultZoom()
    {
        return (getMinZoom() + getMaxZoom()) / 2;
    }

    float CameraComponent::getZoom() const
    {
        return _currentZoom;
    }

    float CameraComponent::getTargetZoom() const
    {
        return _targetZoom;
    }

    gameplay::Matrix CameraComponent::getRenderViewProjectionMatrix()
    {
        gameplay::Camera * camera = gameobjects::GameObjectController::getInstance().getScene()->getActiveCamera();
        gameplay::Matrix viewProj = camera->getViewProjectionMatrix();
        viewProj.rotateX(MATH_DEG_TO_RAD(180));
        viewProj.scale(GAME_UNIT_SCALAR);
        return viewProj;
    }

    gameplay::Matrix const & CameraComponent::getViewProjectionMatrix()
    {
        return gameobjects::GameObjectController::getInstance().getScene()->getActiveCamera()->getViewProjectionMatrix();
    }

    gameplay::Rectangle CameraComponent::getRenderViewport()
    {
        gameplay::Camera * camera = gameobjects::GameObjectController::getInstance().getScene()->getActiveCamera();
        gameplay::Rectangle viewport = gameplay::Game::getInstance()->getViewport();
        float currentZoomX = camera->getZoomX();
#ifndef _FINAL
        if(getConfig()->getBool("show_culling"))
        {
            currentZoomX = getDefaultZoom() * viewport.width;
        }
#endif
        float const zoomScale = GAME_UNIT_SCALAR * ((1.0f / GAME_UNIT_SCALAR) * (currentZoomX / viewport.width));
        viewport.width *= zoomScale,
        viewport.height *= zoomScale;
        viewport.x = camera->getNode()->getTranslationX() - (viewport.width / 2.0f);
        viewport.y = camera->getNode()->getTranslationY() - (viewport.height / 2.0f);
        return viewport;
    }

    void CameraComponent::setZoom(float zoom)
    {
        _targetZoom = zoom;
    }

    gameplay::Vector3 const & CameraComponent::getPosition() const
    {
        return _camera->getNode()->getTranslation();
    }

    gameplay::Rectangle const & CameraComponent::getTargetBoundary() const
    {
        return _boundary;
    }

    gameplay::Vector3 const & CameraComponent::getTargetPosition() const
    {
        return _targetPosition;
    }
}
