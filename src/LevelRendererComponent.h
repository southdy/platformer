#ifndef GAME_LEVEL_RENDERER_COMPONENT_H
#define GAME_LEVEL_RENDERER_COMPONENT_H

#include "Component.h"
#include "LevelLoaderComponent.h"
#include "SpriteAnimationComponent.h"

namespace game
{
    class CharacterRenderer;
    class CameraComponent;
    class EnemyComponent;
    class LevelPlatformsComponent;
    class LevelLoaderComponent;
    class PlayerComponent;
    class SpriteSheet;

    /**
     * Displays a level loaded by LevelLoaderComponent
     *
     * @script{ignore}
    */
    class LevelRendererComponent : public gameobjects::Component
    {
    public:
        explicit LevelRendererComponent();
        ~LevelRendererComponent();
    protected:
        virtual void initialize() override;
        virtual void finalize() override;
        virtual bool onMessageReceived(gameobjects::Message * message, int messageType) override;
        virtual void readProperties(gameplay::Properties & properties) override;
    private:
        struct ParallaxLayer
        {
            float _speed;
            gameplay::Rectangle _dst;
            gameplay::Rectangle _src;
            gameplay::Vector2 _offset;
            bool _cameraIndependent;
        };

        LevelRendererComponent(LevelRendererComponent const &);

        void onLevelLoaded();
        void onLevelUnloaded();
        void resizeTileMap();
        void createWaterDrawTargets();
        void cacheInteractableTextureTargets();
        void createRenderTargets(unsigned int width, unsigned int height);
        void createTileSpriteBatch(gameplay::SpriteBatch ** spriteBatch, std::vector<gameplay::SpriteBatch *> & spriteBatchesToInitialise);
        void createReusableSpriteBatches(std::vector<gameplay::SpriteBatch *> & spriteBatchesToInitialise);
        void createPlayerAnimationSpriteBatches(std::vector<gameplay::SpriteBatch *> & spriteBatchesToInitialise);
        void createEnemyAnimationSpriteBatches(std::vector<gameplay::SpriteBatch *> & spriteBatchesToInitialise);
        void render(float elapsedTime);
        void renderCharacters();
        void renderBackground(float elapsedTime);
        void renderInteractables();
        void renderTiles();
        void renderCollectables();
        void renderWater(float elapsedTime);
        float getWaterTimeUniform() const;

        struct Tile
        {
            int id;
            bool foreground;
        };

        bool _levelLoaded;
        bool _levelLoadedOnce;
        float _waterUniformTimer;
        std::vector<std::vector<Tile>> _tileMap;
        PlayerComponent * _player;
        LevelLoaderComponent * _level;
        std::map<int, gameplay::SpriteBatch *> _playerAnimationBatches;
        std::map<EnemyComponent *, std::map<int, gameplay::SpriteBatch *>> _enemyAnimationBatches;
        LevelPlatformsComponent * _platforms;
        gameplay::SpriteBatch * _backgroundTileBatch;
        gameplay::SpriteBatch * _foregroundTileBatch;
        CameraComponent * _camera;
        CharacterRenderer * _characterRenderer;
        gameplay::SpriteBatch * _pixelSpritebatch;
        SpriteSheet * _interactablesSpritesheet;
        std::vector<ParallaxLayer> _parallaxLayers;
        gameplay::SpriteBatch * _parallaxSpritebatch;
        gameplay::SpriteBatch * _interactablesSpritebatch;
        gameplay::SpriteBatch * _collectablesSpritebatch;
        gameplay::SpriteBatch * _waterSpritebatch;
        gameplay::Vector4 _parallaxFillColor;
        gameplay::Vector2 _parallaxOffset;
        std::vector<std::pair<gameplay::Node *, gameplay::Rectangle>> _dynamicCollisionNodes;
        std::vector<LevelLoaderComponent::Collectable *> _collectables;
        std::vector<gameplay::Rectangle> _waterBounds;
        gameplay::FrameBuffer * _frameBuffer;
        gameplay::SpriteBatch * _pauseSpriteBatch;
        gameplay::Matrix _viewProj;
        gameplay::Rectangle _viewport;
    };
}

#endif
