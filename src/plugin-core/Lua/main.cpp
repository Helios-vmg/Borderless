/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "main.h"
#include "LuaInterpreter.h"

#ifdef WIN32
#include <Windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved){
	switch (ul_reason_for_call){
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}
#endif

LUA_PLUGIN_EXPORT_C LuaInterpreter *new_LuaInterpreter(LuaInterpreterParameters *parameters){
	return new LuaInterpreter(*parameters);
}

LUA_PLUGIN_EXPORT_C void delete_LuaInterpreter(LuaInterpreter *interpreter){
	delete interpreter;
}

LUA_PLUGIN_EXPORT_C void LuaInterpreter_execute(LuaCallResult *result, LuaInterpreter *interpreter, const char *filename, const void *buffer, size_t size){
	auto r = interpreter->execute_buffer(filename, buffer, size);
	if (result)
		*result = r;
}

LUA_PLUGIN_EXPORT_C void delete_LuaCallResult(LuaCallResult *result){
	delete result->impl;
}
