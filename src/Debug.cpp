#ifndef _FINAL

#include "Debug.h"

#include "CameraComponent.h"
#include "Common.h"
#include "Font.h"
#include "Game.h"
#include "GameObjectController.h"
#include "LineBatch.h"
#include "Messages.h"
#include "ProfilerController.h"
#include "Properties.h"
#include "ResourceManager.h"
#include "Scene.h"
#include "SpriteBatch.h"

namespace  game
{
    Debug::RenderScope::RenderScope()
    {
        game::Debug::getInstance().renderStart();
    }

    Debug::RenderScope::~RenderScope()
    {
        game::Debug::getInstance().renderFinish();
    }

    struct DebugDrawScope
    {
        DebugDrawScope(gameplay::FrameBuffer * buffer)
        {
            _previousFrameBuffer = buffer->bind();
        }

        ~DebugDrawScope()
        {
            _previousFrameBuffer->bind();
        }
        gameplay::FrameBuffer * _previousFrameBuffer;
    };

    #define FRAME_BUFFER_SCOPE() DebugDrawScope frameBufferScope(_frameBuffer)

    Debug & Debug::getInstance()
    {
        static Debug instance;
        return instance;
    }

    Debug::Debug()
        : _frameBuffer(nullptr)
        , _batch(nullptr)
        , _lineBatch(nullptr)
        , _loadMessage(nullptr)
        , _textType(TextType::None)
        , _rendererEnabled(true)
        , _inputModifier(false)
        , _nodeCount(0)
        , _nodeDepth(0)
        , _textY(0)
        , _levelIndex(0)
        , _selectedProfilerIndex(0)
    {
    }

    void Debug::initialize()
    {
        _loadMessage = QueueLevelLoadMessage::create();
        gameplay::FileSystem::listFiles("res/levels", _levels);
        for(std::string & level : _levels)
        {
            level = "res/levels/" + level;
        }
        gameplay::Properties * bootConfig = getConfig()->getNamespace("boot", true);
        auto itr = std::find(_levels.begin(), _levels.end(), bootConfig->getString("level"));
        _levelIndex = itr != _levels.end() ? itr - _levels.begin() : _levelIndex;
        _rendererEnabled = getConfig()->getBool("show_debug");
        _frameBuffer = gameplay::FrameBuffer::create("debug_buffer");
        gameplay::RenderTarget * renderTarget = gameplay::RenderTarget::create("pause",
            gameplay::Game::getInstance()->getWidth(), gameplay::Game::getInstance()->getHeight());
        _frameBuffer->setRenderTarget(renderTarget);
        _batch = gameplay::SpriteBatch::create(renderTarget->getTexture());
        _lineBatch = gameplay::LineBatch::create();
        resizeEvent();
        SAFE_RELEASE(renderTarget);
    }

    void Debug::finalize()
    {
        gameobjects::Message::destroy(&_loadMessage);
        SAFE_RELEASE(_frameBuffer);
        SAFE_DELETE(_batch);
        SAFE_DELETE(_lineBatch);
    }

