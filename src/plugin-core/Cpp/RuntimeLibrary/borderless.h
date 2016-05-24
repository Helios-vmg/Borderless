#ifndef BORDERLESS_H
#define BORDERLESS_H

#include "borderless_runtime.h"

extern "C" void borderless_CppInterpreter_get_state(void **, int *);
extern "C" void borderless_CppInterpreter_return_result(int);

Image entry_point(B::Application &app, Image img);

extern "C" void __borderless_main(){
	void *state;
	int img;
	borderless_CppInterpreter_get_state(&state, &img);
	B::g_application.reset(new B::Application(state));
	int ret = entry_point(state, Image(img));
	borderless_CppInterpreter_return_result(ret);	
}
#endif
