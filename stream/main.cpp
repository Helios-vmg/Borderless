#include <string>
#include <cstdio>
#include <iostream>
#include <type_traits>

typedef unsigned char u8;

struct u8_quad{
	u8 data[4];
};

enum class StreamCommand{
	Endl,
	Hex,
};

class Stream{
	std::string buffer;
	bool hex = false;
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
	template <typename T>
	typename std::enable_if<std::is_integral<T>::value && std::is_unsigned<T>::value, Stream &>::type
	operator<<(T x){
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
		return *this;
	}
	template <typename T>
	typename std::enable_if<std::is_integral<T>::value && !std::is_unsigned<T>::value, Stream &>::type
	operator<<(T x){
		if (x < 0){
			this->buffer += '-';
			x = -x;
		}
		return *this << (typename std::make_unsigned<T>::type)x;
	}
	template <typename T>
	typename std::enable_if<std::is_floating_point<T>::value, Stream &>::type
	operator<<(const T &x){
		char temp[100];
		double y = (double)x;
		snprintf(temp, 100, "%f", y);
		this->buffer += temp;
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
			default:
				break;
		}
		return *this;
	}
};

StreamCommand hex = StreamCommand::Hex;
StreamCommand endl = StreamCommand::Endl;

int main(){
	Stream stream;
	u8_quad q = {0, 12, 0, 0};
	stream << "Hello, World!\n" << '!' << endl << q << endl;
	std::cout << stream.str();
	return 0;
}
