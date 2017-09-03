#include "Common.h"

#include "Font.h"
#include "ScreenDisplayer.h"
#include "ScreenOverlay.h"
#include "Texture.h"
#include <random>

namespace game
{
#ifndef _FINAL
    /** @script{ignore} */
    struct LogDisplayMessage
    {
        gameplay::Logger::Level _level;
        char const * _message;
        int _secondsRemaining;
    };

    static std::deque<std::string> & getLogHistory()
    {
        static std::deque<std::string> logHistory;
        return logHistory;
    }
    
    /** @script{ignore} */
    struct LogRenderer
    {
        void render(void * message)
        {
            LogDisplayMessage * msg = static_cast<LogDisplayMessage*>(message);
            gameplay::Game::getInstance()->clear(gameplay::Game::CLEAR_COLOR_DEPTH, gameplay::Vector4::zero(), 1.0f, 0);
            gameplay::Font * font = gameplay::Font::create(getConfig()->getString("font"));
            font->finish();
            int const margin = 10;
            int x = margin;
            int y = margin;
            int const yPadding = font->getSize();
            std::array<char, UCHAR_MAX> buffer;
            sprintf(&buffer[0], "%s! Resuming in %d", msg->_level == gameplay::Logger::LEVEL_ERROR ? "ERROR" : "WARNING", msg->_secondsRemaining);
            font->start();
            font->drawText(&buffer[0], x, y += yPadding, gameplay::Vector4(1, 0, 0, 1));
            font->drawText(msg->_message, x, y += yPadding, gameplay::Vector4::one());
            y += yPadding;
            font->drawText("Recent log history:", x, y += yPadding, gameplay::Vector4::one());
            y += yPadding;

            for (auto itr = getLogHistory().rbegin(); itr != getLogHistory().rend(); ++itr)
            {
                font->drawText(itr->c_str(), x, y += yPadding, gameplay::Vector4::fromColor(0xDBFF28FF));
            }

            font->finish();
        }
    };

    void clearLogHistory()
    {
        getLogHistory().clear();
    }

    int & getIndent()
    {
        static int indent = -1;
        return indent;
    }
#endif

    gameplay::Gamepad * getGamepad()
    {
        return gameplay::Game::getInstance()->getGamepad(0, false);
    }

    gameplay::Properties * getConfig()
    {
        return gameplay::Game::getInstance()->getConfig();
    }

    void toggleSetting(const char * setting)
    {
        getConfig()->setString(setting, getConfig()->getBool(setting) ? "false" : "true");
    }

    float getRandomRange(float min, float max)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(min, max);
        return dis(gen);
    }

    StallScope::~StallScope()
    {
        ScreenOverlay::getInstance().renderImmediate();
    }

    void loggingCallback(gameplay::Logger::Level level, const char* msg)
    {
#ifndef _FINAL
        int const callsPerMessage = 3;
        static int callCount = 0;
        static std::string logOutput;
        static bool forceAssert = callCount == 0 && strcmp(msg, "lua assert:") == 0;
        
        if (level != gameplay::Logger::Level::LEVEL_INFO)
        {
            ++callCount;
            logOutput += msg;
            
            if (callCount == callsPerMessage)
            {
                gameplay::print(logOutput.c_str());

                int timeout = getConfig()->getInt("assert_timeout_ms");

                if (timeout > 0)
                {
                    LogDisplayMessage message;
                    message._level = level;
                    message._message = logOutput.c_str();
                    LogRenderer renderer;
                    int const timeoutTick = 1000;

                    while (timeout > 0)
                    {
                        message._secondsRemaining = timeout / timeoutTick;
                        gameplay::ScreenDisplayer().run(&renderer, &LogRenderer::render, &message, timeoutTick);
                        timeout -= timeoutTick;
                    }

                    if (forceAssert)
                    {
                        GP_ASSERT(false);
                    }
                }

                logOutput.clear();
                callCount = 0;
                forceAssert = false;
            }
        }
        else
        {
            if (getLogHistory().size() == GAME_ON_SCREEN_LOG_HISTORY_CAPACITY)
            {
                getLogHistory().pop_front();
            }

            getLogHistory().push_back(msg);
            gameplay::print(msg);
        }
#else
        if (level != gameplay::Logger::Level::LEVEL_INFO)
        {
            // <Insert User user facing crash notfication/log upload to a server>
        }
#endif
    }
}
