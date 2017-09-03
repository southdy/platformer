#include "LevelLoaderComponent.h"

#include "base64.h"
#include "Common.h"
#include "PhysicsLoaderComponent.h"
#include "ProfilerController.h"
#include "EnemyComponent.h"
#include "Game.h"
#include "GameObject.h"
#include "GameObjectController.h"
#include "LevelPlatformsComponent.h"
#include "Messages.h"
#include "PropertiesRef.h"
#include "ResourceManager.h"
#include "SpriteSheet.h"
#include "zlib.h"

namespace game
{
    LevelLoaderComponent::LevelLoaderComponent()
        : _loadedMessage(nullptr)
        , _unloadedMessage(nullptr)
        , _preUnloadedMessage(nullptr)
        , _loadBroadcasted(true)
    {
    }

    LevelLoaderComponent::~LevelLoaderComponent()
    {
    }

    bool LevelLoaderComponent::onMessageReceived(gameobjects::Message * message, int messageType)
    {
        switch (messageType)
        {
        case Messages::Exit:
            unload();
            break;
        case Messages::PostSimulationUpdate:
            processLoadRequests();
            break;
        case Messages::Type::ScreenFadeStateChanged:
            {
                ScreenFadeStateChangedMessage fadeActive(message);
                if (!fadeActive._isActive)
                {
                    forEachCachedNode(collision::Type::DYNAMIC, [](gameplay::Node * node)
                    {
                        node->getCollisionObject()->setEnabled(true);
                    });
                }
            }
            break;
            break;
        case Messages::Type::QueueLevelLoad:
            {
                QueueLevelLoadMessage loadMessage(message);
                if(loadMessage._fileName)
                {
                    _level = loadMessage._fileName;
                }
                reload();
                break;
            }
        }
        return true;
    }

    void LevelLoaderComponent::reload()
    {
        _loadBroadcasted = false;
    }

    void LevelLoaderComponent::processLoadRequests()
    {
        if(!_loadBroadcasted)
        {
            unload();
            load();
            _loadBroadcasted = true;
        }
    }

    void LevelLoaderComponent::initialize()
    {
        _loadedMessage = LevelLoadedMessage::create();
        _unloadedMessage = LevelUnloadedMessage::create();
        _preUnloadedMessage = PreLevelUnloadedMessage::create();
    }

    void LevelLoaderComponent::finalize()
    {
        unload();
        gameobjects::Message::destroy(&_loadedMessage);
        gameobjects::Message::destroy(&_unloadedMessage);
        gameobjects::Message::destroy(&_preUnloadedMessage);
    }

    void LevelLoaderComponent::loadTerrain(gameplay::Properties * layerNamespace)
    {
        int zlibErr;
        unsigned const int bufferSize = 4096;
        char buffer[bufferSize];
        std::string decompressedData;
        std::string compressedData = base64_decode(layerNamespace->getString("data"));
        z_stream zStream;
        zStream.zalloc = Z_NULL;
        zStream.zfree = Z_NULL;
        zStream.opaque = Z_NULL;
        zStream.next_in = (Bytef*)compressedData.c_str();
        zStream.avail_in = compressedData.size();

        if ((zlibErr = inflateInit(&zStream)) != Z_OK)
        {
            GAME_ASSERTFAIL("ZLIB inflateInit failed. Error: %d.", zlibErr);
        }
        do
        {
            zStream.next_out = (Bytef*)buffer;
            zStream.avail_out = bufferSize;
            zlibErr = inflate(&zStream, Z_NO_FLUSH);
            switch (zlibErr)
            {
                case Z_NEED_DICT:
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    inflateEnd(&zStream);
                    GAME_ASSERTFAIL("ZLIB inflateInit failed. Error: %d.", zlibErr);
                    break;
            }

            std::string decomBlock(buffer, bufferSize - zStream.avail_out);
            decompressedData += decomBlock;
        } while (zlibErr != Z_STREAM_END);

        inflateEnd(&zStream);
        size_t byteDataSize = decompressedData.size();

        int x = 0;
        int y = 0;
        for (unsigned int i = 0; i < byteDataSize; i += 4)
        {
            unsigned int const tileId = static_cast<unsigned char>(decompressedData[i + 0]) |
                (static_cast<unsigned char>(decompressedData[i + 1]) << 8u) |
                (static_cast<unsigned char>(decompressedData[i + 2]) << 16u) |
                (static_cast<unsigned char>(decompressedData[i + 3]) << 24u);

            _grid[y][x]._tileId = tileId;
            ++x;
            if (x == _width)
            {
                x = 0;
                ++y;
                if (y == _height)
                {
                    x = 0;
                    y = 0;
                }
            }
        }
    }

