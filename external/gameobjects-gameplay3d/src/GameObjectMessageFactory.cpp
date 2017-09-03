#include "GameObjectMessageFactory.h"

#include "GameObjectController.h"

namespace gameobjects
{
    void broadcastInternal(Message * message)
    {
        gameobjects::GameObjectController::getInstance().broadcastMessage(message);
    }
}
