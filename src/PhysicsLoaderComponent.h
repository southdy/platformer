#ifndef GAME_COLLISION_OBJECT_COMPONENT_H
#define GAME_COLLISION_OBJECT_COMPONENT_H

#include "Component.h"

namespace gameplay
{
    class Properties;
}

namespace game
{
    /**
     * Creates a collision object from a .physics file and creates a node for it.
     * The created node is added as a child node to the parent GameObject node.
     *
     * @script{ignore}
    */
    class PhysicsLoaderComponent : public gameobjects::Component
    {
    public:
        explicit PhysicsLoaderComponent();
        ~PhysicsLoaderComponent();

        void createPhysics();
        std::string const & getPhysicsPath() const;
        gameplay::Node * getNode() const;
    protected:
        virtual void initialize() override;
        virtual void finalize() override;
        virtual void readProperties(gameplay::Properties & properties) override;
    private:
        PhysicsLoaderComponent(PhysicsLoaderComponent const &);

        bool _createPhysicsOnInitialize;
        std::string _physics;
        gameplay::Node * _node;
    };
}

#endif