    void LevelLoaderComponent::loadCharacters(gameplay::Properties * layerNamespace)
    {
        if (gameplay::Properties * objectsNamespace = layerNamespace->getNamespace("objects", true))
        {
            PROFILE();

            while (gameplay::Properties * objectNamespace = objectsNamespace->getNextNamespace())
            {
                char const * gameObjectTypeName = objectNamespace->getString("name");
                bool const isPlayer = strcmp(gameObjectTypeName, "player") == 0;

#ifndef _FINAL
                if (getConfig()->getBool("spawn_enemies") || isPlayer)
#endif
                {
                    gameobjects::GameObject * gameObject = gameobjects::GameObjectController::getInstance().createGameObject(gameObjectTypeName, getParent());
                    gameplay::Rectangle boumds = getObjectBounds(objectNamespace);
                    gameplay::Vector3 spawnPos(boumds.x, boumds.y, 0.0f);

                    if (isPlayer)
                    {
                        _playerSpawnPosition = spawnPos;
                    }

                    std::vector<PhysicsLoaderComponent*> collisionComponents;
                    gameObject->getComponents(collisionComponents);

                    for (PhysicsLoaderComponent * collisionComponent : collisionComponents)
                    {
                        collisionComponent->getNode()->setTranslation(spawnPos.x, spawnPos.y, 0);
                    }

                    _children.push_back(gameObject);
                }
            }

            objectsNamespace->rewind();
        }
    }

    void LevelLoaderComponent::loadCharacterBounds(gameplay::Properties * layerNamespace)
    {
        if(gameplay::Properties * objectsNamespace = layerNamespace->getNamespace("objects", true))
        {
            while (gameplay::Properties * objectNamespace = objectsNamespace->getNextNamespace())
            {
                PROFILE();
                _characterBounds.push_back(getObjectBounds(objectNamespace));
            }
            objectsNamespace->rewind();
        }
    }

    gameplay::Rectangle LevelLoaderComponent::getObjectBounds(gameplay::Properties * objectNamespace) const
    {
        gameplay::Rectangle rect;
        gameplay::Vector4 bounds;
        objectNamespace->getVector4("dst", &bounds);
        rect.width = bounds.z * GAME_UNIT_SCALAR;
        rect.height = bounds.w * GAME_UNIT_SCALAR;
        rect.x = bounds.x * GAME_UNIT_SCALAR;
        rect.y = ((_tileHeight * _height) - bounds.y) * GAME_UNIT_SCALAR;
        return rect;
    }

    gameplay::Node * LevelLoaderComponent::createCollisionObject(collision::Type::Enum collisionType, gameplay::Properties * collisionProperties, gameplay::Rectangle const & bounds, float rotationZ)
    {
        std::string const name = std::string(collisionProperties->getId()) + "_" + toString(_collisionNodes[collisionType].size());
        gameplay::Node * node = gameplay::Node::create(name.c_str());
        collision::NodeData * info = new collision::NodeData();
        info->_type = collisionType;
        node->setUserObject(info);
        node->translate(bounds.x, bounds.y, 0);
        node->rotateZ(rotationZ);
        getParent()->getNode()->addChild(node);
        node->setScale(bounds.width, bounds.height, 1.0f);
        node->setCollisionObject(collisionProperties);
        _collisionNodes[collisionType].push_back(node);
        return node;
    }

    void setProperty(char const * id, gameplay::Vector3 const & vec, gameplay::Properties * properties)
    {
        std::array<char, 255> buffer;
        sprintf(&buffer[0], "%f, %f, %f", vec.x, vec.y, vec.z);
        properties->setString(id, &buffer[0]);
    }

