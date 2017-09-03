#ifndef KINEMATIC_COMPONENT_H
#define KINEMATIC_COMPONENT_H

#include "Component.h"

namespace game
{
    /**
     * @script{ignore}
    */
    class LevelPlatformsComponent : public gameobjects::Component
    {
    public:
        LevelPlatformsComponent();
        void add(gameplay::Node * node, std::vector<gameplay::Vector2> const & points);
        gameplay::Vector3 getRenderPosition(gameplay::Node * node) const;
    protected:
        virtual void finalize() override;
        virtual bool onMessageReceived(gameobjects::Message * message, int messageType) override;
    private:
        void onPostSimulationUpdate(float elapsedTime);
        
        std::map<gameplay::Node*, gameplay::Curve *> _animations;
        float _timer;
        float _timerDirection;
    };
}

#endif
