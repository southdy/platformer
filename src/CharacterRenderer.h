#ifndef GAME_CHARACTER_RENDERER_COMPONENT_H
#define GAME_CHARACTER_RENDERER_COMPONENT_H

namespace gameplay
{
    class Matrix;
    class Vector3;
    class SpriteBatch;
    class Rectangle;
}

namespace game
{
    class SpriteAnimationComponent;

    class CharacterRenderer
    {
    public:
        explicit CharacterRenderer();
        void start();
        void finish();
        unsigned int getRenderCount() const;
        bool render(SpriteAnimationComponent * animation, gameplay::SpriteBatch * spriteBatch,
            gameplay::Matrix const & spriteBatchProjection, int orientation,
            gameplay::Vector3 const & position, gameplay::Rectangle const & viewport, float alpha = 1.0f);
    private:
        gameplay::SpriteBatch * _previousSpritebatch;
        bool _started;
        unsigned int _renderCount;
    };
}

#endif