    void Debug::keyEvent(gameplay::Keyboard::KeyEvent evt, int key)
    {
        gameplay::Game * game = gameplay::Game::getInstance();
        switch(evt)
        {
            case gameplay::Keyboard::KeyEvent::KEY_PRESS:
            {
                if(!_inputModifier)
                {
                    switch(key)
                    {
                    case gameplay::Keyboard::KEY_F1:
                        toggleSetting("show_legend");
                        break;
                    case gameplay::Keyboard::KEY_F2:
                        toggleSetting("show_level");
                        break;
                    case gameplay::Keyboard::KEY_F3:
                        toggleSetting("show_physics");
                        break;
                    case gameplay::Keyboard::KEY_F4:
                        toggleSetting("show_ui");
                        break;
                    case gameplay::Keyboard::KEY_F5:
                        QueueLevelLoadMessage::setAndBroadcast(_loadMessage, _levels[_levelIndex].c_str());
                        break;
                    case gameplay::Keyboard::KEY_F6:
                        game->getProfilerController()->setRecording(!game->getProfilerController()->isRecording());
                        break;
                    case gameplay::Keyboard::KEY_F7:
                        toggleSetting("show_profiler");
                        break;
                    case gameplay::Keyboard::KEY_F8:
                        toggleSetting("multi_jump");
                        break;
                    case gameplay::Keyboard::KEY_F9:
                        toggleSetting("show_stats");
                        break;
#ifndef WIN32
                    case gameplay::Keyboard::KEY_F10:
                        game->setVsync(!game->isVsync());
                        break;
#endif
                    case gameplay::Keyboard::KEY_TAB:
                        _rendererEnabled = !_rendererEnabled;
                        break;
                    case gameplay::Keyboard::KEY_SHIFT:
                        _inputModifier = true;
                        break;
                    }
                }
                else
                {
                    float const timeScaleDelta = 0.25f;
                    float const minTimeScale = 0.0f;
                    float const maxTimeScale = 5.0f;
                    switch(key)
                    {
                    case gameplay::Keyboard::KEY_F1:
                        toggleSetting("show_culling");
                        break;
                    case gameplay::Keyboard::KEY_F2:
                        game->getPhysicsController()->setEnabled(!game->getPhysicsController()->isEnabled());
                        break;
                    case gameplay::Keyboard::KEY_F3:
                        toggleSetting("clamp_camera");
                        break;
                    case gameplay::Keyboard::KEY_F4:
                        toggleSetting("show_overlay");
                        break;
                    case gameplay::Keyboard::KEY_F5:
                        toggleSetting("show_nodes");
                        break;
                    case gameplay::Keyboard::KEY_F6:
                        toggleSetting("show_level_stats");
                        break;
                    case gameplay::Keyboard::KEY_F7:
                        toggleSetting("show_player_stats");
                        toggleSetting("show_enemy_stats");
                        toggleSetting("show_platform_stats");
                        break;
                    case gameplay::Keyboard::KEY_UP_ARROW:
                        game->setTimeScale(MATH_CLAMP(game->getTimeScale() + timeScaleDelta, minTimeScale, maxTimeScale));
                        break;
                    case gameplay::Keyboard::KEY_DOWN_ARROW:
                        game->setTimeScale(MATH_CLAMP(game->getTimeScale() - timeScaleDelta, minTimeScale, maxTimeScale));
                        break;
                    case gameplay::Keyboard::KEY_LEFT_ARROW:
                    case gameplay::Keyboard::KEY_RIGHT_ARROW:
                        {
                            int previousLevelIndex = _levelIndex;
                            _levelIndex = MATH_CLAMP(_levelIndex + (key == gameplay::Keyboard::KEY_RIGHT_ARROW ? 1 : -1), 0, _levels.size() - 1);
                            if(_levelIndex != previousLevelIndex)
                            {
                                QueueLevelLoadMessage::setAndBroadcast(_loadMessage, _levels[_levelIndex].c_str());
                            }
                        }
                        break;
                    }
                }
            }
            break;
        case gameplay::Keyboard::KEY_RELEASE:
            if(key == gameplay::Keyboard::KEY_SHIFT)
            {
                _inputModifier = false;
            }
            break;
        default:
            break;
        }
    }

    void Debug::resizeEvent()
    {
        gameplay::Game * game = gameplay::Game::getInstance();
        _viewport = game->getViewport();
        _profilerBounds = _viewport;
        _profilerBounds.height /= 5;
        _profilerPointSpacingX = std::max(1.0f, _profilerBounds.width / game->getProfilerController()->getFrameHistorySize());
    }

    void Debug::cursorEvent(unsigned int x, unsigned int)
    {
        _cursorX = x;
    }

