#ifndef LUAOP_H_
#define LUAOP_H_

extern "C" 
{
#include "lua.h"
#include "luaconf.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include <string>

class CLuaOp
{
public:
        static std::string lua_getStringItem(lua_State* L, const char* name);
	static unsigned long lua_getIntItem(lua_State* L, const char* name);
};

#endif //LUAOP_H_
