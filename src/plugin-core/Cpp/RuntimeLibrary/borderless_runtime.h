#ifndef BORDERLESS_RUNTIME_H
#define BORDERLESS_RUNTIME_H

#include "capi.h"
#include <memory>
#include <unordered_map>

namespace B{
	typedef ::Image *handle_t;
	typedef ::PluginCoreState *state_t;
	class Image;
	
	class Application{
		friend class B::Image;
		state_t state;
		std::unordered_map<uintptr_t, std::shared_ptr<handle_t>> handles;
		
		state_t get_state() const{
			return this->state;
		}
	public:
		Application(state_t state): state(state){}
		B::Image get_displayed_image();
		void display_in_current_window(const Image &);
		void debug_print(const std::string &);
		void show_message_box(const std::string &);
	};
	
	std::unique_ptr<Application> g_application;
	
	class Image{
		friend class Application;
		std::shared_ptr<handle_t> handle;
		bool dims_initialized = false;
		int w = -1,
			h = -1;
		bool props_initialized = false;
		int stride = -1,
			pitch = -1;
		unsigned char *pixels = nullptr;
		
		Image(const std::shared_ptr<handle_t> &handle): handle(handle){}
	public:
		Image(){}
		Image(handle_t handle): handle(new handle_t(handle)){}
		Image(const char *path);
		Image(const std::string &path): Image(path.c_str()){}
		Image(int w, int h);
		~Image();
		Image clone() const;
		Image clone_without_data() const;
		operator bool() const{
			return !!this->handle;
		}
		void get_dimensions(int &w, int &h);
		void get_pixel_data(int &w, int &h, int &stride, int &pitch, unsigned char *&pixels);
		bool save(const char *path);
		bool save(const std::string &path){
			return this->save(path.c_str());
		}
		handle_t get_handle() const{
			return this->handle ? *this->handle : nullptr;
		}
	};
	
	/*
	class Stream{
	public:
	};
	
	class DebugStream : public Stream{
	};
	
	class MsgboxStream : public Stream{
	};
	
	DebugStream DEBUG;
	MsgboxStream MSGBOX;
	*/
	
	#include "borderless_runtime.cpp"
}

#endif