    void Debug::renderStart()
    {
        gameplay::Game * game = gameplay::Game::getInstance();
        bool const isLevelRendering = getConfig()->getBool("show_level");
        gameplay::Vector4 const & clearColor = isLevelRendering ? gameplay::Vector4::zero() : gameplay::Vector4(0,0,0,1);
        float const clearDepth = 0;
        int const clearStencil = 0;
        if(!isLevelRendering)
        {
            game->clear(gameplay::Game::CLEAR_COLOR, clearColor, clearDepth, clearStencil);
        }
        if(_rendererEnabled)
        {
            PROFILE();
            _previousTextType = TextType::None;
            _textY = 0;
            {
                FRAME_BUFFER_SCOPE();
                game->clear(gameplay::Game::CLEAR_COLOR, clearColor, clearDepth, clearStencil);
            }
            DEBUG_RENDER_TEXT_WITH_ARGS("show_stats", "[%dfps/%.2fms][%dx%d][%.2fdt]",
                game->getFrameRate(),
                1000.0f / game->getFrameRate(),
                game->getWidth(),
                game->getHeight(),
                game->getTimeScale());
            renderLegend(                                                   "ARROWS            - move");
            renderLegend(                                                   "SPACE             - jump");
            renderLegend(                                                   "PGUP/PGDOWN       - +/- zoom");
            renderLegend(                                                   "SHIFT+UP/DOWN     - +/- time scale");
            renderLegend(                                                   "SHIFT+LEFT/RIGHT  - +/- level");
            renderLegendToggleSetting("show_debug",                         "TAB               - toggle debug render       ");
            renderLegendToggleSetting("show_legend",                        "F1                - toggle legend             ");
            renderLegendToggleSetting("show_level",                         "F2                - toggle level render       ");
            renderLegendToggleSetting("show_physics",                       "F3                - toggle wireframes         ");
            renderLegendToggleSetting("show_ui",                            "F4                - toggle ui                 ");
            renderLegend(                                                   "F5                - reload level");
            renderLegendToggle(game->getProfilerController()->isRecording(),"F6                - toggle profiler           ");
            renderLegendToggleSetting("show_profiler",                      "F7                - toggle profiler render    ");
            renderLegendToggleSetting("multi_jump",                         "F8                - toggle multi jump         ");
            renderLegendToggleSetting("show_stats",                         "F9                - toggle stats              ");
#ifndef WIN32
            renderLegendToggle(game->isVsync(), "F10               - toggle vsync              ");
#endif
            renderLegendToggleSetting("show_culling",                       "SHIFT+F1          - toggle culling            ");
            renderLegendToggle(game->getPhysicsController()->isEnabled(),   "SHIFT+F2          - toggle physics            ");
            renderLegendToggleSetting("clamp_camera",                       "SHIFT+F3          - toggle camera clamp       ");
            renderLegendToggleSetting("show_overlay",                       "SHIFT+F4          - toggle overlay            ");
            renderLegendToggleSetting("show_nodes",                         "SHIFT+F5          - toggle nodes              ");
            renderLegendToggleSetting("show_level_stats",                   "SHIFT+F6          - toggle level stats        ");
            renderLegendToggleSetting("show_physics_stats",                 "SHIFT+F7          - toggle physics stats      ");
        }
    }

    void Debug::renderFinish()
    {
        if(_rendererEnabled)
        {
            PROFILE();
            renderPhysics();
            renderNodes();
            renderProfiler();
            gameplay::Font * font = ResourceManager::getInstance().getDebugFront();
            if(font->getSpriteBatch(font->getSize())->isStarted())
            {
                FRAME_BUFFER_SCOPE();
                font->finish();
            }
            _batch->start();
            _batch->draw(_viewport, _viewport);
            _batch->finish();
        }
    }

    void Debug::renderPhysics()
    {
        if(_rendererEnabled && getConfig()->getBool("show_physics"))
        {
            FRAME_BUFFER_SCOPE();
            gameplay::Game::getInstance()->getPhysicsController()->drawDebug(CameraComponent::getViewProjectionMatrix(), CameraComponent::getRenderViewport());
        }
    }

    void Debug::renderNodes()
    {
        if (_rendererEnabled && getConfig()->getBool("show_nodes"))
        {
            renderText("show_nodes", "%d", _nodeCount);
            _nodeCount = 0;
            _nodeDepth = 0;
            gameobjects::GameObjectController::getInstance().getScene()->visit(this, &Debug::renderNode);
        }
    }

    #define PROFILER_NAME_COLUMN_WIDTH 80

