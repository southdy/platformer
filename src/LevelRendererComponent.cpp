#include "LevelRendererComponent.h"

#include "CameraComponent.h"
#include "Common.h"
#include "CharacterRenderer.h"
#include "Debug.h"
#include "ProfilerController.h"
#include "EnemyComponent.h"
#include "Game.h"
#include "GameObject.h"
#include "GameObjectController.h"
#include "LevelPlatformsComponent.h"
#include "Messages.h"
#include "PlayerComponent.h"
#include "PhysicsCharacter.h"
#include "PropertiesRef.h"
#include "ResourceManager.h"
#include "Scene.h"
#include "ScreenOverlay.h"
#include "SpriteSheet.h"

namespace game
{
    LevelRendererComponent::LevelRendererComponent()
        : _levelLoaded(false)
        , _levelLoadedOnce(false)
        , _player(nullptr)
        , _platforms(nullptr)
        , _level(nullptr)
        , _backgroundTileBatch(nullptr)
        , _foregroundTileBatch(nullptr)
        , _camera(nullptr)
        , _parallaxSpritebatch(nullptr)
        , _interactablesSpritebatch(nullptr)
        , _collectablesSpritebatch(nullptr)
        , _pixelSpritebatch(nullptr)
        , _interactablesSpritesheet(nullptr)
        , _waterSpritebatch(nullptr)
        , _frameBuffer(nullptr)
        , _waterUniformTimer(0.0f)
    {
    }

    LevelRendererComponent::~LevelRendererComponent()
    {
    }

    bool LevelRendererComponent::onMessageReceived(gameobjects::Message * message, int messageType)
    {
        switch (messageType)
        {
        case(Messages::Type::LevelLoaded):
            onLevelUnloaded();
            onLevelLoaded();
            break;
        case(Messages::Type::LevelUnloaded):
            onLevelUnloaded();
            break;
        case(Messages::Type::Render):
        {
            RenderMessage msg(message);
            render(msg._elapsedTime);
        }
            return false;
        }

        return true;
    }

    gameplay::Rectangle getSafeDrawRect(gameplay::Rectangle const & src, float paddingX = 0.5f, float paddingY = 0.5f)
    {
        return gameplay::Rectangle(src.x + paddingX, src.y + paddingY, src.width - (paddingX * 2), src.height - (paddingY * 2));
    }

    gameplay::Rectangle getRenderDestination(gameplay::Rectangle const & worldDestination)
    {
        gameplay::Rectangle result;
        result.width = worldDestination.width / GAME_UNIT_SCALAR;
        result.height = worldDestination.height / GAME_UNIT_SCALAR;
        result.x = worldDestination.x / GAME_UNIT_SCALAR;
        result.y = -worldDestination.y / GAME_UNIT_SCALAR - result.height;
        return result;
    }

    void LevelRendererComponent::resizeTileMap()
    {
        _tileMap.resize(_level->getHeight());

        for (auto & horizontalTiles : _tileMap)
        {
            horizontalTiles.resize(_level->getWidth());
        }

        for (int y = 0; y < _level->getHeight(); ++y)
        {
            for (int x = 0; x < _level->getWidth(); ++x)
            {
                _tileMap[y][x].id = _level->getTile(x, y);
                _tileMap[y][x].foreground = false;
            }
        }
    }

    void LevelRendererComponent::createTileSpriteBatch(gameplay::SpriteBatch ** spriteBatch, std::vector<gameplay::SpriteBatch *> & spriteBatchesToInitialise)
    {
        *spriteBatch = gameplay::SpriteBatch::create(_level->getTexturePath().c_str());
        (*spriteBatch)->getSampler()->setFilterMode(gameplay::Texture::Filter::LINEAR, gameplay::Texture::Filter::LINEAR);
        (*spriteBatch)->getSampler()->setWrapMode(gameplay::Texture::Wrap::CLAMP, gameplay::Texture::Wrap::CLAMP);
        spriteBatchesToInitialise.push_back(*spriteBatch);
    }

