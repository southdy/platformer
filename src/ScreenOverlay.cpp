#include "ScreenOverlay.h"

#include "Common.h"
#include "Font.h"
#include "Game.h"
#include "GameObjectController.h"
#include "Messages.h"
#include "ResourceManager.h"
#include "SpriteSheet.h"

namespace  game
{
    static const gameplay::Vector4 LOADING_BG_COLOR(gameplay::Vector4::fromColor(0xF5F5F5FF));

    ScreenOverlay::ScreenOverlay()
        : _alpha(1.0f)
        , _previousAlpha(0.0f)
        , _fadeDuration(0.0f)
        , _fadeActive(false)
        , _texuturesSpriteBatch(nullptr)
        , _fillSpriteBatch(nullptr)
        , _isFadingIn(true)
        , _requestType(Request::Type::COLOR_FILL)
        , _wasUpdatedThisFrame(false)
        , _fadeTimer(0.0f)
        , _spinnerRotation(0.0f)
        , _previousSpinnerTime(0.0f)
        , _backgroundColor(gameplay::Vector4::one())
        , _fadeActiveMessage(nullptr)
    {
    }

    ScreenOverlay::~ScreenOverlay()
    {
    }

    ScreenOverlay::ScreenOverlay(ScreenOverlay const &)
    {
    }

    ScreenOverlay & ScreenOverlay::getInstance()
    {
        static ScreenOverlay instance;
        return instance;
    }

    void ScreenOverlay::initialize()
    {
        _fillSpriteBatch = ResourceManager::getInstance().createSinglePixelSpritebatch();

        SpriteSheet * textureSpritesheet = ResourceManager::getInstance().getSpriteSheet("res/spritesheets/screen.ss");
        _logoSrc = textureSpritesheet->getSprite("logo")->_src;;
        _spinnerSrc = textureSpritesheet->getSprite("spinner")->_src;
        _spinnerDst = _spinnerSrc;
        float const spinnerVeticalScreenScale = 0.33f;
        _spinnerDst.width = MATH_CLAMP(gameplay::Game::getInstance()->getHeight() * spinnerVeticalScreenScale, 0, _spinnerSrc.width);
        _spinnerDst.height = _spinnerDst.width;
        _spinnerDst.x = (gameplay::Game::getInstance()->getWidth() / 2) - (_spinnerDst.width / 2);
        _spinnerDst.y = (gameplay::Game::getInstance()->getHeight() / 2) - (_spinnerDst.height / 2);

        _bannersSrc = textureSpritesheet->getSprite("banners")->_src;
        _bannersDst.height = MATH_CLAMP(gameplay::Game::getInstance()->getHeight() - (_spinnerDst.width * 2.0f), 0, _bannersSrc.height);
        _bannersDst.width = ((1.0f / _bannersSrc.height) * _bannersDst.height) * _bannersSrc.width;
        _bannersDst.x = (gameplay::Game::getInstance()->getWidth() / 2) - (_bannersDst.width / 2);
        _bannersDst.y = (gameplay::Game::getInstance()->getHeight() / 2) + _spinnerDst.height / 2;

        _texuturesSpriteBatch = gameplay::SpriteBatch::create(textureSpritesheet->getTexture());
        SAFE_RELEASE(textureSpritesheet);
        _fadeActiveMessage = ScreenFadeStateChangedMessage::create();
        queueFadeToLoadingScreen(0.0f);
    }

    void ScreenOverlay::finalize()
    {
        SAFE_DELETE(_fillSpriteBatch);
        SAFE_DELETE(_texuturesSpriteBatch);
        gameobjects::Message::destroy(&_fadeActiveMessage);
    }

    void ScreenOverlay::update(float elapsedTime)
    {
        updateRequests(elapsedTime / 1000.0f);
        _wasUpdatedThisFrame = false;
    }

    void ScreenOverlay::setIsFadeActive(bool isFadeActive)
    {
        _fadeActive = isFadeActive;
        ScreenFadeStateChangedMessage::setAndBroadcast(_fadeActiveMessage, _fadeActive);
    }

    void ScreenOverlay::updateRequests(float dt)
    {
        if(_wasUpdatedThisFrame)
        {
            return;
        }

        if(!_fadeActive && !_requests.empty())
        {
            Request & request = _requests.front();
            _requests.pop();
            _fadeDuration = request._duration;
            _isFadingIn = request._isFadingIn;
            _requestType = request._type;
            _backgroundColor = request._backgroundColor;
            _fadeTimer = _fadeDuration;
            _alpha = _isFadingIn ? 0.0f : 1.0f;
            setIsFadeActive(true);
        }

        if(_fadeActive)
        {
            static float const minPlayableDt = 1.0f / 15.0f;

            if(_fadeTimer > 0.0f)
            {
                if (dt <= minPlayableDt)
                {
                    _fadeTimer = MATH_CLAMP(_fadeTimer - dt, 0.0f, std::numeric_limits<float>::max());
                    float const startAlpha = _isFadingIn ? 0.0f : 1.0f;
                    float const alphaDt = (1.0f / _fadeDuration) *
                            (_fadeDuration - _fadeTimer) *
                            (_isFadingIn ? 1.0f : -1.0f);
                    _alpha = startAlpha + alphaDt;
                }
            }
            else
            {
                _alpha = _isFadingIn ? 1.0f : 0.0f;
                setIsFadeActive(false);
                _fadeDuration = 0.0f;
                _fadeTimer = 0.0f;
            }
        }

        _wasUpdatedThisFrame = true;
    }