    struct EventRenderer : public gameplay::ProfilerController::EventReader
    {
        virtual void read(const Event& event) override
        {
            double const frameDuration = gameplay::Game::getInstance()->getProfilerController()->getDuration(Debug::getInstance()._selectedProfilerIndex);
            Debug::getInstance().renderText("show_profiler", "%*s%-*s%4d %8.3f %8.3f %8.3f %8.3f %8.1f",
                event._depth,
                "",
                PROFILER_NAME_COLUMN_WIDTH - event._depth,
                event._name,
                event._hits,
                event._minTime,
                event._maxTime,
                event._totalTime / event._hits,
                event._totalTime,
                (100.0f / frameDuration) * event._totalTime);
        }
    };

    void Debug::renderProfiler()
    {
        if (_rendererEnabled && getConfig()->getBool("show_profiler"))
        {
            gameplay::Matrix view;
            gameplay::Matrix::createOrthographicOffCenter(0, _viewport.width, _viewport.height, 0, 0, 1, &view);
            gameplay::ProfilerController * profiler = gameplay::Game::getInstance()->getProfilerController();
            static gameplay::Vector4 const targetMsColor(0.0, 1.0f, 0.0f, 1.0f);
            _lineBatch->start(view, _viewport);
            _lineBatch->addLine(
                        gameplay::Vector3(_profilerBounds.x, _viewport.height - _profilerBounds.height, 0.0f),
                        gameplay::Vector3(_profilerBounds.width, _viewport.height - _profilerBounds.height, 0.0f),
                        targetMsColor,
                        targetMsColor);
            unsigned int index =  profiler->getFrameIndex() + 1;
            gameplay::Vector3 previousPoint;
            previousPoint.x = -_profilerPointSpacingX;
            previousPoint.y = 0;
            while(true)
            {
                if(index == profiler->getFrameHistorySize())
                {
                    index = 0;
                }
                static gameplay::Vector4 const lineColor(1.0, 0.0f, 0.0f, 1.0f);
                gameplay::Vector3 graphFramePosition = previousPoint;
                graphFramePosition.x += _profilerPointSpacingX;
                float const scale = (1.0f / ((1.0f / 60.0f) * 1000.0f)) * profiler->getDuration(index);
                gameplay::Rectangle const & viewport = gameplay::Game::getInstance()->getViewport();
                graphFramePosition.y =  viewport.height - (_profilerBounds.height * scale);
                _lineBatch->addLine(previousPoint, graphFramePosition, lineColor, lineColor);
                previousPoint = graphFramePosition;
                if(index == profiler->getFrameIndex())
                {
                    break;
                }
                ++index;
            }

            if(!profiler->isRecording())
            {
                static gameplay::Vector4 const selectionColor(0.0, 0.0f, 1.0f, 1.0f);
                float const scale = MATH_CLAMP((1.0f / _profilerBounds.width) * _cursorX, 0.0f, 1.0f);
                unsigned int const renderIndex = profiler->getFrameHistorySize() * scale;
                float const selectionX = _profilerPointSpacingX * renderIndex;
                _lineBatch->addLine(
                            gameplay::Vector3(selectionX, _viewport.height, 0),
                            gameplay::Vector3(selectionX, 0, 0),
                            selectionColor,
                            selectionColor);
                unsigned int const size = profiler->getFrameHistorySize();
                _selectedProfilerIndex = profiler->getFrameIndex() + 1;
                if(_selectedProfilerIndex == size)
                {
                    _selectedProfilerIndex = 0;
                }
                _selectedProfilerIndex += renderIndex;
                if(_selectedProfilerIndex >= size)
                {
                    _selectedProfilerIndex -= size;
                }
                renderText("show_profiler", "%*sHits      Min      Max  Average    Total", PROFILER_NAME_COLUMN_WIDTH , "");
                static EventRenderer framePrinter;
                profiler->getFrameEvents(_selectedProfilerIndex, &framePrinter);
            }

            FRAME_BUFFER_SCOPE();
            _lineBatch->finish();
        }
    }

