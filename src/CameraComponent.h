#ifndef GAME_CAMERA_COMPONENT_H
#define GAME_CAMERA_COMPONENT_H

#include "Component.h"

namespace game
{
    class PlayerComponent;

    /**
     * Controls the active camera, any smoothing/zooming etc should be applied by and configured via this component
     *
     * @script{ignore}
    */
    class CameraComponent : public gameobjects::Component
    {
    public:
        explicit CameraComponent();
        ~CameraComponent();

        static gameplay::Matrix getRenderViewProjectionMatrix();
        static gameplay::Matrix const & getViewProjectionMatrix();
        static gameplay::Rectangle getRenderViewport();
        static float getMinZoom();
        static float getMaxZoom();
        static float getDefaultZoom();

        float getZoom() const;
        float getTargetZoom() const;
        void setZoom(float zoom);
        void setBoundary(gameplay::Rectangle boundary);
        void setPosition(gameplay::Vector3 const & position);
        gameplay::Vector3 const &  getPosition() const;
        gameplay::Vector3 const & getTargetPosition() const;
        gameplay::Rectangle const & getTargetBoundary() const;
    protected:
        virtual void readProperties(gameplay::Properties & properties) override;
        virtual void onStart() override;
        virtual void finalize() override;
        virtual bool onMessageReceived(gameobjects::Message *message, int messageType) override;
    private:
        CameraComponent(CameraComponent const &);

        void onPostSimulationUpdate(gameplay::Vector3 const & target, float elapsedTime);

        gameplay::Camera * _camera;
        float _initialCurrentZoom;
        float _initialTargetZoom;
        float _currentZoom;
        float _previousZoom;
        float _targetZoom;
        float _zoomSpeedScale;
        float _smoothSpeedScale;
        gameplay::Rectangle _boundary;
        gameplay::Vector2 _targetBoundaryScale;
        gameplay::Vector3 _targetPosition;
        PlayerComponent * _player;
    };
}

#endif
