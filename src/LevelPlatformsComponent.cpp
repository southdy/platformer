#include "LevelPlatformsComponent.h"

#include "Game.h"
#include "GameObject.h"
#include "PlayerComponent.h"
#include "Messages.h"

#include "Common.h"

namespace game
{
    LevelPlatformsComponent::LevelPlatformsComponent()
        : _timer(0.0f)
        , _timerDirection(1.0f)
    {
    }

    void LevelPlatformsComponent::finalize()
    {
        for (auto & pair : _animations)
        {
            gameplay::Node * node = pair.first;
            gameplay::Curve * curve = pair.second;
            SAFE_RELEASE(node);
            SAFE_RELEASE(curve);
        }
    }

    bool LevelPlatformsComponent::onMessageReceived(gameobjects::Message * message, int messageType)
    {
        if(messageType == Messages::Type::PostSimulationUpdate)
        {
            onPostSimulationUpdate(PostSimulationUpdateMessage(message)._elapsedTime);
        }
        return true;
    }

    void LevelPlatformsComponent::add(gameplay::Node * node, std::vector<gameplay::Vector2> const & points)
    {
        node->addRef();
        gameplay::Curve * curve = gameplay::Curve::create(points.size(), 2);
        float controlPoints[2];

        for (int i = 0; i < points.size(); ++i)
        {
            controlPoints[0] = points[i].x;
            controlPoints[1] = points[i].y;
            curve->setPoint(i, (1.0f / (points.size() - 1)) * i, controlPoints, gameplay::Curve::FLAT);
        }

        _animations[node] = curve;
    }

    gameplay::Vector3 LevelPlatformsComponent::getRenderPosition(gameplay::Node * node) const
    {
        gameplay::Vector3 result;
        auto itr = _animations.find(node);
        if(itr != _animations.end())
        {
            result = node->getParent()->getTranslation();
        }
        return result;
    }

    void LevelPlatformsComponent::onPostSimulationUpdate(float elapsedTime)
    {
        // TODO: Travel duration can be defined per platform
        float const elapsedTimeMs = elapsedTime / 1000;
        _timer += (elapsedTimeMs * 0.25f) * _timerDirection;

        if(_timer > 1.0f)
        {
            _timerDirection = -1;
        }
        else if(_timer < 0.0f)
        {
            _timerDirection = 1;
        }

        _timer = MATH_CLAMP(_timer, 0.0f, 1.0f);

        for (auto & pair : _animations)
        {
            gameplay::Node * node = pair.first;
            float kinematicPos[2];
            pair.second->evaluate(_timer, kinematicPos);
            gameplay::Vector3 position(kinematicPos[0], kinematicPos[1], 0);
            node->getParent()->setTranslation(position);
        }
    }
}
