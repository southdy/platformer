#include "UI.h"

#include "Common.h"
#include "Form.h"
#include "Game.h"
#include "ScreenOverlay.h"

namespace game
{
    char const * PAUSE_TOGGLE_ID = "pauseToggle";
    char const * SOUND_TOGGLE_ID = "soundToggle";

    UI & UI::getInstance()
    {
        static UI instance;
        return instance;
    }

    UI::UI()
        : _optionsForm(nullptr)
    {
    }

    void UI::initialize()
    {
        _optionsForm = gameplay::Form::create("res/ui/options.form");
        _optionsForm->getControl(PAUSE_TOGGLE_ID)->addListener(this, gameplay::Control::Listener::VALUE_CHANGED);
        _optionsForm->getControl(SOUND_TOGGLE_ID)->addListener(this, gameplay::Control::Listener::VALUE_CHANGED);
#ifndef _FINAL
        if(getConfig()->getBool("mute"))
        {
            controlEvent(_optionsForm->getControl(SOUND_TOGGLE_ID), gameplay::Control::Listener::EventType::PRESS);
        }
#endif
    }

    void UI::finalize()
    {
        _optionsForm->getControl(PAUSE_TOGGLE_ID)->removeListener(this);
        _optionsForm->getControl(SOUND_TOGGLE_ID)->removeListener(this);
        SAFE_RELEASE(_optionsForm);
    }

    void UI::controlEvent(gameplay::Control * control, gameplay::Control::Listener::EventType)
    {
        if(strcmp(control->getId(), PAUSE_TOGGLE_ID) == 0)
        {
            // TODO: Fix resume for android, workaround
            bool const isPaused = gameplay::Game::getInstance()->getState() == gameplay::Game::State::PAUSED;
            for(int i = 0; i < 256; ++i)
            {
                 isPaused ? gameplay::Game::getInstance()->resume() : gameplay::Game::getInstance()->pause();
            }
        }
        else if(strcmp(control->getId(), SOUND_TOGGLE_ID) == 0)
        {
            gameplay::Game::getInstance()->getAudioListener()->setGain(
                        gameplay::Game::getInstance()->getAudioListener()->getGain() == 1.0f ? 0.0f : 1.0f);
        }
    }

    void UI::update(float elapsedTime)
    {
        bool enabled = !ScreenOverlay::getInstance().isVisible();
#ifndef _FINAL
        enabled &= getConfig()->getBool("show_ui");
#endif
        _optionsForm->setEnabled(enabled);
        if (gameplay::Gamepad * gamepad = getGamepad())
        {
            if (gameplay::Form * gamepadForm = gamepad->getForm())
            {
                gamepadForm->setEnabled(enabled);
            }
        }
    }

    void UI::render()
    {
        bool visible = !ScreenOverlay::getInstance().isVisible();
#ifndef _FINAL
        visible &= getConfig()->getBool("show_ui");
#endif
        if (visible)
        {
            PROFILE();
            if(gameplay::Game::getInstance()->getState() != gameplay::Game::State::PAUSED)
            {
                if (gameplay::Gamepad * gamepad = getGamepad())
                {
                    if (gameplay::Form * gamepadForm = gamepad->getForm())
                    {
                        gamepadForm->draw();
                    }
                }
            }
            _optionsForm->draw();
        }
    }
}
