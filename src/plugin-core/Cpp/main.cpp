/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "stdafx.h"
#include "main.h"
#include "CppInterpreter.h"
#include "../CallResultImpl.h"

#ifdef WIN32
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

CPP_PLUGIN_EXPORT_C CppInterpreter *new_CppInterpreter(CppInterpreterParameters *parameters){
	return new CppInterpreter(*parameters);
}

CPP_PLUGIN_EXPORT_C void delete_CppInterpreter(CppInterpreter *interpreter){
	delete interpreter;
}

CPP_PLUGIN_EXPORT_C void CppInterpreter_execute(CallResult *result, CppInterpreter *interpreter, const char *filename){
	auto r = interpreter->execute_buffer(filename);
	if (result)
		*result = r;
}

CPP_PLUGIN_EXPORT_C void delete_CppCallResult(CallResult *result){
	delete result->impl;
}
