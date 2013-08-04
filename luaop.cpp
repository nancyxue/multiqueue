#include "luaop.h"

std::string CLuaOp::lua_getStringItem(lua_State* L, const char* name)
{
        std::string ret;

	lua_pushstring(L, name);
	lua_gettable(L, -2);
	ret = lua_tostring(L, -1);
	lua_pop(L, 1);
	
	return ret;
}

unsigned long CLuaOp::lua_getIntItem(lua_State* L, const char* name)
{
	unsigned long ret;

	lua_pushstring(L, name);
	lua_gettable(L, -2);
	ret = (unsigned long)lua_tonumber(L, -1);
	lua_pop(L, 1);
	
	return ret;
}
