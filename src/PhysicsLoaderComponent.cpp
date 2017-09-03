#include "PhysicsLoaderComponent.h"

#include "GameObject.h"
#include "Common.h"
#include "PropertiesRef.h"
#include "ResourceManager.h"

namespace game
{
    PhysicsLoaderComponent::PhysicsLoaderComponent()
        : _node(nullptr)
        , _createPhysicsOnInitialize(true)
    {
    }

    PhysicsLoaderComponent::~PhysicsLoaderComponent()
    {
    }

    void PhysicsLoaderComponent::initialize()
    {
        if (_createPhysicsOnInitialize)
        {
            createPhysics();
        }
    }

    void PhysicsLoaderComponent::finalize()
    {
        getParent()->getNode()->removeChild(_node);
        SAFE_RELEASE(_node);
    }

    void PhysicsLoaderComponent::createPhysics()
    {
        if (!_node)
        {
            gameplay::PropertiesRef * physicsPropertiesRef = ResourceManager::getInstance().getProperties(_physics.c_str());
            gameplay::Properties * physicsProperties = physicsPropertiesRef->get();
            _node = gameplay::Node::create(_physics.c_str());
            _node->setCollisionObject(physicsProperties);

            if (physicsProperties->exists("extents"))
            {
                gameplay::Vector3 scale;
                physicsProperties->getVector3("extents", &scale);
                _node->setScale(scale);
            }
            else if (physicsProperties->exists("radius"))
            {
                _node->setScale(gameplay::Vector3(physicsProperties->getFloat("radius") * 2, physicsProperties->getFloat("height"), 0.0f));
            }

            getParent()->getNode()->addChild(_node);
            SAFE_RELEASE(physicsPropertiesRef);
        }
    }

    void PhysicsLoaderComponent::readProperties(gameplay::Properties & properties)
    {
        _physics = properties.getString("physics");
        gameobjects::setIfExists(properties, "create_physics_on_init", _createPhysicsOnInitialize);
    }

    gameplay::Node * PhysicsLoaderComponent::getNode() const
    {
        return _node;
    }

    std::string const & PhysicsLoaderComponent::getPhysicsPath() const
    {
        return _physics;
    }
}
