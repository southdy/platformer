#ifndef GAME_MESSAGES_H
#define GAME_MESSAGES_H

#include "GameObjectCommon.h"

namespace game
{
    GAMEOBJECTS_MESSAGE_TYPES_BEGIN()
        GAMEOBJECTS_MESSAGE_TYPE(EnemyKilled)
        GAMEOBJECTS_MESSAGE_TYPE(Exit)
        GAMEOBJECTS_MESSAGE_TYPE(Key)
        GAMEOBJECTS_MESSAGE_TYPE(Mouse)
        GAMEOBJECTS_MESSAGE_TYPE(Touch)
        GAMEOBJECTS_MESSAGE_TYPE(Pinch)
        GAMEOBJECTS_MESSAGE_TYPE(PlayerJump)
        GAMEOBJECTS_MESSAGE_TYPE(PlayerReset)
        GAMEOBJECTS_MESSAGE_TYPE(LevelLoaded)
        GAMEOBJECTS_MESSAGE_TYPE(PreLevelUnloaded)
        GAMEOBJECTS_MESSAGE_TYPE(LevelUnloaded)
        GAMEOBJECTS_MESSAGE_TYPE(QueueLevelLoad)
        GAMEOBJECTS_MESSAGE_TYPE(Render)
        GAMEOBJECTS_MESSAGE_TYPE(PreSimulationUpdate)
        GAMEOBJECTS_MESSAGE_TYPE(SimulationUpdate)
        GAMEOBJECTS_MESSAGE_TYPE(PostSimulationUpdate)
        GAMEOBJECTS_MESSAGE_TYPE(ScreenFadeStateChanged)
    GAMEOBJECTS_MESSAGE_TYPES_END()

    GAMEOBJECTS_MESSAGE_0(Exit)
    GAMEOBJECTS_MESSAGE_0(EnemyKilled)
    GAMEOBJECTS_MESSAGE_0(LevelLoaded)
    GAMEOBJECTS_MESSAGE_0(LevelUnloaded)
    GAMEOBJECTS_MESSAGE_0(PreLevelUnloaded)
    GAMEOBJECTS_MESSAGE_0(PlayerJump)
    GAMEOBJECTS_MESSAGE_0(PlayerReset)
    GAMEOBJECTS_MESSAGE_1(QueueLevelLoad, char const *, fileName)
    GAMEOBJECTS_MESSAGE_1(ScreenFadeStateChanged, bool, isActive)
    GAMEOBJECTS_MESSAGE_1(Render, float, elapsedTime)
    GAMEOBJECTS_MESSAGE_1(PreSimulationUpdate, float, elapsedTime)
    GAMEOBJECTS_MESSAGE_1(SimulationUpdate, float, elapsedTime)
    GAMEOBJECTS_MESSAGE_1(PostSimulationUpdate, float, elapsedTime)
    GAMEOBJECTS_MESSAGE_2(Key, int, event, int, key)
    GAMEOBJECTS_MESSAGE_3(Pinch, int, x, int, y, float, scale)
    GAMEOBJECTS_MESSAGE_4(Touch, int, event, int, x, int, y, int, contactIndex)
    GAMEOBJECTS_MESSAGE_4(Mouse, int, event, int, x, int, y, int, wheelDelta)
}

#endif
