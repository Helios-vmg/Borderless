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
			break;
		case DLL_PROCESS_DETACH:
			llvm::llvm_shutdown();
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
	try{
		auto r = interpreter->execute_path(filename);
		if (result)
			*result = r;
	}catch (std::exception &e){
		result->impl = new CallResultImpl((std::string)"Exception thrown: " + e.what());
		result->error_message = result->impl->message.c_str();
		result->success = false;
	}
}

CPP_PLUGIN_EXPORT_C void CppInterpreter_reset_imag(CppInterpreter *interpreter, external_state image){
	interpreter->reset_image(image);
}

CPP_PLUGIN_EXPORT_C void delete_CppCallResult(CallResult *result){
	delete result->impl;
}