    void getLineCollisionObjectParams(gameplay::Properties * lineVectorNamespace, gameplay::Rectangle & bounds, float & rotationZ, gameplay::Vector2 & direction)
    {
        gameplay::Vector2 const start(bounds.x, bounds.y);
        gameplay::Vector2 localEnd;
        lineVectorNamespace->getVector2("point", &localEnd);
        localEnd *= GAME_UNIT_SCALAR;
        direction = localEnd;
        direction.normalize();
        rotationZ = -acos(direction.dot(gameplay::Vector2::unitX() * (direction.y > 0 ? 1.0f : -1.0f)));
        gameplay::Vector2 end(start.x + localEnd.x, start.y + localEnd.y);
        gameplay::Vector2 tranlsation = start - (start + end) / 2;
        bounds.x -= tranlsation.x;
        bounds.y += tranlsation.y;
        bounds.width = start.distance(end);
    }

    void LevelLoaderComponent::loadStaticCollision(gameplay::Properties * layerNamespace, collision::Type::Enum collisionType)
    {
        if (gameplay::Properties * objectsNamespace = layerNamespace->getNamespace("objects", true))
        {
            std::string collisionId;

            switch (collisionType)
            {
            case collision::Type::STATIC:
                collisionId = "world_collision";
                break;
            case collision::Type::LADDER:
                collisionId = "ladder";
                break;
            case collision::Type::RESET:
                collisionId = "reset";
                break;
            case collision::Type::WATER:
                collisionId = "water";
                break;
            default:
                GAME_ASSERTFAIL("Unhandled CollisionType %d", collisionType);
                break;
            }

            gameplay::PropertiesRef * collisionPropertiesRef = ResourceManager::getInstance().getProperties((std::string("res/physics/level.physics#") + collisionId).c_str());
            gameplay::Properties * collisionProperties = collisionPropertiesRef->get();

            while (gameplay::Properties * objectNamespace = objectsNamespace->getNextNamespace())
            {
                float rotationZ = 0.0f;
                gameplay::Rectangle bounds = getObjectBounds(objectNamespace);

                if (gameplay::Properties * lineVectorNamespace = objectNamespace->getNamespace("polyline", true))
                {
                    gameplay::Vector2 direction;
                    getLineCollisionObjectParams(lineVectorNamespace, bounds, rotationZ, direction);
                    static float const lineHeight = 0.05f;
                    bounds.height = lineHeight;
                }
                else
                {
                    bounds.x += bounds.width / 2;
                    bounds.y -= bounds.height / 2;
                }

                setProperty("extents", gameplay::Vector3(bounds.width, bounds.height, 1), collisionProperties);
                createCollisionObject(collisionType, collisionProperties, bounds, rotationZ);
            }

            objectsNamespace->rewind();
            SAFE_RELEASE(collisionPropertiesRef);
        }
    }

    void LevelLoaderComponent::loadDynamicCollision(gameplay::Properties * layerNamespace)
    {
        if (gameplay::Properties * objectsNamespace = layerNamespace->getNamespace("objects", true))
        {
            while (gameplay::Properties * objectNamespace = objectsNamespace->getNextNamespace())
            {
                bool const isBoulder = objectNamespace->exists("ellipse");
                std::string collisionId = isBoulder ? "boulder" : "crate";

                gameplay::PropertiesRef * collisionPropertiesRef = ResourceManager::getInstance().getProperties((std::string("res/physics/level.physics#") + collisionId).c_str());
                gameplay::Properties * collisionProperties = collisionPropertiesRef->get();
                gameplay::Rectangle bounds = getObjectBounds(objectNamespace);
                std::array<char, 255> dimensionsBuffer;
                std::string dimensionsId = "extents";
                bounds.x += bounds.width / 2;
                bounds.y -= bounds.height / 2;

                if (isBoulder)
                {
                    dimensionsId = "radius";
                    sprintf(&dimensionsBuffer[0], "%f", bounds.height / 2);
                }
                else
                {
                    sprintf(&dimensionsBuffer[0], "%f, %f, 1", bounds.width, bounds.height);
                }

                collisionProperties->setString(dimensionsId.c_str(), &dimensionsBuffer[0]);
                gameplay::Node * node = createCollisionObject(collision::Type::DYNAMIC, collisionProperties, bounds);
                node->getCollisionObject()->setEnabled(false);
                SAFE_RELEASE(collisionPropertiesRef);
            }

            objectsNamespace->rewind();
        }
    }

