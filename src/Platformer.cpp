#include "Common.h"
#include "Platformer.h"

#include "AudioComponent.h"
#include "CameraComponent.h"
#include "Debug.h"
#include "PhysicsLoaderComponent.h"
#include "ProfilerController.h"
#include "EnemyComponent.h"
#include "GameObjectController.h"
#include "LevelPlatformsComponent.h"
#include "LevelCollisionComponent.h"
#include "LevelLoaderComponent.h"
#include "LevelRendererComponent.h"
#include "Messages.h"
#include "PlayerComponent.h"
#include "PlayerResetComponent.h"
#include "PlayerInputComponent.h"
#include "ResourceManager.h"
#include "ScriptController.h"
#include "Scene.h"
#include "ScreenOverlay.h"
#include "SpriteSheet.h"
#include "SpriteAnimationComponent.h"
#include "UI.h"

game::Platformer platformer;

namespace game
{
    Platformer::Platformer()
        : _keyMessage(nullptr)
        , _pinchMessage(nullptr)
        , _touchMessage(nullptr)
        , _mouseMessage(nullptr)
        , _renderMessage(nullptr)
        , _preSimulationUpdateMessage(nullptr)
        , _simulationUpdateMessage(nullptr)
        , _postSimulationUpdateMessage(nullptr)
        , _elapsedTimeToRender(0.0f)
    {
    }

    Platformer::~Platformer()
    {
    }

    void Platformer::initialize()
    {
        gameplay::Logger::set(gameplay::Logger::Level::LEVEL_INFO, loggingCallback);
        gameplay::Logger::set(gameplay::Logger::Level::LEVEL_WARN, loggingCallback);
        gameplay::Logger::set(gameplay::Logger::Level::LEVEL_ERROR, loggingCallback);
        ResourceManager::getInstance().initializeForBoot();
        ScreenOverlay::getInstance().initialize();
        UI::getInstance().initialize();
        DEBUG_INITIALIZE();
#ifndef GP_NO_LUA_BINDINGS
        if(getConfig()->getBool("run_tools"))
        {
            getScriptController()->loadScript("res/lua/run_tools.lua");

            if(getConfig()->getBool("run_tools_only"))
            {
                exit();
                return;
            }
        }
#endif
        setMultiTouch(true);

        if (isGestureSupported(gameplay::Gesture::GESTURE_PINCH))
        {
            registerGesture(gameplay::Gesture::GESTURE_PINCH);
        }

        if(gameplay::Properties * windowSettings = getConfig()->getNamespace("window", true))
        {
            char const * vsyncOption = "vsync";
            if(windowSettings->exists(vsyncOption))
            {
                setVsync(windowSettings->getBool(vsyncOption));
            }
        }

        ResourceManager::getInstance().initialize();
        gameobjects::GameObjectController::getInstance().registerComponent<CameraComponent>("camera");
        gameobjects::GameObjectController::getInstance().registerComponent<PhysicsLoaderComponent>("physics_loader");
        gameobjects::GameObjectController::getInstance().registerComponent<EnemyComponent>("enemy");
        gameobjects::GameObjectController::getInstance().registerComponent<LevelLoaderComponent>("level_loader");
        gameobjects::GameObjectController::getInstance().registerComponent<LevelRendererComponent>("level_renderer");
        gameobjects::GameObjectController::getInstance().registerComponent<AudioComponent>("audio");
        gameobjects::GameObjectController::getInstance().registerComponent<PlayerComponent>("player");
        gameobjects::GameObjectController::getInstance().registerComponent<PlayerResetComponent>("player_reset");
        gameobjects::GameObjectController::getInstance().registerComponent<PlayerInputComponent>("player_input");
        gameobjects::GameObjectController::getInstance().registerComponent<SpriteAnimationComponent>("sprite_animation");
        gameobjects::GameObjectController::getInstance().registerComponent<LevelCollisionComponent>("level_collision");
        gameobjects::GameObjectController::getInstance().registerComponent<LevelPlatformsComponent>("level_platforms");
        gameobjects::GameObjectController::getInstance().initialize();

        _pinchMessage = PinchMessage::create();
        _keyMessage = KeyMessage::create();
        _touchMessage = TouchMessage::create();
        _mouseMessage = MouseMessage::create();
        _renderMessage = RenderMessage::create();
        _preSimulationUpdateMessage = PreSimulationUpdateMessage::create();
        _simulationUpdateMessage = SimulationUpdateMessage::create();
        _postSimulationUpdateMessage = PostSimulationUpdateMessage::create();

        float const unitInCM = 100;
        float const unitInPixels = GAME_UNIT_SCALAR;
        gameplay::Vector3 const gravity = getPhysicsController()->getGravity() * (unitInCM * unitInPixels);
        gameplay::Camera * camera = gameplay::Camera::createOrthographic(getWidth(), getHeight(), getWidth() / getHeight(), DEFAULT_NEAR_PLANE, DEFAULT_FAR_PLANE);
        gameplay::Node * node = gameplay::Node::create("camera");
        node->setCamera(camera);
        getPhysicsController()->setGravity(gravity);
        gameobjects::GameObjectController::getInstance().getScene()->addNode(node);
        getAudioListener()->setCamera(nullptr);
        gameobjects::GameObject * rootGameObject = gameobjects::GameObjectController::getInstance().createGameObject("root");
        gameobjects::GameObjectController::getInstance().createGameObject("level", rootGameObject);
        gameobjects::Message * loadLevel = QueueLevelLoadMessage::create();
        QueueLevelLoadMessage::setAndBroadcast(loadLevel, getConfig()->getNamespace("boot", true)->getString("level"));
        gameobjects::Message::destroy(&loadLevel);
        camera->release();
        node->release();
    }

