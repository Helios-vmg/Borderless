#ifndef BORDERLESS_RUNTIME_H
#define BORDERLESS_RUNTIME_H

#include "capi.h"
//#include <memory>
#include <string>
#include <unordered_map>
#include <cstdint>
#include <array>

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
	
	class ImageIterator{
		Image &image;
		u8 *pixels;
		int w, h, stride, pitch, i, n;
		unsigned state;
	public:
		ImageIterator(Image &);
		// Behaves like iterator++ != end for while and for predicate.
		bool next(u8 *&pi);
		void position(int &x, int &y) const;
		void reset();
	};

	
	enum class StreamCommand{
		Endl,
		Hex,
		DebugPrint,
		MsgBox,
	};
	
	class Stream{
		std::string buffer;
		bool hex = false;
		template <typename T>
		void write_unsigned(T x){
			if (!this->hex){
				if (!x)
					this->buffer += '0';
				else{
					const auto n = sizeof(T) * 8 / 3;
					this->buffer.reserve(this->buffer.size() + n);
					char temp[n];
					unsigned i = 0;
					while (x && i < 100){
						temp[i++] = '0' + x % 10;
						x /= 10;
					}
					while (i--)
						this->buffer += temp[i];
				}
			}else{
				const auto n = sizeof(T) * 8 / 4;
				this->buffer.reserve(this->buffer.size() + n);
				char temp[n];
				unsigned i = 0;
				for (; i < n; i++){
					auto digit = x % 16;
					temp[i] = x < 10 ? '0' + digit : 'a' - 10 + digit;
					x /= 16;
				}
				while (i--)
					this->buffer += temp[i];
				this->hex = false;
			}
		}
		template <typename T>
		void write_signed(T x){
			if (x < 0){
				this->buffer += '-';
				x = -x;
			}
			this->write_unsigned(x);
		}
	public:
		Stream(){}
		Stream(const std::string &s): buffer(s){}
		const std::string &str() const{
			return this->buffer;
		}
		Stream &operator<<(char c){
			this->buffer += c;
			return *this;
		}
		
		Stream &operator<<(unsigned char x){
			this->write_unsigned(x);
			return *this;
		}
		Stream &operator<<(unsigned short x){
			this->write_unsigned(x);
			return *this;
		}
		Stream &operator<<(unsigned int x){
			this->write_unsigned(x);
			return *this;
		}
		Stream &operator<<(unsigned long x){
			this->write_unsigned(x);
			return *this;
		}
		Stream &operator<<(unsigned long long x){
			this->write_unsigned(x);
			return *this;
		}
		
		Stream &operator<<(signed char x){
			this->write_signed(x);
			return *this;
		}
		Stream &operator<<(signed short x){
			this->write_signed(x);
			return *this;
		}
		Stream &operator<<(signed int x){
			this->write_signed(x);
			return *this;
		}
		Stream &operator<<(signed long x){
			this->write_signed(x);
			return *this;
		}
		Stream &operator<<(signed long long x){
			this->write_signed(x);
			return *this;
		}
		
		Stream &operator<<(float x){
			auto temp = double_to_string(x);
			this->buffer += temp;
			release_double_to_string(temp);
			return *this;
		}
		Stream &operator<<(double x){
			auto temp = double_to_string(x);
			this->buffer += temp;
			release_double_to_string(temp);
			return *this;
		}
		
		Stream &operator<<(const char *s){
			this->buffer += s;
			return *this;
		}
		Stream &operator<<(const std::string &s){
			this->buffer += s;
			return *this;
		}
		Stream &operator<<(const u8_quad &q){
			this->buffer.reserve(this->buffer.size() + 17);
			this->buffer += '#';
			for (int i = 0; i < 4; i++){
				this->hex = true;
				*this << (unsigned char)q.data[i];
			}
			return *this;
		}
		Stream &operator<<(const StreamCommand &c){
			switch (c){
				case StreamCommand::Endl: 
					*this << '\n';
					break;
				case StreamCommand::Hex:
					this->hex = true;
					break;
				case StreamCommand::DebugPrint:
					g_application->debug_print(this->buffer);
					this->buffer.clear();
					break;
				case StreamCommand::MsgBox:
					g_application->show_message_box(this->buffer);
					this->buffer.clear();
					break;
				default:
					break;
			}
			return *this;
		}
	};

	StreamCommand hex = StreamCommand::Hex;
	StreamCommand endl = StreamCommand::Endl;
	StreamCommand debugprint = StreamCommand::DebugPrint;
	StreamCommand msgbox = StreamCommand::MsgBox;
	
	typedef std::array<std::uint32_t, 4> xorshift128_state;
	
	class XorShift128{
		xorshift128_state state;
	public:
		XorShift128();
		XorShift128(const xorshift128_state &seed): state(seed){}
		XorShift128(xorshift128_state &&seed): state(std::move(seed)){}
		std::uint32_t operator()();
		void generate_block(void *buffer, size_t size);
		std::uint32_t get32(){
			return (*this)();
		}
		std::uint64_t get64(){
			return (*this)() | ((std::uint64_t)(*this)() << 32);
		}
		double get_float(){
			auto bits = this->get64();
			double ret = 0;
			double addend = 0.5;
			for (int i = 52; i--;){
				ret += addend * (bits % 2);
				bits >>= 1;
				addend *= 0.5;
			}
			return ret;
		}
	};
	
	#include "borderless_runtime.cpp"
}

#endif