    void LevelRendererComponent::createPlayerAnimationSpriteBatches(std::vector<gameplay::SpriteBatch *> & spriteBatchesToInitialise)
    {
        _player->forEachAnimation([this, &spriteBatchesToInitialise](PlayerComponent::State::Enum state, SpriteAnimationComponent * animation) -> bool
        {
            SpriteSheet * animSheet = ResourceManager::getInstance().getSpriteSheet(animation->getSpriteSheetPath());
            gameplay::SpriteBatch * spriteBatch = gameplay::SpriteBatch::create(animSheet->getTexture());
            spriteBatch->getSampler()->setFilterMode(gameplay::Texture::Filter::LINEAR, gameplay::Texture::Filter::LINEAR);
            spriteBatch->getSampler()->setWrapMode(gameplay::Texture::Wrap::CLAMP, gameplay::Texture::Wrap::CLAMP);
            _playerAnimationBatches[state] = spriteBatch;
            spriteBatchesToInitialise.push_back(spriteBatch);
            SAFE_RELEASE(animSheet);
            return false;
        });
    }

    void LevelRendererComponent::createEnemyAnimationSpriteBatches(std::vector<gameplay::SpriteBatch *> & spriteBatchesToInitialise)
    {
        std::map<std::string, gameplay::SpriteBatch *> enemyspriteBatchesToInitialise;
        std::vector<EnemyComponent *> enemies;
        _level->getParent()->getComponentsInChildren(enemies);

        for (EnemyComponent * enemy : enemies)
        {
            enemy->addRef();

            enemy->forEachAnimation([this, &enemyspriteBatchesToInitialise, &enemy, &spriteBatchesToInitialise](EnemyComponent::State::Enum state, SpriteAnimationComponent * animation) -> bool
            {
                SpriteSheet * animSheet = ResourceManager::getInstance().getSpriteSheet(animation->getSpriteSheetPath());
                gameplay::SpriteBatch * spriteBatch = nullptr;

                auto enemyBatchItr = enemyspriteBatchesToInitialise.find(animation->getSpriteSheetPath());

                if (enemyBatchItr != enemyspriteBatchesToInitialise.end())
                {
                    spriteBatch = enemyBatchItr->second;
                }
                else
                {
                    spriteBatch = gameplay::SpriteBatch::create(animSheet->getTexture());
                    spriteBatch->getSampler()->setFilterMode(gameplay::Texture::Filter::LINEAR, gameplay::Texture::Filter::LINEAR);
                    spriteBatch->getSampler()->setWrapMode(gameplay::Texture::Wrap::CLAMP, gameplay::Texture::Wrap::CLAMP);
                    enemyspriteBatchesToInitialise[animation->getSpriteSheetPath()] = spriteBatch;
                    spriteBatchesToInitialise.push_back(spriteBatch);
                }

                _enemyAnimationBatches[enemy][state] = spriteBatch;
                SAFE_RELEASE(animSheet);
                return false;
            });
        }
    }

    void LevelRendererComponent::createRenderTargets(unsigned int width, unsigned int height)
    {
        _frameBuffer = gameplay::FrameBuffer::create("lr_buffer");
        gameplay::RenderTarget * pauseRenderTarget = gameplay::RenderTarget::create("pause", width, height);
        _frameBuffer->setRenderTarget(pauseRenderTarget);
        gameplay::Effect * pauseEffect = gameplay::Effect::createFromFile("res/shaders/sprite.vert", "res/shaders/sepia.frag");
        _pauseSpriteBatch = gameplay::SpriteBatch::create(pauseRenderTarget->getTexture(), pauseEffect);
        SAFE_RELEASE(pauseEffect);
        SAFE_RELEASE(pauseRenderTarget);
    }