    void LevelLoaderComponent::loadKinematicCollision(gameplay::Properties * layerNamespace)
    {
        if (gameplay::Properties * objectsNamespace = layerNamespace->getNamespace("objects", true))
        {
            gameplay::Node * node = nullptr;

            while (gameplay::Properties * objectNamespace = objectsNamespace->getNextNamespace())
            {
                gameplay::Properties * pointNamespace = objectNamespace->getNextNamespace();

                if(!pointNamespace)
                {
                    gameplay::PropertiesRef * collisionPropertiesRef = ResourceManager::getInstance().getProperties("res/physics/level.physics#platform");
                    gameplay::Properties * collisionProperties = collisionPropertiesRef->get();
                    gameplay::Rectangle bounds = getObjectBounds(objectNamespace);
                    std::array<char, 255> dimensionsBuffer;
                    std::string dimensionsId = "extents";
                    bounds.x += bounds.width / 2;
                    bounds.y -= bounds.height / 2;
                    sprintf(&dimensionsBuffer[0], "%f, %f, 1", bounds.width, bounds.height);
                    collisionProperties->setString(dimensionsId.c_str(), &dimensionsBuffer[0]);
                    node = createCollisionObject(collision::Type::KINEMATIC, collisionProperties, bounds);
                    gameplay::Node * parent = gameplay::Node::create();
                    parent->setTranslation(node->getTranslation());
                    node->setTranslation(gameplay::Vector3::zero());
                    parent->addChild(node);
                    SAFE_RELEASE(collisionPropertiesRef);
                }
                else
                {
                    gameplay::Rectangle bounds = getObjectBounds(objectNamespace);
                    gameplay::Vector2 const startPos(bounds.x, bounds.y);
                    std::vector<gameplay::Vector2> points;
                    points.push_back(startPos);

                    while(char const * pointName = pointNamespace->getNextProperty())
                    {
                        gameplay::Vector2 point;
                        pointNamespace->getVector2(pointName, &point);
                        point *= GAME_UNIT_SCALAR;
                        point.y *= -1.0f;
                        point += startPos;
                        points.push_back(point);
                    }

                    gameobjects::GameObject * gameObject = nullptr;
                    LevelPlatformsComponent * kinematicComponent = nullptr;
                    for(gameobjects::GameObject * childGameObject : _children)
                    {
                        kinematicComponent = childGameObject->getComponent<LevelPlatformsComponent>();
                        if(kinematicComponent)
                        {
                            gameObject = childGameObject;
                            break;
                        }
                    }

                    if(!gameObject)
                    {
                        gameObject = gameobjects::GameObjectController::getInstance().createGameObject(getParent());
                        kinematicComponent = gameObject->addComponent<LevelPlatformsComponent>("kinematic_platforms");
                        _children.push_back(gameObject);
                    }

                    kinematicComponent->add(node, points);

                    pointNamespace->rewind();
                }
                objectNamespace->rewind();
            }
            objectsNamespace->rewind();
        }
    }

    void LevelLoaderComponent::loadCollectables(gameplay::Properties * layerNamespace, float scale)
    {
        if (gameplay::Properties * objectsNamespace = layerNamespace->getNamespace("objects", true))
        {
            SpriteSheet * spriteSheet = ResourceManager::getInstance().getSpriteSheet("res/spritesheets/collectables.ss");
            gameplay::PropertiesRef * collisionPropertiesRef = ResourceManager::getInstance().getProperties("res/physics/level.physics#collectable");
            gameplay::Properties * collisionProperties = collisionPropertiesRef->get();
            std::vector<Sprite> sprites;

            spriteSheet->forEachSprite([&sprites](Sprite const & sprite)
            {
                sprites.push_back(sprite);
            });

            while (gameplay::Properties * objectNamespace = objectsNamespace->getNextNamespace())
            {
                gameplay::Rectangle const dst = getObjectBounds(objectNamespace);
                gameplay::Vector2 position(dst.x, dst.y);

                if (gameplay::Properties * lineVectorNamespace = objectNamespace->getNamespace("polyline", true))
                {
                    gameplay::Vector2 line;
                    lineVectorNamespace->getVector2("point", &line);
                    line *= GAME_UNIT_SCALAR;
                    line.y *= -1.0f;
                    float lineLength = line.length();
                    gameplay::Vector2 direction = line;
                    direction.normalize();

                    while(true)
                    {
                        Sprite & sprite = sprites[getRandomRange(0, sprites.size() - 1)];
                        float const collectableWidth = sprite._src.width * GAME_UNIT_SCALAR * scale;
                        lineLength -= collectableWidth;

                        if(lineLength > 0)
                        {
                            std::array<char, 255> extentsBuffer;
                            sprintf(&extentsBuffer[0], "%f, %f, 1", collectableWidth, collectableWidth);
                            collisionProperties->setString("extents", &extentsBuffer[0]);
                            gameplay::Rectangle bounds(position.x, position.y, collectableWidth, collectableWidth);
                            gameplay::Node * collectableNode = createCollisionObject(collision::Type::COLLECTABLE, collisionProperties, bounds);
                            collectableNode->addRef();
                            Collectable collectable;
                            collectable._src = sprite._src;
                            collectable._node = collectableNode;
                            collectable._startPosition = collectableNode->getTranslation();
                            collectable._active = true;
                            collectable._visible = false;
                            _collectables[collectableNode] = collectable;
                            float const padding = 1.25f;
                            position += direction * (collectableWidth * padding);
                        }
                        else
                        {
                            break;
                        }
                    }
                }
            }

            objectsNamespace->rewind();
            sprites.clear();
            SAFE_RELEASE(collisionPropertiesRef);
            SAFE_RELEASE(spriteSheet);
        }
    }