    bool ScreenOverlay::render()
    {
        PROFILE();
        bool stateChanged = false;
        if(isVisible())
        {
            gameplay::Vector4 bgColor = _backgroundColor;
            bgColor.w = _alpha;

            _fillSpriteBatch->start();
            _fillSpriteBatch->draw(gameplay::Rectangle(0, 0, gameplay::Game::getInstance()->getWidth(), gameplay::Game::getInstance()->getHeight()),
                                               gameplay::Rectangle(0, 0, 1, 1),
                                               bgColor);
            _fillSpriteBatch->finish();

            if(_requestType == Request::Type::LOADING_SPINNER)
            {
                _texuturesSpriteBatch->start();

                static float const spinnerDeltaMs = 200.0f;
                float const elapsed = gameplay::Game::getAbsoluteTime() - _previousSpinnerTime;
#ifndef _FINAL
                static float const spinnerDeltaPaddingMs = 50.0f;
                if (elapsed > spinnerDeltaMs + spinnerDeltaPaddingMs)
                {
                    std::array<char, 256> buffer;
                    sprintf(&buffer[0], "LOADING SPINNER STALLED %.3f", elapsed - (spinnerDeltaMs + spinnerDeltaPaddingMs));
                    GAME_LOG("%s", &buffer[0]);
                }
#endif
                bool const shouldRotate = elapsed >= spinnerDeltaMs || _previousAlpha == 0;

                if(shouldRotate)
                {
                    static float const rotationPerUpdate = MATH_DEG_TO_RAD(15.0f);
                    _spinnerRotation += rotationPerUpdate;
                    stateChanged |= true;
                    _previousSpinnerTime = gameplay::Game::getAbsoluteTime();
                }

                _texuturesSpriteBatch->draw(_spinnerDst, _logoSrc, gameplay::Vector4(1, 1, 1, _alpha));
                _texuturesSpriteBatch->draw(gameplay::Vector3(_spinnerDst.x, _spinnerDst.y + _spinnerDst.height, 0),
                                            _spinnerSrc, gameplay::Vector2(_spinnerDst.width, -_spinnerDst.height),
                                            gameplay::Vector4(1, 1, 1, _alpha),
                                            gameplay::Vector2(0.5f, 0.5f),
                                            _spinnerRotation);


                _texuturesSpriteBatch->draw(_bannersDst, _bannersSrc, gameplay::Vector4(1, 1, 1, _alpha));
                _texuturesSpriteBatch->finish();
            }
        }
        else
        {
            _previousSpinnerTime = gameplay::Game::getAbsoluteTime();

            if(gameplay::Game::getInstance()->getState() == gameplay::Game::State::PAUSED)
            {
                _fillSpriteBatch->start();
                _fillSpriteBatch->draw(gameplay::Rectangle(0, _bannersDst.y,gameplay::Game::getInstance()->getWidth(),
                                                           gameplay::Game::getInstance()->getHeight() - _bannersDst.y),
                                                            gameplay::Rectangle(0, 0, 1, 1),
                                                            LOADING_BG_COLOR);
                _fillSpriteBatch->draw(gameplay::Rectangle(0, _bannersDst.y,gameplay::Game::getInstance()->getWidth(),
                                                           (gameplay::Game::getInstance()->getHeight() - _bannersDst.y) * 0.025f),
                                                   gameplay::Rectangle(0, 0, 1, 1),
                                                   gameplay::Vector4(0, 0, 0, 1));
                _fillSpriteBatch->finish();
                _texuturesSpriteBatch->start();
                _texuturesSpriteBatch->draw(_bannersDst, _bannersSrc, gameplay::Vector4::one());
                _texuturesSpriteBatch->finish();
            }
        }

        stateChanged |= _previousAlpha != _alpha;
        _previousAlpha = _alpha;
        return stateChanged;
    }

    void ScreenOverlay::queue(Request const & request)
    {
        _requests.push(request);
        updateRequests();
        if(request._type == Request::Type::LOADING_SPINNER)
        {
            renderImmediate();
        }
    }

    void ScreenOverlay::queueFadeToBlack(float duration)
    {
        Request request;
        request._duration = duration;
        request._type = Request::Type::COLOR_FILL;
        request._isFadingIn = true;
        request._backgroundColor = gameplay::Vector4::fromColor(0x000000FF);
        queue(request);
    }

    void ScreenOverlay::queueFadeToLoadingScreen(float duration)
    {
        Request request;
        request._duration = duration;
        request._type = Request::Type::LOADING_SPINNER;
        request._isFadingIn = true;
        request._backgroundColor = LOADING_BG_COLOR;
        queue(request);
    }

    void ScreenOverlay::queueFadeOut(float duration)
    {
        Request request;
        request._duration = duration;
        request._type = _requestType;
        request._isFadingIn = false;
        request._backgroundColor = _backgroundColor;
        queue(request);
    }

    void ScreenOverlay::renderImmediate()
    {
        if(render())
        {
#if !_FINAL && !__ANDROID__
            static bool isFirstRender = false;

            if(!isFirstRender && getConfig()->getBool("run_tools"))
            {
                gameplay::Font * debugFont = ResourceManager::getInstance().getDebugFront();
                debugFont->start();
                debugFont->drawText("Running tools...", 5, 5, gameplay::Vector4(1, 0, 0, 1));
                debugFont->finish();
                isFirstRender = true;
            }
#endif
            gameplay::Platform::swapBuffers();
            glFinish(); // TODO: Move to platform layer
        }
    }

    bool ScreenOverlay::isVisible() const
    {
        bool visible = _alpha > 0.0f;
#ifndef _FINAL
        visible &= getConfig()->getBool("show_overlay");
#endif
        return visible;
    }

    float ScreenOverlay::getAlpha() const
    {
        float alpha = _alpha;
#ifndef _FINAL
        alpha = !getConfig()->getBool("show_overlay") ? 0.0f : _alpha;
#endif
        return alpha;
    }
}
