#ifndef GAME_PLAYER_HOG_COMPONENT_H
#define GAME_PLAYER_HOG_COMPONENT_H

#include "Component.h"

namespace game
{
    class CameraComponent;
    class PlayerComponent;

    /**
     * Moves the player to the spawn position when they venture outside the boundaries of the current level
     *
     * @script{ignore}
    */
    class PlayerResetComponent : public gameobjects::Component
    {
    public:
        explicit PlayerResetComponent();
        ~PlayerResetComponent();
    protected:
        virtual void finalize() override;
        virtual void onStart() override;
        virtual bool onMessageReceived(gameobjects::Message * message, int messageType) override;
    private:
        void onPostSimulationUpdate();
        gameplay::Vector3 _resetPosition;
        gameplay::Rectangle _boundary;
        PlayerComponent * _player;
        CameraComponent * _camera;
        bool _levelLoaded;
        bool _forceNextReset;
    };
}

#endif
