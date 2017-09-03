#ifndef GAMEOBJECT_COMPONENT_H
#define GAMEOBJECT_COMPONENT_H

#include "GameObjectCommon.h"
#include <typeindex>

namespace gameobjects
{
    class GameObject;

    /**
     * A component enapsulates some aspect of gameplay logic. A component can only be created as part of a GameObject
     * file (*.go). A component declaration within a GameObject may define serializable properties.
     *
     * The re-usability of the component is up to the implementation of the derrived class. A component may be completley
     * de-coupled from other components by using the message passing system. Alternatively, a component may depend on other
     * components at various level of the GameObject hierarchy. Ideally, any inter-dependency between components should be
     * kept to a minimum and should avoid being bi-directional.
     *
     * @script{ignore}
    */
    class Component : public gameplay::Ref
    {
        friend class GameObject;
        friend class GameObjectController;

    public:
        explicit Component();
        virtual ~Component() {}

        GameObject * getParent();

        GameObject * getRootParent();

        /**
         * Returns an id that uniquely identifies this component from its sibling components.
         */
        std::string const & getId() const;
    protected:

        /**
         * Read messages that were broadcast to the parent GameObject here. Return false to indicate
         * message has been handled and no other gameobjects should be notified
         */
        virtual bool onMessageReceived(Message * message, int messageType) { return true; }

        /**
         * Read properties defined in the component declaration here.
         *
         * This will be invoked before initialize
         */
        virtual void readProperties(gameplay::Properties & properties) {}

        /**
         * Run any initialization here that depends on serialized properties but not on
         * other sibling components.
         *
         * This will be invoked after readProperties
         */
        virtual void initialize() {}

        /**
         * Run any initialization here that depends on sibling components/GameObject ancestors.
         *
         * This will be invoked after readProperties and initialize
         */
        virtual void onStart() {}

        /**
         * Release resources and shared references here.
         */
        virtual void finalize() {}
    private:
        Component(Component const &);

        std::string _id;
        std::type_index _typeId;
        GameObject * _parent;
        bool _waitForNextFrameBeforeDeletion;
    };
}

#endif
