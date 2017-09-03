#ifndef GAME_H
#define GAME_H

#include "Game.h"
#include "GameObjectMessage.h"

namespace game
{
    /**
     * Message pump for game systems
     *
     * @script{ignore}
    */
    class Platformer : public gameplay::Game
    {
    public:
        explicit Platformer();
        virtual ~Platformer();
    protected:
        virtual void initialize() override;
        virtual void finalize() override;
        virtual void gesturePinchEvent(int x, int y, float scale) override;
        virtual void keyEvent(gameplay::Keyboard::KeyEvent evt, int key) override;
        virtual void touchEvent(gameplay::Touch::TouchEvent evt, int x, int y, unsigned int contactIndex) override;
        virtual bool mouseEvent(gameplay::Mouse::MouseEvent evt, int x, int y, int wheelDelta) override;
        virtual void resizeEvent(unsigned int, unsigned int) override;
        virtual void preSimulationUpdate(float elapsedTime) override;
        virtual void simulationUpdate(float elapsedTime) override;
        virtual void postSimulationUpdate(float elapsedTime) override;
        virtual void render(float) override;
    private:
        gameobjects::Message * _pinchMessage;
        gameobjects::Message * _keyMessage;
        gameobjects::Message * _touchMessage;
        gameobjects::Message * _mouseMessage;
        gameobjects::Message * _renderMessage;
        gameobjects::Message * _preSimulationUpdateMessage;
        gameobjects::Message * _simulationUpdateMessage;
        gameobjects::Message * _postSimulationUpdateMessage;
        float _elapsedTimeToRender;
    };
}

#endif