    void LevelLoaderComponent::loadBridges(gameplay::Properties * layerNamespace)
    {
        if (gameplay::Properties * objectsNamespace = layerNamespace->getNamespace("objects", true))
        {
            gameplay::PropertiesRef * collisionPropertiesRef = ResourceManager::getInstance().getProperties("res/physics/level.physics#bridge");
            gameplay::Properties * collisionProperties = collisionPropertiesRef->get();

            while (gameplay::Properties * objectNamespace = objectsNamespace->getNextNamespace())
            {
                if (gameplay::Properties * lineVectorNamespace = objectNamespace->getNamespace("polyline", true))
                {
                    // Get the params for a line
                    gameplay::Rectangle bounds = getObjectBounds(objectNamespace);
                    float rotationZ = 0.0f;
                    gameplay::Vector2 bridgeDirection;
                    getLineCollisionObjectParams(lineVectorNamespace, bounds, rotationZ, bridgeDirection);

                    // Recalculate its starting position based on the size and orientation of the bridge segment(s)
                    bounds.x += (bounds.width / 2) * -bridgeDirection.x;
                    bounds.y += (bounds.width / 2) * bridgeDirection.y;
                    int const numSegments = std::ceil(bounds.width / (getTileWidth() * GAME_UNIT_SCALAR));
                    bounds.width = bounds.width / numSegments;
                    bounds.x += (bounds.width / 2) * bridgeDirection.x;
                    bounds.y += (bounds.width / 2) * -bridgeDirection.y;
                    bounds.height = (getTileHeight() * GAME_UNIT_SCALAR) * 0.25f;
                    setProperty("extents", gameplay::Vector3(bounds.width, bounds.height, 0.0f), collisionProperties);

                    // Create collision nodes for them
                    std::vector<gameplay::Node *> segmentNodes;
                    for (int i = 0; i < numSegments; ++i)
                    {
                        segmentNodes.push_back(createCollisionObject(collision::Type::BRIDGE, collisionProperties, bounds, rotationZ));
                        bounds.x += bridgeDirection.x * bounds.width;
                        bounds.y -= bridgeDirection.y * bounds.width;
                    }

                    // Link them to each other and the end pieces with the world
                    for (int segmentIndex = 0; segmentIndex < numSegments; ++segmentIndex)
                    {
                        gameplay::Node * segmentNode = segmentNodes[segmentIndex];
                        gameplay::PhysicsRigidBody * segmentRigidBody = static_cast<gameplay::PhysicsRigidBody*>(segmentNode->getCollisionObject());
                        gameplay::Vector3 const hingeOffset((bounds.width / 2) * (1.0f / segmentNode->getScaleX()) * (bridgeDirection.y >= 0 ? 1.0f : -1.0f), 0.0f, 0.0f);
                        gameplay::PhysicsController * physicsController = gameplay::Game::getInstance()->getPhysicsController();

                        bool const isFirstSegment = segmentIndex == 0;
                        if (isFirstSegment)
                        {
                            physicsController->createHingeConstraint(segmentRigidBody, gameplay::Quaternion(), -hingeOffset);
                        }

                        bool const isEndSegment = segmentIndex == numSegments - 1;
                        if (!isEndSegment)
                        {
                            gameplay::PhysicsRigidBody * nextSegmentRigidBody = static_cast<gameplay::PhysicsRigidBody*>(segmentNodes[segmentIndex + 1]->getCollisionObject());
                            physicsController->createHingeConstraint(segmentRigidBody, gameplay::Quaternion(), hingeOffset, nextSegmentRigidBody, gameplay::Quaternion(), -hingeOffset);
                        }
                        else
                        {
                            physicsController->createHingeConstraint(segmentRigidBody, gameplay::Quaternion(), hingeOffset);
                        }
                    }
                }
            }

            objectsNamespace->rewind();

            SAFE_RELEASE(collisionPropertiesRef);
        }
    }