    void LevelRendererComponent::createReusableSpriteBatches(std::vector<gameplay::SpriteBatch *> & spriteBatchesToInitialise)
    {
        if (!_levelLoadedOnce)
        {
            _pixelSpritebatch = ResourceManager::getInstance().createSinglePixelSpritebatch();
            spriteBatchesToInitialise.push_back(_pixelSpritebatch);

            spriteBatchesToInitialise.push_back(_parallaxSpritebatch);

            _interactablesSpritesheet = ResourceManager::getInstance().getSpriteSheet("res/spritesheets/interactables.ss");
            _interactablesSpritebatch = gameplay::SpriteBatch::create(_interactablesSpritesheet->getTexture());
            _interactablesSpritebatch->getSampler()->setFilterMode(gameplay::Texture::Filter::LINEAR, gameplay::Texture::Filter::LINEAR);
            spriteBatchesToInitialise.push_back(_interactablesSpritebatch);

            gameplay::Effect* waterEffect = gameplay::Effect::createFromFile("res/shaders/sprite.vert", "res/shaders/water.frag");
            _waterSpritebatch = gameplay::SpriteBatch::create("@res/textures/water", waterEffect);
            _waterSpritebatch->getSampler()->setFilterMode(gameplay::Texture::Filter::NEAREST, gameplay::Texture::Filter::NEAREST);
            _waterSpritebatch->getSampler()->setWrapMode(gameplay::Texture::Wrap::REPEAT, gameplay::Texture::Wrap::CLAMP);
            SAFE_RELEASE(waterEffect);
            spriteBatchesToInitialise.push_back(_waterSpritebatch);
            gameplay::Material* waterMaterial = _waterSpritebatch->getMaterial();
            gameplay::Texture::Sampler* noiseSampler = gameplay::Texture::Sampler::create("@res/textures/water-noise");
            waterMaterial->getParameter("u_texture_noise")->setValue(noiseSampler);
            SAFE_RELEASE(noiseSampler);
            waterMaterial->getParameter("u_time")->bindValue(this, &LevelRendererComponent::getWaterTimeUniform);

            SpriteSheet * collectablesSpriteSheet = ResourceManager::getInstance().getSpriteSheet("res/spritesheets/collectables.ss");
            _collectablesSpritebatch = gameplay::SpriteBatch::create(collectablesSpriteSheet->getTexture());
            SAFE_RELEASE(collectablesSpriteSheet);
            spriteBatchesToInitialise.push_back(_collectablesSpritebatch);

            createRenderTargets(gameplay::Game::getInstance()->getWidth(), gameplay::Game::getInstance()->getHeight());
        }
    }

    void LevelRendererComponent::cacheInteractableTextureTargets()
    {
        std::vector<Sprite*> crateSprites;
        _interactablesSpritesheet->getSprites("crate_", crateSprites);

        _level->forEachCachedNode(collision::Type::DYNAMIC, [this, &crateSprites](gameplay::Node * node)
        {
            node->addRef();
            bool const isBoulder = node->getCollisionObject()->getShapeType() == gameplay::PhysicsCollisionShape::SHAPE_SPHERE;
            _dynamicCollisionNodes.push_back(std::make_pair(node,
                                                getSafeDrawRect(isBoulder ? _interactablesSpritesheet->getSprite("boulder")->_src :
                                                crateSprites[getRandomRange(0, crateSprites.size() -1)]->_src)));
        });

        _level->forEachCachedNode(collision::Type::BRIDGE, [this](gameplay::Node * node)
        {
            node->addRef();
            _dynamicCollisionNodes.push_back(std::make_pair(node, _interactablesSpritesheet->getSprite("bridge")->_src));
        });

        _level->forEachCachedNode(collision::Type::KINEMATIC, [this](gameplay::Node * node)
        {
            node->addRef();
            _dynamicCollisionNodes.push_back(std::make_pair(node, _interactablesSpritesheet->getSprite("platform")->_src));
        });
    }

    void LevelRendererComponent::createWaterDrawTargets()
    {
        _level->forEachCachedNode(collision::Type::WATER, [this](gameplay::Node * node)
        {
            float const submergeHeight = _player->getNode()->getScaleY() / 2;
            gameplay::Rectangle bounds;
            bounds.width = node->getScaleX();
            bounds.height = node->getScaleY() + submergeHeight;
            bounds.x = node->getTranslationX() - bounds.width / 2.0f;
            bounds.y = node->getTranslationY() + (submergeHeight * 0.5f) - bounds.height / 2.0f;
            // Scale the boundary height to add the area that was removed to make room for waves in the texture (95px of 512px)
            float const textureScale = 1.18f;
            bounds.height *= textureScale;

            gameplay::Rectangle waterTileArea;
            waterTileArea.width = (bounds.width / GAME_UNIT_SCALAR) / _level->getTileWidth();
            waterTileArea.height = (bounds.height / GAME_UNIT_SCALAR) / _level->getTileHeight();
            waterTileArea.x = (bounds.x / GAME_UNIT_SCALAR) / _level->getTileWidth();
            waterTileArea.y = _level->getHeight() - (((bounds.y + bounds.height) / GAME_UNIT_SCALAR) / _level->getTileHeight());
            int const targetX = MATH_CLAMP(waterTileArea.x + waterTileArea.width, 0, _level->getWidth());
            int const targetY = MATH_CLAMP(waterTileArea.y + waterTileArea.height, 0, _level->getHeight());

            for (int y = MATH_CLAMP(waterTileArea.y, 0, std::numeric_limits<float>::max()); y < targetY; ++y)
            {
                for (int x = MATH_CLAMP(waterTileArea.x, 0, std::numeric_limits<float>::max()); x < targetX; ++x)
                {
                    _tileMap[y][x].foreground = _tileMap[y][x].id != LevelLoaderComponent::EMPTY_TILE;
                }
            }
            _waterBounds.push_back(bounds);
        });
    }

