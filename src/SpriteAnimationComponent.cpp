#include "SpriteAnimationComponent.h"

#include "Common.h"
#include "FileSystem.h"
#include "ResourceManager.h"
#include "SpriteSheet.h"

namespace game
{
    SpriteAnimationComponent::SpriteAnimationComponent()
        : _playbackState(PlaybackState::Stopped)
        , _elapsed(0.0f)
        , _frameIndex(0)
        , _frameCount(0)
        , _speed(1.0f)
        , _fps(30.0f)
        , _scale(1.0f)
        , _autoStart(false)
        , _loop(false)
    {
    }

    SpriteAnimationComponent::~SpriteAnimationComponent()
    {
    }

    void SpriteAnimationComponent::initialize()
    {
        _fps = 1000.0f / _fps;

        SpriteSheet * spriteSheet = ResourceManager::getInstance().getSpriteSheet(_spriteSheetPath);

        int frames = 0;
        std::array<char, UCHAR_MAX> str;

        if(_spriteNames.empty())
        {
            while (true)
            {
                bool done = false;

                if (strstr(_spritePrefix.c_str(), "__"))
                {
                    sprintf(&str[0], "%s%.2d", _spritePrefix.c_str(), frames++);
                }
                else
                {
                    sprintf(&str[0], "%s", _spritePrefix.c_str());
                    ++frames;
                    done = true;
                }

                Sprite * sprite = spriteSheet->getSprite(&str[0]);

                if (sprite)
                {
                    _sprites.push_back(*sprite);
                }

                if (!sprite || done)
                {
                    break;
                }
            }
        }
        else
        {
            for(std::string const spriteName : _spriteNames)
            {
                if(Sprite * sprite = spriteSheet->getSprite(spriteName))
                {
                    _sprites.push_back(*sprite);
                }
                else
                {
                    GAME_ASSERTFAIL("Couldn't find sprite '%s'", spriteName.c_str());
                }
            }
        }

        GAME_ASSERT(!_sprites.empty(), "%s contains no sprites", _spriteSheetPath.c_str());

        _frameCount = _sprites.size();

        if (_frameCount == 1)
        {
            _loop = true;
        }

        SAFE_RELEASE(spriteSheet);

        if (_autoStart)
        {
            play();
        }
    }

    void SpriteAnimationComponent::readProperties(gameplay::Properties & properties)
    {
        _autoStart = properties.getBool("autostart", _autoStart);
        _loop = properties.getBool("loop", _loop);
        gameobjects::setIfExists(properties, "fps", _fps);
        gameobjects::setIfExists(properties, "scale", _scale);
        _spriteSheetPath = properties.getString("spritesheet", _spriteSheetPath.c_str());
        GAME_ASSERT(gameplay::FileSystem::fileExists(_spriteSheetPath.c_str()), "Spritesheet '%s' not found", _spriteSheetPath.c_str());

        if(!properties.exists("sprite"))
        {
            _spritePrefix = properties.getString("spriteprefix", _spritePrefix.c_str());
            GAME_ASSERT(!_spritePrefix.empty(), "Sprite prefix cannot be empty");
        }
        else
        {
            while(char const * propertyName = properties.getNextProperty())
            {
                if(strcmp(propertyName, "sprite") == 0)
                {
                    _spriteNames.push_back(properties.getString());
                }
            }

            properties.rewind();

            GAME_ASSERT(!_spriteNames.empty(), "No sprites were specified");
        }
    }

    void SpriteAnimationComponent::update(float dt)
    {
        if (_playbackState == PlaybackState::Playing)
        {
            if (_frameCount > 1)
            {
                _elapsed += dt * _speed;

                if (fabs(_elapsed) >= _fps)
                {
                    bool isTimePositive = _elapsed > 0.0f;

                    int newFrameIndex = isTimePositive ? _frameIndex + 1 : _frameIndex - 1;

                    if (newFrameIndex == _frameCount)
                    {
                        _frameIndex = 0;

                        if (!_loop)
                        {
                            pause();
                        }
                    }
                    else if (newFrameIndex < 0)
                    {
                        _frameIndex = _frameCount - 1;
                    }
                    else
                    {
                        _frameIndex = isTimePositive ? ++_frameIndex : --_frameIndex;
                    }

                    _elapsed = 0.0f;
                }
            }
        }
    }

    SpriteAnimationComponent::DrawTarget SpriteAnimationComponent::getDrawTarget(float rotationRadians, Sprite::Flip::Enum flip) const
    {
        Sprite const & sprite = _sprites[_frameIndex];
        int width = (flip & Sprite::Flip::Horizontal) != 0 ? -sprite._src.width : sprite._src.width;
        int height = (flip & Sprite::Flip::Vertical) != 0 ? -sprite._src.height : sprite._src.height;
        gameplay::Vector2 realScale = gameplay::Vector2(width * _scale, height * _scale);
        gameplay::Vector2 flipOffset = gameplay::Vector2(width >= 0.0f ? 0.0f : fabs(realScale.x), height >= 0.0f ? 0.0f : fabs(realScale.y));
        gameplay::Vector3 destination = gameplay::Vector3(flipOffset.x, flipOffset.y, 0.0f);
        DrawTarget result;
        result._dst = destination;
        result._scale = realScale;
        result._src = sprite._src;
        return result;
    }

    void SpriteAnimationComponent::play()
    {
        _playbackState = PlaybackState::Playing;
    }

    void SpriteAnimationComponent::pause()
    {
        _playbackState = PlaybackState::Paused;
    }

    void SpriteAnimationComponent::stop()
    {
        _elapsed = 0.0f;
        _frameIndex = 0;
        _playbackState = PlaybackState::Stopped;
    }

    void SpriteAnimationComponent::setSpeed(float speed)
    {
        _speed = speed;
    }

    gameplay::Rectangle const & SpriteAnimationComponent::getCurrentSpriteSrc() const
    {
        return _sprites[_frameIndex]._src;
    }

    SpriteAnimationComponent::PlaybackState::Enum SpriteAnimationComponent::getState() const
    {
        return _playbackState;
    }

    float SpriteAnimationComponent::getSpeed() const
    {
        return _speed;
    }

    float SpriteAnimationComponent::getScale() const
    {
        return _scale;
    }

    bool SpriteAnimationComponent::getIsLooping() const
    {
        return _loop;
    }

    std::string const & SpriteAnimationComponent::getSpritePrefix() const
    {
        return _spritePrefix;
    }

    std::string const & SpriteAnimationComponent::getSpriteSheetPath() const
    {
        return _spriteSheetPath;
    }
}
