#ifndef GAME_INPUT_COMPONENT_H
#define GAME_INPUT_COMPONENT_H

#include "Component.h"
#include "Messages.h"
#include "PlayerComponent.h"

namespace gameplay
{
    class Gamepad;
}

namespace game
{
    class CameraComponent;

    /**
     * Applies keyboard and gamepad input to a sibling PlayerComponent
     *
     * @script{ignore}
    */
    class PlayerInputComponent : public gameobjects::Component
    {
    public:
        explicit PlayerInputComponent();
        ~PlayerInputComponent();
    protected:
        virtual void initialize() override;
        virtual void finalize() override;
        virtual void onStart() override;
        virtual bool onMessageReceived(gameobjects::Message * message, int messageType) override;
    private:
        void onPreSimulationUpdate(float elapsedTime);
        struct GamepadButtons
        {
            enum Enum
            {
                Jump,
                EnumCount
            };
        };

        PlayerInputComponent(PlayerInputComponent const &);
        bool isGamepadButtonPressed(GamepadButtons::Enum button) const;
        bool isGamepadButtonReleased(GamepadButtons::Enum button) const;
        float calculateCameraZoomStep(float scale) const;

        void onKeyboardInput(KeyMessage keyMessage);
        void onMouseInput(MouseMessage mouseMessage);
        void onPinchInput(PinchMessage pinchMessage);

        CameraComponent * _camera;
        PlayerComponent * _player;
        gameplay::Gamepad * _gamePad;
        std::bitset<GamepadButtons::EnumCount> _gamepadButtonState;
        std::bitset<GamepadButtons::EnumCount> _previousGamepadButtonState;
        std::bitset<gameplay::Keyboard::Key::KEY_SEARCH + 1> _keyState;
        std::array<int, GamepadButtons::EnumCount> _gamepadButtonMapping;
        gameplay::Vector2 _previousJoystickValue;
        bool _pinchEnabled;
        float _keyboardZoomScale;
    };
}

#endif