    void LevelLoaderComponent::load()
    {
        PROFILE();

        getParent()->getNode()->setId(_level.c_str());
        gameplay::PropertiesRef * rootRef = ResourceManager::getInstance().getProperties(_level.c_str());
        gameplay::Properties * root = rootRef->get();
        float collectablesScale = 1.0f;

        if (gameplay::Properties * propertiesNamespace = root->getNamespace("properties", true, false))
        {
            _texturePath = propertiesNamespace->getString("texture");
            if (propertiesNamespace->exists("collectable_scale"))
            {
                collectablesScale = propertiesNamespace->getFloat("collectable_scale");
            }
        }

        _width = root->getInt("width");
        _height = root->getInt("height");
        _tileWidth = root->getInt("tilewidth");
        _tileHeight = root->getInt("tileheight");
        _grid.resize(_height);
        
        for (std::vector<Tile> & horizontalTiles : _grid)
        {
            horizontalTiles.resize(_width);
        }

        std::vector<gameplay::Properties*> characterProperties;

        if (gameplay::Properties * layersNamespace = root->getNamespace("layers", true))
        {
            while (gameplay::Properties * layerNamespace = layersNamespace->getNextNamespace())
            {
                std::string const layerName = layerNamespace->getString("name");

                if (layerName == "terrain" || layerName == "props")
                {
                    loadTerrain(layerNamespace);
                }
                else if (layerName == "characters")
                {
                    characterProperties.push_back(layerNamespace);
                }
                else if (layerName == "character_bounds")
                {
                    loadCharacterBounds(layerNamespace);
                }
                else if (layerName.find("collision") != std::string::npos)
                {
                    collision::Type::Enum collisionType = collision::Type::STATIC;

                    if (layerName == "collision_ladder")
                    {
                        collisionType = collision::Type::LADDER;
                    }
                    else if (layerName == "collision_hand_of_god")
                    {
                        collisionType = collision::Type::RESET;
                    }
                    else if (layerName == "collision_water")
                    {
                        collisionType = collision::Type::WATER;
                    }
                    else if (layerName == "collision_bridge")
                    {
                        collisionType = collision::Type::BRIDGE;
                    }
                    else if (layerName == "collision_kinematic")
                    {
                        collisionType = collision::Type::KINEMATIC;
                    }

                    if (collisionType == collision::Type::BRIDGE)
                    {
                        loadBridges(layerNamespace);
                    }
                    else if(collisionType == collision::Type::KINEMATIC)
                    {
                        loadKinematicCollision(layerNamespace);
                    }
                    else
                    {
                        loadStaticCollision(layerNamespace, collisionType);
                    }
                }
                else if (layerName == "interactive_props")
                {
#ifndef _FINAL
                    if (getConfig()->getBool("spawn_interactables"))
#endif
                        loadDynamicCollision(layerNamespace);
                }
                else if(layerName == "collectables")
                {
#ifndef _FINAL
                    if (getConfig()->getBool("spawn_collectables"))
#endif
                        loadCollectables(layerNamespace, collectablesScale);
                }
            }

            layersNamespace->rewind();
        }

        for (gameplay::Properties * properties : characterProperties)
        {
            loadCharacters(properties);
        }

        placeEnemies();
        root->rewind();
        SAFE_RELEASE(rootRef);
        getRootParent()->broadcastMessage(_loadedMessage);
    }

