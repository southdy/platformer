// Autogenerated by gameplay-luagen
#include "Base.h"
#include "lua_all_bindings.h"
#include "lua_FileSystem.h"
#include "lua_Game.h"
#include "lua_Properties.h"
#include "lua_ScriptController.h"
#include "lua_Vector2.h"
#include "lua_Vector3.h"
#include "lua_Vector4.h"

namespace gameplay
{

void lua_RegisterAllBindings()
{
    luaRegister_FileSystem();
    luaRegister_Game();
    luaRegister_Properties();
    luaRegister_ScriptController();
    luaRegister_Vector2();
    luaRegister_Vector3();
    luaRegister_Vector4();
}

}

