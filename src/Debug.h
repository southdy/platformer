#ifndef GAME_DEBUG
#define GAME_DEBUG

#include "GameObjectMessage.h"
#include "Keyboard.h"
#include "Rectangle.h"
#include <string>
#include <vector>
#include "Vector3.h"

#ifndef _FINAL
namespace gameplay
{
    class FrameBuffer;
    class LineBatch;
    class Matrix;
    class Node;
    class SpriteBatch;
    class Vector4;
}

namespace game
{
    /** @script{ignore} */
    class Debug
    {
        friend class Platformer;
        friend class RenderScope;
        friend struct EventRenderer;
    public:
        struct RenderScope
        {
            RenderScope();
            ~RenderScope();
        };
        Debug();
        static Debug & getInstance();
        void renderText(char const * setting, char const * message, ...);
        void renderTextWorld(gameplay::Vector3 const & position, char const * setting, char const * message, ...);
    private:
        enum TextType
        {
            None,
            Screen,
            World
        };
        void renderStart();
        void renderFinish();
        void initialize();
        void finalize();
        void renderPhysics();
        void renderNodes();
        void renderProfiler();
        void renderLegend(char const * description);
        void renderLegendToggle(bool const enabled, char const * description);
        void renderLegendToggleSetting(char const * setting, char const * description);
        bool renderNode(gameplay::Node * node);
        void keyEvent(gameplay::Keyboard::KeyEvent evt, int key);
        void resizeEvent();
        void cursorEvent(unsigned int x, unsigned int y);
        bool renderText(TextType textType, gameplay::Matrix const & view, gameplay::Vector3 const & position,
                        char const * setting, char const * message, ...);
        int _textY;
        int _nodeCount;
        int _nodeDepth;
        int _levelIndex;
        bool _rendererEnabled;
        bool _inputModifier;
        float _profilerPointSpacingX;
        unsigned int _cursorX;
        unsigned int _selectedProfilerIndex;
        TextType _textType;
        TextType _previousTextType;
        gameplay::FrameBuffer * _frameBuffer;
        gameplay::SpriteBatch * _batch;
        gameplay::LineBatch * _lineBatch;
        std::vector<std::string> _levels;
        gameobjects::Message * _loadMessage;
        gameplay::Rectangle _viewport;
        gameplay::Rectangle _profilerBounds;
    };
}

#define DEBUG_INITIALIZE() game::Debug::getInstance().initialize()
#define DEBUG_FINALIZE() game::Debug::getInstance().finalize()
#define DEBUG_KEY_EVENT(evt, key) game::Debug::getInstance().keyEvent(evt, key)
#define DEBUG_RESIZE_EVENT() game::Debug::getInstance().resizeEvent()
#define DEBUG_CURSOR_EVENT(x,y) game::Debug::getInstance().cursorEvent(x,y)
#define DEBUG_RENDER() game::Debug::RenderScope debugRenderScope
#define DEBUG_RENDER_TEXT(setting, message) game::Debug::getInstance().renderText(setting, message)
#define DEBUG_RENDER_TEXT_WITH_ARGS(setting, message, ...) game::Debug::getInstance().renderText(setting, message, __VA_ARGS__)
#define DEBUG_RENDER_WORLD_TEXT(position, setting, message, ...) game::Debug::getInstance().renderTextWorld(position, setting, message, __VA_ARGS__)
#else
#define DEBUG_INITIALIZE()
#define DEBUG_FINALIZE()
#define DEBUG_KEY_EVENT(evt, key)
#define DEBUG_RESIZE_EVENT()
#define DEBUG_CURSOR_EVENT(x,y)
#define DEBUG_RENDER()
#define DEBUG_RENDER_TEXT(setting, message, ...)
#define DEBUG_RENDER_TEXT_WITH_ARGS(setting, message, ...)
#define DEBUG_RENDER_WORLD_TEXT(position, setting, message, ...)
#endif
#endif