    void LevelRendererComponent::onLevelLoaded()
    {
        PROFILE();

        std::vector<gameplay::SpriteBatch *> spriteBatchesToInitialise;
        _player = _level->getParent()->getComponentInChildren<PlayerComponent>();
        _player->addRef();
        _camera->setBoundary(gameplay::Rectangle(
                                 0,
                                 0,
                                 (_level->getTileWidth() * _level->getWidth())  * GAME_UNIT_SCALAR,
                                 std::numeric_limits<float>::max()));
        _level->getCollectables(_collectables);
        _waterUniformTimer = 0.0f;
        _platforms = _level->getParent()->getComponentInChildren<LevelPlatformsComponent>();
        GAME_SAFE_ADD(_platforms);

        resizeTileMap();
        createReusableSpriteBatches(spriteBatchesToInitialise);
        createTileSpriteBatch(&_backgroundTileBatch, spriteBatchesToInitialise);
        createTileSpriteBatch(&_foregroundTileBatch, spriteBatchesToInitialise);
        createPlayerAnimationSpriteBatches(spriteBatchesToInitialise);
        createEnemyAnimationSpriteBatches(spriteBatchesToInitialise);
        cacheInteractableTextureTargets();
        createWaterDrawTargets();

        // The first call to draw will perform some lazy initialisation in Effect::Bind
        for (gameplay::SpriteBatch * spriteBatch : spriteBatchesToInitialise)
        {
            spriteBatch->start();
            spriteBatch->draw(gameplay::Rectangle(), gameplay::Rectangle());
            spriteBatch->finish();
        }

        _levelLoaded = true;
        _levelLoadedOnce = true;

        float const fadeOutDuration = 1.5f;
        ScreenOverlay::getInstance().queueFadeOut(fadeOutDuration);
    }

    void LevelRendererComponent::onLevelUnloaded()
    {
        PROFILE();

        SAFE_RELEASE(_player);
        SAFE_RELEASE(_platforms);
        SAFE_DELETE(_backgroundTileBatch);
        SAFE_DELETE(_foregroundTileBatch);

        for (auto & playerAnimBatchPairItr : _playerAnimationBatches)
        {
            SAFE_DELETE(playerAnimBatchPairItr.second);
        }

        static std::set<gameplay::SpriteBatch *> uniqueEnemyBatches;

        for (auto & enemyAnimPairItr : _enemyAnimationBatches)
        {
            EnemyComponent * enemy = enemyAnimPairItr.first;
            SAFE_RELEASE(enemy);

            for (auto & enemyAnimBatchPairItr : enemyAnimPairItr.second)
            {
                uniqueEnemyBatches.insert(enemyAnimBatchPairItr.second);
            }
        }

        for(gameplay::SpriteBatch * spriteBatch : uniqueEnemyBatches)
        {
            SAFE_DELETE(spriteBatch);
        }

        uniqueEnemyBatches.clear();

        for (auto & nodePair : _dynamicCollisionNodes)
        {
            SAFE_RELEASE(nodePair.first);
        }

        _dynamicCollisionNodes.clear();
        _playerAnimationBatches.clear();
        _enemyAnimationBatches.clear();
        _waterBounds.clear();
        _collectables.clear();
        _tileMap.clear();

        if(_levelLoaded)
        {
            ScreenOverlay::getInstance().queueFadeToLoadingScreen(0.0f);
        }

        _levelLoaded = false;
    }

    void LevelRendererComponent::initialize()
    {
        _level = getRootParent()->getComponentInChildren<LevelLoaderComponent>();
        _level->addRef();
        _camera = getRootParent()->getComponentInChildren<CameraComponent>();
        _camera->addRef();
        _characterRenderer = new CharacterRenderer();
    }