    void LevelLoaderComponent::placeEnemies()
    {
        for(gameobjects::GameObject * gameObject : _children)
        {
            if(EnemyComponent * enemyComponent = gameObject->getComponent<EnemyComponent>())
            {
                gameplay::Rectangle * nearestCharacterBound = nullptr;
                float nearestDistance = std::numeric_limits<float>::max();
                gameplay::Vector3 const & enemyPosition = enemyComponent->getNode()->getTranslation();

                for (gameplay::Rectangle & characterBound : _characterBounds)
                {
                    gameplay::Vector3 const boundPosition(characterBound.x, characterBound.y, 0);
                    float const distance = boundPosition.distanceSquared(enemyPosition);

                    if (distance < nearestDistance)
                    {
                        nearestCharacterBound = &characterBound;
                        nearestDistance = distance;
                    }
                }

                if(nearestCharacterBound)
                {
                    if (enemyComponent->isSnappedToCollisionY())
                    {
                        enemyComponent->getNode()->setTranslationY(nearestCharacterBound->y + enemyComponent->getNode()->getScaleY() / 2);
                    }

                    enemyComponent->setHorizontalConstraints(nearestCharacterBound->x - (enemyComponent->getNode()->getScaleX() / 2),
                                                    nearestCharacterBound->x + nearestCharacterBound->width + (enemyComponent->getNode()->getScaleX() / 2));
                }
                else
                {
                    GAME_ASSERTFAIL("Unable to place %s", enemyComponent->getId().c_str());
                }
            }
        }
    }

    void LevelLoaderComponent::unload()
    {
        PROFILE();
        getRootParent()->broadcastMessage(_preUnloadedMessage);

        for (auto & listPair : _collisionNodes)
        {
            for (gameplay::Node* node : listPair.second)
            {
                STALL_SCOPE();
                collision::NodeData * info = collision::NodeData::get(node);
                node->setUserObject(nullptr);
                SAFE_RELEASE(info);
                if (listPair.first == collision::Type::KINEMATIC)
                {
                    gameplay::Node * parent = node->getParent();
                    SAFE_RELEASE(parent);
                }
                
                getParent()->getNode()->removeChild(node);
                SAFE_RELEASE(node);
            }
        }

        for(auto & collectablePair : _collectables)
        {
            STALL_SCOPE();
            getParent()->getNode()->removeChild(collectablePair.second._node);
            SAFE_RELEASE(collectablePair.second._node);
        }

        _collectables.clear();
        _collisionNodes.clear();
        _characterBounds.clear();
        _grid.clear();

        for(auto childItr = _children.begin(); childItr != _children.end(); ++childItr)
        {
            STALL_SCOPE();
            gameobjects::GameObjectController::getInstance().destroyGameObject(*childItr);
        }

        _children.clear();

        getRootParent()->broadcastMessage(_unloadedMessage);
    }

    void LevelLoaderComponent::readProperties(gameplay::Properties & properties)
    {
        if(properties.exists("level"))
        {
            _level = properties.getString("level");
        }
    }

    std::string const & LevelLoaderComponent::getTexturePath() const
    {
        return _texturePath;
    }

    int LevelLoaderComponent::getTileWidth() const
    {
        return _tileWidth;
    }

    int LevelLoaderComponent::getTileHeight() const
    {
        return _tileHeight;
    }

    int LevelLoaderComponent::getTile(int x, int y) const
    {
        return _grid[y][x]._tileId;
    }

    int LevelLoaderComponent::getWidth() const
    {
        return _width;
    }

    int LevelLoaderComponent::getHeight() const
    {
        return _height;
    }

    gameplay::Vector3 const & LevelLoaderComponent::getPlayerSpawnPosition() const
    {
        return _playerSpawnPosition;
    }

    void LevelLoaderComponent::forEachCachedNode(collision::Type::Enum collisionType, std::function<void(gameplay::Node *)> func)
    {
        for (gameplay::Node * node : _collisionNodes[collisionType])
        {
            func(node);
        }
    }

    LevelLoaderComponent::Collectable * LevelLoaderComponent::findCollectable(gameplay::Node * node)
    {
        auto itr = _collectables.find(node);
        return itr != _collectables.end() ? &itr->second : nullptr;
    }

    void LevelLoaderComponent::getCollectables(std::vector<Collectable *> & collectablesOut)
    {
        for (auto & collectablePair : _collectables)
        {
            collectablesOut.push_back(&collectablePair.second);
        }
    }
}