    void Debug::renderLegend(char const * description)
    {
        DEBUG_RENDER_TEXT("show_legend", description);
    }

    void Debug::renderLegendToggle(bool const enabled, char const * description)
    {
        DEBUG_RENDER_TEXT_WITH_ARGS("show_legend", "%s [%s]", description, enabled ? "+" : "-");
    }

    void Debug::renderLegendToggleSetting(char const * setting, char const * description)
    {
        renderLegendToggle(gameplay::Game::getInstance()->getConfig()->getBool(setting), description);
    }

    bool Debug::renderNode(gameplay::Node * node)
    {
        bool const isFirstChild = !node->getPreviousSibling() && node->getParent();
        bool const isLastChild = node->getPreviousSibling() && !node->getNextSibling();
        bool const hasChildren = node->getChildCount() > 0;
        _nodeDepth += isFirstChild ? 1 : 0;
        char const * id = node->getId() ? node->getId() : "NO_ID";
        renderText("show_nodes", "%s%s[%d]", std::string(_nodeDepth, ' ').c_str(), id, node->getRefCount());
        _nodeDepth -= isLastChild && !hasChildren ? 1 : 0;
        ++_nodeCount;
        return true;
    }

    void Debug::renderText(char const * setting, char const * message, ...)
    {
        if(_rendererEnabled)
        {
            va_list args;
            va_start(args, message);
            static std::array<char, 4096> buffer;
            vsprintf(&buffer[0], message, args);
            va_end(args);
            gameplay::Matrix view;
            gameplay::Matrix::createOrthographicOffCenter(0, gameplay::Game::getInstance()->getWidth(),
                gameplay::Game::getInstance()->getHeight(), 0, 0, 1, &view);
            int const paddingX = 5;
            int const paddingY = ResourceManager::getInstance().getDebugFront()->getSize() * 0.75f;
            if(renderText(TextType::Screen, view, gameplay::Vector3(paddingX, _textY, 0), setting, &buffer[0]))
            {
                _textY += paddingY;
            }
        }
    }

    void Debug::renderTextWorld(gameplay::Vector3 const & position, char const * setting, char const * message, ...)
    {
        if(_rendererEnabled)
        {
            gameplay::Font * font = ResourceManager::getInstance().getDebugFront();
            va_list args;
            va_start(args, message);
            static std::array<char, CHAR_MAX> buffer;
            vsprintf(&buffer[0], message, args);
            va_end(args);
            gameplay::Vector3 renderPosition = position;
            renderPosition.y *= -1.0f;
            renderPosition.scale(1.0f / GAME_UNIT_SCALAR);
            unsigned int pixelWidth, pixelHeight = 0;
            char const * text = &buffer[0];
            font->measureText(text, font->getSize(GAME_FONT_SIZE_LARGE_INDEX), &pixelWidth, &pixelHeight);
            renderPosition.x -= pixelWidth * 0.25f;
            renderText(TextType::World, CameraComponent::getRenderViewProjectionMatrix(), renderPosition, setting, text);
        }
    }

    bool Debug::renderText(TextType textType, gameplay::Matrix const & view, gameplay::Vector3 const & position, char const * setting, char const * message, ...)
    {
        if(getConfig()->getBool(setting))
        {
            gameplay::Font * font = ResourceManager::getInstance().getDebugFront();
            gameplay::SpriteBatch * batch = font->getSpriteBatch(font->getSize());
            if(batch->isStarted() && _previousTextType != TextType::None && _previousTextType != textType)
            {
                FRAME_BUFFER_SCOPE();
                font->finish();
            }
            static std::array<char, 4096> buffer;
            va_list args;
            va_start(args, message);
            va_end(args);
            vsprintf(&buffer[0], message, args);
            if(!batch->isStarted() && textType == TextType::World)
            {
                font->drawText("", 0, 0, gameplay::Vector4::one());
            }
            batch->setProjectionMatrix(view);
            static const gameplay::Vector4 color = gameplay::Vector4(1, 0, 0, 1);
            font->drawText(&buffer[0], position.x, position.y, color);
            _previousTextType = textType;
            return true;
        }
        return false;
    }
}

#endif