    void LevelRendererComponent::finalize()
    {
        SAFE_RELEASE(_level);
        SAFE_RELEASE(_camera);
        SAFE_DELETE(_pixelSpritebatch);
        SAFE_DELETE(_parallaxSpritebatch);
        SAFE_DELETE(_interactablesSpritebatch);
        SAFE_DELETE(_collectablesSpritebatch);
        SAFE_DELETE(_waterSpritebatch);
        SAFE_DELETE(_pauseSpriteBatch);
        SAFE_DELETE(_characterRenderer);
        SAFE_RELEASE(_interactablesSpritesheet);
        SAFE_RELEASE(_frameBuffer);
        onLevelUnloaded();
    }

    void LevelRendererComponent::readProperties(gameplay::Properties & properties)
    {
        if(properties.exists("parallax"))
        {
            if (gameplay::PropertiesRef * parallaxRef = ResourceManager::getInstance().getProperties(properties.getString("parallax")))
            {
                gameplay::Properties * ns = parallaxRef->get();
                SpriteSheet * spritesheet = ResourceManager::getInstance().getSpriteSheet(ns->getString("spritesheet"));
                ns->getVector4("fill", &_parallaxFillColor);
                ns->getVector2("offset", &_parallaxOffset);
                _parallaxOffset *= GAME_UNIT_SCALAR;
                _parallaxSpritebatch = gameplay::SpriteBatch::create(spritesheet->getTexture());
                _parallaxSpritebatch->getSampler()->setWrapMode(gameplay::Texture::Wrap::REPEAT, gameplay::Texture::Wrap::CLAMP);
                _parallaxSpritebatch->getSampler()->setFilterMode(gameplay::Texture::Filter::NEAREST, gameplay::Texture::Filter::NEAREST);

                while (gameplay::Properties * childNs = ns->getNextNamespace())
                {
                    if (strcmp(childNs->getNamespace(), "layer") == 0)
                    {
                        ParallaxLayer layer;
                        layer._src = spritesheet->getSprite(childNs->getString("id"))->_src;
                        layer._dst = layer._src;
                        layer._dst.width *= GAME_UNIT_SCALAR;
                        layer._dst.height *= GAME_UNIT_SCALAR;
                        childNs->getVector2("offset", &layer._offset);
                        layer._offset *= GAME_UNIT_SCALAR;
                        layer._dst.y = layer._offset.y + _parallaxOffset.y;
                        layer._src.x = layer._offset.x + _parallaxOffset.x;
                        layer._speed = childNs->getFloat("speed");
                        layer._cameraIndependent = childNs->getBool("camera_independent");
                        _parallaxLayers.push_back(layer);
                    }
                }

                SAFE_RELEASE(spritesheet);
                ns->rewind();
                SAFE_RELEASE(parallaxRef);
            }
        }
    }

    void LevelRendererComponent::renderBackground(float elapsedTime)
    {
        // Clear the screen to the colour of the sky
        static unsigned int const SKY_COLOR = 0xD0F4F7FF;
        gameplay::Game::getInstance()->clear(gameplay::Game::ClearFlags::CLEAR_COLOR, gameplay::Vector4::fromColor(SKY_COLOR), 1.0f, 0);

        // Draw the solid colour parallax fill layer
        float const layerWidth = _viewport.width;
        float const layerPosX = _viewport.x;

        gameplay::Rectangle parallaxFillLayer(layerPosX, 0.0f, layerWidth, _parallaxOffset.y);

        if(parallaxFillLayer.intersects(_viewport))
        {
            _pixelSpritebatch->setProjectionMatrix(_viewProj);
            _pixelSpritebatch->start();
            _pixelSpritebatch->draw(getRenderDestination(parallaxFillLayer), gameplay::Rectangle(), _parallaxFillColor);
            _pixelSpritebatch->finish();
        }

        // Draw the parallax texture layers
        int parallaxLayerDrawn = 0;

        for(auto itr = _parallaxLayers.rbegin(); itr != _parallaxLayers.rend(); ++itr)
        {
            ParallaxLayer & layer = *itr;

            if (layer._cameraIndependent && gameplay::Game::getInstance()->getState() == gameplay::Game::State::RUNNING)
            {
                layer._src.x += (layer._speed * (elapsedTime / 1000.0f)) / GAME_UNIT_SCALAR;
            }

            layer._dst.width = layerWidth;
            layer._dst.x = layerPosX;

            if (layer._dst.intersects(_viewport))
            {
                if (parallaxLayerDrawn == 0)
                {
                    _parallaxSpritebatch->setProjectionMatrix(_viewProj);
                    _parallaxSpritebatch->start();
                }

                ++parallaxLayerDrawn;

                if (!layer._cameraIndependent)
                {
                    layer._src.x = ((layerPosX + layer._offset.x + _parallaxOffset.x + _viewport.x) * layer._speed) / GAME_UNIT_SCALAR;
                }

                layer._src.width = layer._dst.width / GAME_UNIT_SCALAR;

                _parallaxSpritebatch->draw(getRenderDestination(layer._dst), getSafeDrawRect(layer._src, 0, 0.5f));
            }
        }

        DEBUG_RENDER_TEXT_WITH_ARGS("show_level_stats", "layers        [%d/%d]", parallaxLayerDrawn, _parallaxLayers.size());

        if (parallaxLayerDrawn)
        {
            _parallaxSpritebatch->finish();
        }
    }

