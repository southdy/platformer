#ifndef GAME_LEVEL_COMPONENT_H
#define GAME_LEVEL_COMPONENT_H

#include "Component.h"
#include "LevelCollision.h"

namespace gameplay
{
    class Properties;
}

namespace game
{
    /**
     * Loads a level from a .level file
     *
     * Level files are created in [Tiled], exported as JSON and then converted to the
     * gameplay property format using [Json2gp3d]
     *
     * [Tiled]      Download @ http://www.mapeditor.org/download.html
     * [Json2gp3d]  Download @ https://github.com/louis-mclaughlin/json-to-gameplay3d
     *
     * @script{ignore}
    */
    class LevelLoaderComponent : public gameobjects::Component
    {
    public:
        static int const EMPTY_TILE = 0;

        struct Collectable
        {
            gameplay::Rectangle _src;
            gameplay::Node * _node;
            gameplay::Vector3 _startPosition;
            bool _active;
            bool _visible;
        };

        explicit LevelLoaderComponent();
        ~LevelLoaderComponent();

        void reload();

        std::string const & getTexturePath() const;
        int getTileWidth() const;
        int getTileHeight() const;
        int getTile(int x, int y) const;
        int getWidth() const;
        int getHeight() const;
        gameplay::Vector3 const & getPlayerSpawnPosition() const;
        Collectable * findCollectable(gameplay::Node * node);
        void getCollectables(std::vector<Collectable*> & collectablesOut);
        void forEachCachedNode(collision::Type::Enum terrainType, std::function<void(gameplay::Node *)> func);
    protected:
        virtual void initialize() override;
        virtual void finalize() override;
        virtual bool onMessageReceived(gameobjects::Message * message, int messageType) override;
        virtual void readProperties(gameplay::Properties & properties) override;
    private:
        struct Tile
        {
            Tile() : _tileId(0) {}
            int _tileId;
        };

        LevelLoaderComponent(LevelLoaderComponent const &);

        void load();
        void loadTerrain(gameplay::Properties * layerNamespace);
        void loadCharacters(gameplay::Properties * layerNamespace);
        void loadCharacterBounds(gameplay::Properties * layerNamespace);
        void loadStaticCollision(gameplay::Properties * layerNamespace, collision::Type::Enum terrainType);
        void loadDynamicCollision(gameplay::Properties * layerNamespace);
        void loadKinematicCollision(gameplay::Properties * layerNamespace);
        void loadCollectables(gameplay::Properties * layerNamespace, float scale);
        void loadBridges(gameplay::Properties * layerNamespace);
        void unload();

        gameplay::Rectangle getObjectBounds(gameplay::Properties * objectNamespace) const;
        gameplay::Node * createCollisionObject(collision::Type::Enum collisionType, gameplay::Properties * collisionProperties, gameplay::Rectangle const & bounds, float rotationZ = 0.0f);
        void placeEnemies();
        void processLoadRequests();

        std::string _level;
        std::string _texturePath;
        int _width;
        int _height;
        int _tileWidth;
        int _tileHeight;
        bool _loadBroadcasted;
        gameplay::Vector3 _playerSpawnPosition;
        std::vector<std::vector<Tile>> _grid;
        gameobjects::Message * _loadedMessage;
        gameobjects::Message * _unloadedMessage;
        gameobjects::Message * _preUnloadedMessage;
        std::vector<gameobjects::GameObject*> _children;
        std::vector<gameplay::Rectangle> _characterBounds;
        std::map <collision::Type::Enum, std::vector<gameplay::Node*>> _collisionNodes;
        std::map<gameplay::Node *, Collectable> _collectables;
    };
}

#endif
