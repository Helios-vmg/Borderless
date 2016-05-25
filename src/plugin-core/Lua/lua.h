/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef BORDERLESSLUA_H
#define BORDERLESSLUA_H

#include <lua.hpp>
#include <memory>

class LuaInterpreter;

bool get_lua_global_function(lua_State *state, const char *name);
void handle_call_to_c_error(lua_State *state, const char *function, const char *msg);
std::shared_ptr<lua_State> init_lua_state(LuaInterpreter *core_state);
int lua_panic_function(lua_State *state);

class LuaStackUnwind : public std::exception{
public:
	const char *what() const{
		return "Control flow exception.";
	}
};

#endif