    void LevelRendererComponent::renderTiles()
    {
        int tilesRendered = 0;
        int const tileWidth = _level->getTileWidth();
        int const tileHeight = _level->getTileHeight();

        float const renderedOffsetY = _level->getHeight() * tileHeight;
        gameplay::Rectangle renderedViewport = getRenderDestination(_viewport);
        renderedViewport.y *= -1.0f;
        renderedViewport.y -= renderedOffsetY;

        int const minXUnClamped = ceil((renderedViewport.x - tileWidth) / tileWidth);
        int const minX = MATH_CLAMP(minXUnClamped, 0, _level->getWidth());
        int const maxX = MATH_CLAMP(minXUnClamped + ((renderedViewport.width + tileWidth * 2.0f) / tileWidth), 0, _level->getWidth());

        int const minYUnClamped = -ceil((renderedViewport.y) / tileHeight);
        int const minY = MATH_CLAMP(minYUnClamped, 0, _level->getHeight());
        int const maxY = MATH_CLAMP(minYUnClamped + ((renderedViewport.height + (tileHeight * 2.0f)) / tileHeight), 0, _level->getHeight());

        gameplay::Rectangle const levelArea((_level->getTileWidth() * _level->getWidth()) * GAME_UNIT_SCALAR,
            (_level->getTileHeight() * _level->getHeight()) * GAME_UNIT_SCALAR);

        if(levelArea.intersects(_viewport))
        {
            // Draw only the tiles the player can see
            float const tileTexureScale = 4;
            int tileTextureWidth = tileWidth * tileTexureScale;
            int tileTextureHeight = tileHeight * tileTexureScale;
            int const numSpritesX = _backgroundTileBatch->getSampler()->getTexture()->getWidth() / tileTextureWidth;

            for (int y = minY; y < maxY; ++y)
            {
                for (int x = minX; x < maxX; ++x)
                {
                    Tile const & tile =  _tileMap[y][x];

                    if (tile.id != LevelLoaderComponent::EMPTY_TILE)
                    {
                        if(tilesRendered == 0)
                        {
                            _backgroundTileBatch->start();
                            _foregroundTileBatch->start();
                        }
                        ++tilesRendered;
                        int const tileIndex = tile.id - 1;
                        int const tileX = (tileIndex % numSpritesX) * tileTextureWidth;
                        int const tileY = (tileIndex / numSpritesX) * tileTextureWidth;
                        gameplay::Rectangle const dst(x * tileWidth, (y * tileHeight) - renderedOffsetY, tileWidth, tileHeight);
                        gameplay::Rectangle const src(getSafeDrawRect(gameplay::Rectangle(tileX, tileY, tileTextureWidth, tileTextureHeight)));
                        gameplay::SpriteBatch * batch = tile.foreground ? _foregroundTileBatch : _backgroundTileBatch;
                        batch->setProjectionMatrix(_viewProj);
                        batch->draw(dst,src);
                    }
                }
            }
            _backgroundTileBatch->finish();
        }

        int const maxVisibleTiles = (maxX - minX) * (maxY - minY);
        DEBUG_RENDER_TEXT_WITH_ARGS("show_level_stats", "tiles         [%d/%d]", tilesRendered, maxVisibleTiles);
    }

