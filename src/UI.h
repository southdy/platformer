#ifndef GAME_UI_H
#define GAME_UI_H

#include "Control.h"

namespace game
{
    class UI : public gameplay::Control::Listener
    {
        friend class Platformer;
    public:
        UI();
        static UI & getInstance();
    private:
        virtual void controlEvent(gameplay::Control * control, gameplay::Control::Listener::EventType) override;
        void initialize();
        void finalize();
        void update(float elapsedTime);
        void render();

        gameplay::Form * _optionsForm;
    };
}

#endif
