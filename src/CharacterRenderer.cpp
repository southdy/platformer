#include "CharacterRenderer.h"

#include "Common.h"
#include "SpriteAnimationComponent.h"

namespace game
{
    CharacterRenderer::CharacterRenderer()
        : _started(false)
        , _previousSpritebatch(nullptr)
        , _renderCount(0)
    {
    }

    void CharacterRenderer::start()
    {
        GAME_ASSERT(!_started, "Start called before Finish");
        _started = true;
        _renderCount = 0;
    }

    void CharacterRenderer::finish()
    {
        GAME_ASSERT(_started, "Finsh called before Start");
        _started = false;

        if (_previousSpritebatch)
        {
            _previousSpritebatch->finish();
            _previousSpritebatch = nullptr;
        }
    }

    unsigned int CharacterRenderer::getRenderCount() const
    {
        return _renderCount;
    }

    bool CharacterRenderer::render(SpriteAnimationComponent * animation, gameplay::SpriteBatch * spriteBatch,
        gameplay::Matrix const & projection, int orientation,
        gameplay::Vector3 const & position, gameplay::Rectangle const & viewport, float alpha)
    {
        bool wasRendered = false;
        SpriteAnimationComponent::DrawTarget drawTarget = animation->getDrawTarget(0.0f, static_cast<Sprite::Flip::Enum>(orientation));
        gameplay::Rectangle const bounds(position.x - ((fabs(drawTarget._scale.x / 2) * GAME_UNIT_SCALAR)),
            position.y - ((fabs(drawTarget._scale.y / 2) * GAME_UNIT_SCALAR)),
            fabs(drawTarget._scale.x) * GAME_UNIT_SCALAR,
            fabs(drawTarget._scale.y) * GAME_UNIT_SCALAR);

        if (bounds.intersects(viewport))
        {
            gameplay::Vector3 drawPosition = position / GAME_UNIT_SCALAR;
            drawPosition.x -= fabs(drawTarget._scale.x / 2);
            drawPosition.y += fabs(drawTarget._scale.y / 2);
            drawPosition.y *= -1.0f;
            drawTarget._dst.x += drawPosition.x;
            drawTarget._dst.y += drawPosition.y;

            if (_previousSpritebatch != spriteBatch)
            {
                if (_previousSpritebatch)
                {
                    _previousSpritebatch->finish();
                }

                spriteBatch->start();
            }

            spriteBatch->setProjectionMatrix(projection);
            spriteBatch->draw(drawTarget._dst, drawTarget._src, drawTarget._scale, gameplay::Vector4(1.0f, 1.0f, 1.0f, alpha));
            ++_renderCount;
            _previousSpritebatch = spriteBatch;
            wasRendered = true;
        }

        return wasRendered;
    }
}
