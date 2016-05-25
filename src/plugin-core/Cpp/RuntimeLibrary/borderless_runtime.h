#ifndef BORDERLESS_RUNTIME_H
#define BORDERLESS_RUNTIME_H

#include "capi.h"
//#include <memory>
#include <string>
#include <unordered_map>

namespace B{
	typedef ::Image *handle_t;
	typedef ::PluginCoreState *state_t;
	class Image;
	
	class shared_ptr{
		handle_t p;
		unsigned *refcount;
		void clear(){
			if (!*this)
				return;
			if (this->unique())
				delete this->refcount;
			else
				--*this->refcount;
			this->refcount = nullptr;
			this->p = nullptr;
		}
	public:
		shared_ptr(): p(nullptr), refcount(nullptr){}
		explicit shared_ptr(handle_t p): p(p), refcount(new unsigned(1)){}
		shared_ptr(const shared_ptr &o): p(o.p), refcount(o.refcount){
			++*this->refcount;
		}
		~shared_ptr(){
			this->clear();
		}
		operator bool() const{
			return !!this->p;
		}
		bool unique() const{
			return !!this->refcount && *this->refcount == 1;
		}
		handle_t get() const{
			return this->p;
		}
		void reset(handle_t p = nullptr){
			this->clear();
			if (!p)
				return;
			this->p = p;
			this->refcount = new unsigned(1);
		}
		void ref(){
			if (*this)
				++*this->refcount;
		}
	};
	
	class Application{
		friend class B::Image;
		state_t state;
		std::unordered_map<uintptr_t, shared_ptr> handles;
		
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
	
	Application *g_application;
	
	class Image{
		friend class Application;
		shared_ptr handle;
		bool dims_initialized = false;
		int w = -1,
			h = -1;
		bool props_initialized = false;
		int stride = -1,
			pitch = -1;
		unsigned char *pixels = nullptr;
		
		Image(const decltype(Image::handle) &handle): handle(handle){}
	public:
		Image(){}
		Image(handle_t handle): handle(handle){}
		Image(const char *path);
		Image(const std::string &path): Image(path.c_str()){}
		Image(int w, int h);
		~Image();
		Image clone() const;
		Image clone_without_data() const;
		void ref(){
			this->handle.ref();
		}
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
			return this->handle ? this->handle.get() : nullptr;
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
