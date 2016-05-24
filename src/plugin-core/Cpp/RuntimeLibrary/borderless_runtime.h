#ifndef BORDERLESS_RUNTIME_H
#define BORDERLESS_RUNTIME_H

#include "capi.h"
#include <memory>

namespace B{
	class Image;
	
	class Application{
		friend extern "C" void __borderless_main();
		void *state;
		Application(void *state): state(state){}
	public:
		std::shared_ptr<Image> get_displayed_image();
		void display_in_current_window(const Image &);
		void debug_print(const std::string &);
		void show_message_box(const std::string &);
	};
	
	std::unique_ptr<Application> g_application;
	
	class Image{
		friend extern "C" void __borderless_main();
		std::shared_ptr<int> handle;
		bool props_initialized = false;
		int w = -1;
		int h = -1;
		int stride = -1;
		int pitch = -1;
		unsigned char *pixels = nullptr;
		Image(int handle): handle(new int(handle)){}
	public:
		Image(int w, int h);
		~Image();
		Image clone() const;
		Image clone_without_data() const;
	};
	
	class Stream{
	public:
	};
	
	class DebugStream : public Stream{
	};
	
	class MsgboxStream : public Stream{
	};
	
	DebugStream DEBUG;
	MsgboxStream MSGBOX;
}

#endif