    void Platformer::finalize()
    {
#ifdef GP_USE_MEM_LEAK_DETECTION
        gameobjects::Message * exitMessage = ExitMessage::create();
        ExitMessage::setAndBroadcast(exitMessage);
        gameobjects::GameObjectController::getInstance().finalize();
        gameobjects::Message::destroy(&exitMessage);
        gameobjects::Message::destroy(&_pinchMessage);
        gameobjects::Message::destroy(&_keyMessage);
        gameobjects::Message::destroy(&_touchMessage);
        gameobjects::Message::destroy(&_mouseMessage);
        gameobjects::Message::destroy(&_renderMessage);
        gameobjects::Message::destroy(&_preSimulationUpdateMessage);
        gameobjects::Message::destroy(&_postSimulationUpdateMessage);
        gameobjects::Message::destroy(&_simulationUpdateMessage);
        UI::getInstance().finalize();
        DEBUG_FINALIZE();
        ScreenOverlay::getInstance().finalize();
        ResourceManager::getInstance().finalize();
#ifndef _FINAL
        clearLogHistory();
#endif
        void(*nullFuncPtr) (gameplay::Logger::Level, const char*) = nullptr;
        gameplay::Logger::set(gameplay::Logger::Level::LEVEL_INFO, nullFuncPtr);
        gameplay::Logger::set(gameplay::Logger::Level::LEVEL_WARN, nullFuncPtr);
        gameplay::Logger::set(gameplay::Logger::Level::LEVEL_ERROR, nullFuncPtr);
#endif
    }

    void Platformer::gesturePinchEvent(int x, int y, float scale)
    {
        if(_pinchMessage && !ScreenOverlay::getInstance().isVisible() && getState() != gameplay::Game::State::PAUSED)
        {
            PinchMessage::setAndBroadcast(_pinchMessage, x, y, scale);
        }
    }

    void Platformer::keyEvent(gameplay::Keyboard::KeyEvent evt, int key)
    {
        if (_keyMessage && !ScreenOverlay::getInstance().isVisible() && getState() != gameplay::Game::State::PAUSED)
        {
            KeyMessage::setAndBroadcast(_keyMessage, evt, key);
        }
        if (evt == gameplay::Keyboard::KEY_PRESS && key == gameplay::Keyboard::KEY_ESCAPE)
        {
            exit();
        }
        DEBUG_KEY_EVENT(evt, key);
    }

    void Platformer::touchEvent(gameplay::Touch::TouchEvent evt, int x, int y, unsigned int contactIndex)
    {
        if (_touchMessage && !ScreenOverlay::getInstance().isVisible() && getState() != gameplay::Game::State::PAUSED)
        {
            TouchMessage::setAndBroadcast(_touchMessage, evt, x, y, contactIndex);
        }
        DEBUG_CURSOR_EVENT(x,y);
    }

    bool Platformer::mouseEvent(gameplay::Mouse::MouseEvent evt, int x, int y, int wheelDelta)
    {
        if (_mouseMessage && !ScreenOverlay::getInstance().isVisible() && getState() != gameplay::Game::State::PAUSED)
        {
            MouseMessage::setAndBroadcast(_mouseMessage, evt, x, y, wheelDelta);
        }
        DEBUG_CURSOR_EVENT(x,y);
        return false;
    }

    void Platformer::resizeEvent(unsigned int, unsigned int)
    {
        DEBUG_RESIZE_EVENT();
    }

    void Platformer::preSimulationUpdate(float elapsedTime)
    {
        PROFILE();
        PreSimulationUpdateMessage::setAndBroadcast(_preSimulationUpdateMessage, elapsedTime);
    }

    void Platformer::simulationUpdate(float elapsedTime)
    {
        PROFILE();
        SimulationUpdateMessage::setAndBroadcast(_simulationUpdateMessage, elapsedTime);
    }

    void Platformer::postSimulationUpdate(float elapsedTime)
    {
        PROFILE();
        _elapsedTimeToRender = elapsedTime;
        UI::getInstance().update(elapsedTime);
        ScreenOverlay::getInstance().update(elapsedTime);
        PostSimulationUpdateMessage::setAndBroadcast(_postSimulationUpdateMessage, elapsedTime);
    }

    void Platformer::render(float)
    {
        DEBUG_RENDER();
        {
            PROFILE();
            RenderMessage::setAndBroadcast(_renderMessage, _elapsedTimeToRender);
            ScreenOverlay::getInstance().render();
            UI::getInstance().render();
            _elapsedTimeToRender = 0.0f;
        }
    }
}