    void LevelRendererComponent::renderInteractables()
    {
        int interactableDrawn = 0;

        // Draw dynamic collision (crates, boulders etc)
        for (auto & nodePair : _dynamicCollisionNodes)
        {
            gameplay::Node * node = nodePair.first;
            gameplay::Rectangle dst;
            dst.width = node->getScaleX();
            dst.height = node->getScaleY();
            collision::NodeData * data = collision::NodeData::get(node);
            bool const isPlatform = data->_type == collision::Type::KINEMATIC;
            gameplay::Vector3 const nodePosition = isPlatform ? _platforms->getRenderPosition(node) : node->getTranslation();
            dst.x = nodePosition.x - (dst.width / 2);
            dst.y = nodePosition.y - (dst.height / 2);

            gameplay::Rectangle maxBounds = dst;

            // Extend non-spherical shapes for viewport intersection test, prevents shapes being culled when still visible during rotation
            if(node->getCollisionObject()->getShapeType() != gameplay::PhysicsCollisionShape::Type::SHAPE_SPHERE)
            {
                maxBounds.width = gameplay::Vector2(dst.width, dst.height).length();
                maxBounds.height = maxBounds.width;
                maxBounds.x = nodePosition.x - (maxBounds.width / 2);
                maxBounds.y = nodePosition.y - (maxBounds.height / 2);
            }

            if (maxBounds.intersects(_viewport))
            {
                if (interactableDrawn == 0)
                {
                    _interactablesSpritebatch->setProjectionMatrix(_viewProj);
                    _interactablesSpritebatch->start();
                }

                ++interactableDrawn;

                gameplay::Quaternion const & q = node->getRotation();
                float const rotation = -static_cast<float>(atan2f(2.0f * q.x * q.y + 2.0f * q.z * q.w, 1.0f - 2.0f * ((q.y * q.y) + (q.z * q.z))));
                gameplay::Rectangle const renderDst = getRenderDestination(dst);
                _interactablesSpritebatch->draw(gameplay::Vector3(renderDst.x, renderDst.y, 0),
                    nodePair.second,
                    gameplay::Vector2(renderDst.width, renderDst.height),
                    gameplay::Vector4::one(),
                    (gameplay::Vector2::one() / 2),
                    rotation);

                if(isPlatform)
                {
                    DEBUG_RENDER_WORLD_TEXT(nodePosition, "show_platform_stats",
                                          GAME_VEC3_STR "\n" GAME_VEC3_STR,
                                          GAME_VEC3_ARG(nodePosition),
                                          GAME_VEC3_ARG(static_cast<gameplay::PhysicsRigidBody*>(node->getCollisionObject())->getLinearVelocity()));
                }
            }
        }

        DEBUG_RENDER_TEXT_WITH_ARGS("show_level_stats", "interactables [%d/%d]", interactableDrawn, _dynamicCollisionNodes.size());

        if (interactableDrawn)
        {
            _interactablesSpritebatch->finish();
        }
    }

    void LevelRendererComponent::renderCollectables()
    {
        int collectableDrawn = 0;

        for(LevelLoaderComponent::Collectable * collectable : _collectables)
        {
            gameplay::Rectangle dst;
            dst.width = collectable->_node->getScaleX();
            dst.height = collectable->_node->getScaleY();
            dst.x = collectable->_startPosition.x - dst.width / 2;
            dst.y = collectable->_startPosition.y - dst.height / 2;

            if (collectable->_active)
            {
                collectable->_visible = dst.intersects(_viewport);

                if (collectable->_visible)
                {
                    if (collectableDrawn == 0)
                    {
                        _collectablesSpritebatch->setProjectionMatrix(_viewProj);
                        _collectablesSpritebatch->start();
                    }
                    ++collectableDrawn;

                    if(gameplay::Game::getInstance()->getState() != gameplay::Game::State::PAUSED)
                    {
                        float const speed = 5.0f;
                        float const height = collectable->_node->getScaleY() * 0.05f;
                        float bounce = sin((gameplay::Game::getGameTime() / 1000.0f) * speed + (collectable->_node->getTranslationX() + collectable->_node->getTranslationY())) * height;
                        dst.y += bounce;
                    }
                    _collectablesSpritebatch->draw(getRenderDestination(dst), getSafeDrawRect(collectable->_src));
                }
            }
        }

        DEBUG_RENDER_TEXT_WITH_ARGS("show_level_stats", "collectables  [%d/%d]", collectableDrawn, _collectables.size());

        if(collectableDrawn > 0)
        {
            _collectablesSpritebatch->finish();
        }
    }

