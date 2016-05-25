#ifndef BORDERLESS_H
#define BORDERLESS_H

#include "borderless_runtime.h"

extern "C" void borderless_CppInterpreter_get_state(B::state_t *, B::handle_t *);
extern "C" void borderless_CppInterpreter_return_result(void *);

B::Image entry_point(B::Application &app, B::Image img);

extern "C" void __borderless_main(){
	B::state_t state;
	B::handle_t img;
	borderless_CppInterpreter_get_state(&state, &img);
	B::g_application.reset(new B::Application(state));
	auto ret = entry_point(*B::g_application, B::Image(img));
	borderless_CppInterpreter_return_result(ret.get_handle());
}
#endif