    void LevelRendererComponent::renderCharacters()
    {
        _characterRenderer->start();

        // Enemies
        for (auto & enemyAnimPairItr : _enemyAnimationBatches)
        {
            EnemyComponent * enemy = enemyAnimPairItr.first;
            float const alpha = enemy->getAlpha();

            if(alpha > 0.0f)
            {
                std::map<int, gameplay::SpriteBatch *> & enemyBatches = enemyAnimPairItr.second;
                bool const wasRenderered = _characterRenderer->render(enemy->getCurrentAnimation(),
                                enemyBatches[enemy->getState()], _viewProj,
                                enemy->getFlipFlags(),
                                enemy->getRenderPosition(), _viewport, alpha);
                if(wasRenderered)
                {
                    DEBUG_RENDER_WORLD_TEXT(enemy->getRenderPosition(), "show_enemy_stats",
                                          GAME_VEC3_STR "\n" GAME_VEC3_STR,
                                          GAME_VEC3_ARG(enemy->getRenderPosition()),
                                          GAME_VEC3_ARG(enemy->getVelocity()));
                }
            }
        }

        DEBUG_RENDER_TEXT_WITH_ARGS("show_level_stats", "enemies       [%d/%d]", _characterRenderer->getRenderCount(), _enemyAnimationBatches.size());

        // Player
        _characterRenderer->render(_player->getCurrentAnimation(),
                        _playerAnimationBatches[_player->getState()], _viewProj,
                        _player->getFlipFlags(),
                        _player->getRenderPosition(), _viewport);
        _characterRenderer->finish();
        DEBUG_RENDER_WORLD_TEXT(_player->getRenderPosition(), "show_player_stats",
            GAME_VEC3_STR "\n" GAME_VEC3_STR,
            GAME_VEC3_ARG(_player->getRenderPosition()),
            GAME_VEC3_ARG(_player->getCharacter()->getCurrentVelocity()));
    }

    void LevelRendererComponent::renderWater(float elapsedTime)
    {
        if (!_waterBounds.empty())
        {
            if(gameplay::Game::getInstance()->getState() == gameplay::Game::State::RUNNING)
            {
                float const dt = elapsedTime / 1000.0f;
                _waterUniformTimer += dt;
            }

            int waterBoundsDrawn = 0;

            for (gameplay::Rectangle const & dst : _waterBounds)
            {
                if(dst.intersects((_viewport)))
                {
                    if(waterBoundsDrawn == 0)
                    {
                        _waterSpritebatch->setProjectionMatrix(_viewProj);
                        _waterSpritebatch->start();
                        ++waterBoundsDrawn;
                    }

                    gameplay::Rectangle src;
                    src.width = _waterSpritebatch->getSampler()->getTexture()->getWidth();
                    src.width *= (1.0f / src.width) * (dst.width / GAME_UNIT_SCALAR);
                    src.height = _waterSpritebatch->getSampler()->getTexture()->getHeight();
                    _waterSpritebatch->draw(getRenderDestination(dst), getSafeDrawRect(src));
                }
            }

            if(waterBoundsDrawn > 0)
            {
                _waterSpritebatch->finish();
            }
            else
            {
                _waterUniformTimer = 0;
            }
            DEBUG_RENDER_TEXT_WITH_ARGS("show_level_stats", "water         [%d/%d]", waterBoundsDrawn, _waterBounds.size());
        }
    }

    float LevelRendererComponent::getWaterTimeUniform() const
    {
        return _waterUniformTimer * MATH_PIX2;
    }

    void LevelRendererComponent::render(float elapsedTime)
    {
        bool renderingEnabled = _levelLoaded;
#ifndef _FINAL
        renderingEnabled &= getConfig()->getBool("show_level");
#endif
        if(renderingEnabled)
        {
            bool const isPaused = gameplay::Game::getInstance()->getState() == gameplay::Game::State::PAUSED;
            gameplay::FrameBuffer * previousFrameBuffer = nullptr;

            if(isPaused)
            {
                previousFrameBuffer = _frameBuffer->bind();
            }

            _viewProj = CameraComponent::getRenderViewProjectionMatrix();
            _viewport = CameraComponent::getRenderViewport();
            renderBackground(elapsedTime);
            renderTiles();
            renderCollectables();
            renderCharacters();
            renderInteractables();
            renderWater(elapsedTime);
            _foregroundTileBatch->finish();

            if(previousFrameBuffer)
            {
                previousFrameBuffer->bind();
                _pauseSpriteBatch->start();
                _pauseSpriteBatch->draw(gameplay::Game::getInstance()->getViewport(), gameplay::Game::getInstance()->getViewport());
                _pauseSpriteBatch->finish();
            }
        }
    }
}
