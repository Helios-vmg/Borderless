/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef COLOR_SPACE_H
#define COLOR_SPACE_H

#include <cstdint>
#include <cmath>

typedef std::uint8_t byte_t;
template <typename T>
struct Cielab;

template <typename T>
inline T saturate(T x, T min, T max){
	if (x < min)
		return min;
	if (x > max)
		return max;
	return x;
}

struct RgbInt{
	byte_t r;
	byte_t g;
	byte_t b;

	explicit RgbInt(byte_t r = 0, byte_t g = 0, byte_t b = 0): r(r), g(g), b(b){}
	explicit RgbInt(std::uint32_t rgb): r(rgb & 0xFF), g((rgb >> 8) & 0xFF), b((rgb >> 16) & 0xFF){}
	bool operator==(const RgbInt &other) const{
		return
			this->r == other.r &&
			this->g == other.g &&
			this->b == other.b;
	}
	bool operator<(const RgbInt &other) const{
		return this->get_int() < other.get_int();
	}
	std::uint32_t get_int() const{
		return this->r | (this->g << 8) | (this->b << 16);
	}
	int distance_sq(const RgbInt &other) const{
		auto x = (int)this->r - (int)other.r;
		auto y = (int)this->g - (int)other.g;
		auto z = (int)this->b - (int)other.b;
		return x * x + y * y + z * z;
	}
	template <typename T>
	explicit operator Cielab<T>() const;
};

template <typename T>
struct Xyz;

template <typename T>
struct Cielab;

#define DEFINE_VECTOR4(name)								\
	typedef T number_type;									\
	T x[3];													\
	explicit name(T x0 = 0, T x1 = 0, T x2 = 0, T x3 = 0){	\
		this->x[0] = x0;									\
		this->x[1] = x1;									\
		this->x[2] = x2;									\
	}														\
	const name &operator+=(const name &other){				\
		this->x[0] += other.x[0];							\
		this->x[1] += other.x[1];							\
		this->x[2] += other.x[2];							\
		return *this;										\
	}														\
	const name &operator-=(const name &other){				\
		this->x[0] -= other.x[0];							\
		this->x[1] -= other.x[1];							\
		this->x[2] -= other.x[2];							\
		return *this;										\
	}														\
	const name &operator*=(T multiplier){					\
		this->x[0] *= multiplier;							\
		this->x[1] *= multiplier;							\
		this->x[2] *= multiplier;							\
		return *this;										\
	}														\
	name operator+(const name &other) const{				\
		auto ret = *this;									\
		ret += other;										\
		return ret;											\
	}														\
	name operator-(const name &other) const{				\
		auto ret = *this;									\
		ret -= other;										\
		return ret;											\
	}														\
	name operator*(T multiplier) const{						\
		auto ret = *this;									\
		ret *= multiplier;									\
		return ret;											\
	}														\
	T norm_sq() const{										\
		return												\
			this->x[0] * this->x[0] +						\
			this->x[1] * this->x[1] +						\
			this->x[2] * this->x[2];						\
	}

#define DEFINE_NAMED_VECTOR_MEMBER(type, name, index) \
	type &name(){ return this->x[index]; } \
	type name() const{ return this->x[index]; }

template <typename T>
struct RgbFloat{
	DEFINE_VECTOR4(RgbFloat<T>)

	explicit RgbFloat(const RgbInt &rgb): RgbFloat(rgb.r, rgb.g, rgb.b){
		*this *= (T)(1.0 / 255.0);
	}
	explicit operator Xyz<T>() const;
	explicit operator Cielab<T>() const;
	explicit operator RgbInt() const;
	DEFINE_NAMED_VECTOR_MEMBER(T, r, 0)
	DEFINE_NAMED_VECTOR_MEMBER(T, g, 1)
	DEFINE_NAMED_VECTOR_MEMBER(T, b, 2)
	double saturation() const{
		auto max = this->r();
		max = std::max(max, this->g());
		max = std::max(max, this->b());
		if (!max)
			return 0;
		auto min = this->r();
		min = std::min(min, this->g());
		min = std::min(min, this->b());
		return (max - min) / max;
	}
	RgbFloat<T> saturate_channels() const{
		return RgbFloat{
			saturate<T>(this->r(), 0, 1),
			saturate<T>(this->g(), 0, 1),
			saturate<T>(this->b(), 0, 1),
		};
	}
	T distance_sq(const RgbFloat &other, T = 0) const{
		return (*this - other).norm_sq();
	}
	T brightness() const{
		return this->r() * (T).2126 + this->g() * (T).7152 + this->b() * (T).0722;
	}
};

template <typename T>
struct Xyz{
	DEFINE_VECTOR4(Xyz<T>)
	explicit operator RgbFloat<T>() const;
	explicit operator Cielab<T>() const;
};

template <typename T>
struct Cielab{
	DEFINE_VECTOR4(Cielab<T>)
	explicit operator RgbFloat<T>() const;
	explicit operator Xyz<T>() const;
	explicit operator RgbInt() const{
		return (RgbInt)(RgbFloat<T>)*this;
	}
	DEFINE_NAMED_VECTOR_MEMBER(T, L, 0)
	DEFINE_NAMED_VECTOR_MEMBER(T, a, 1)
	DEFINE_NAMED_VECTOR_MEMBER(T, b, 2)
	T distance_sq(const Cielab &other, T extra_weight = 1) const{
		auto diff = *this - other;
		diff.L() *= extra_weight;
		return diff.norm_sq();
	}
	Cielab saturate_channels() const{
		return Cielab{
			saturate<T>(this->L(), (T)0.0,      (T)100.0),
			saturate<T>(this->a(), (T)-86.1846, (T)98.2542),
			saturate<T>(this->b(), (T)-107.864, (T)94.4825),
		};
	}
	T brightness() const{
		return this->L() * (T)(1.0 / 100.0);
	}
};

template <typename T>
struct Grayscale{
	T x;
	typedef T number_type;
	explicit Grayscale(): x(0){}
	explicit Grayscale(T x): x(x){}
	explicit Grayscale(T r, T g, T b){
		this->x = r * (T).2126 + g * (T).7152 + b * (T).0722;
	}
	explicit Grayscale(const RgbInt &rgb){
		this->x = ((int)rgb.r * 2126 + (int)rgb.g * 7152 + (int)rgb.b * 722) / (T)(10000.0 * 255.0);
	}
	explicit Grayscale(const RgbFloat<T> &rgb){
		this->x = rgb.r() * (T).2126 + rgb.g() * (T).7152 + rgb.b() * (T).0722;
	}
	const Grayscale<T> &operator+=(const Grayscale<T> &other){
		this->x += other.x;
		return *this;
	}
	const Grayscale<T> &operator-=(const Grayscale<T> &other){
		this->x -= other.x;
		return *this;
	}
	const Grayscale<T> &operator*=(T multiplier){
		this->x *= multiplier;
		return *this;
	}
	Grayscale<T> operator+(const Grayscale<T> &other) const{
		auto ret = *this;
		ret += other;
		return ret;
	}
	Grayscale<T> operator-(const Grayscale<T> &other) const{
		auto ret = *this;
		ret -= other;
		return ret;
	}
	Grayscale<T> operator*(T multiplier) const{
		auto ret = *this;
		ret *= multiplier;
		return ret;
	}
	T norm_sq() const{
		return this->x * this->x;
	}
	explicit operator RgbFloat<T>() const{
		return RgbFloat<T>(x, x, x);
	}
	explicit operator RgbInt() const{
		return (RgbInt)(RgbFloat<T>)*this;
	}
	T distance_sq(const Grayscale<T> &other, T = 1) const{
		auto diff = *this - other;
		return diff.norm_sq();
	}
	Grayscale<T> saturate_channels() const{
		return Grayscale<T>(saturate<T>(this->x, (T)0.0, (T)1.0));
	}
	T brightness() const{
		return this->x;
	}
};

template <typename T>
struct template_pow{};

template <>
struct template_pow<float>{
	static float pow(float x, float y){
		return powf(x, y);
	}
};

template <>
struct template_pow<double>{
	static double pow(double x, double y){
		return ::pow(x, y);
	}
};

template <typename T>
void fix_var_xyz(T &x){
	if (x > (T)0.04045)
		x = template_pow<T>::pow((x + (T)0.055) * (T)(1.0 / 1.055), (T)2.4) * (T)100.0;
	else
		x *= (T)(100.0 / 12.92);
}

template <typename T>
void unfix_var_xyz(T &x){
	if (x > (T)0.0031308)
		x = (T)1.055 * template_pow<T>::pow(x, (T)(1.0 / 2.4)) - (T)0.055;
	else
		x *= (T)12.92;
}

template <typename T>
void fix_var_cielab(T &x){
	if (x > (T)0.008856)
		x = template_pow<T>::pow(x, (T)(1.0 / 3.0));
	else
		x = x * (T)7.787 + (T)(16.0 / 116.0);
}

template <typename T>
void unfix_var_cielab(T &x){
	auto cubed = x * x * x;
	if (cubed > (T)0.008856)
		x = cubed;
	else
		x = (x - (T)(16.0 / 116.0)) * (T)(1.0 / 7.787);
}

//RgbInt

template <typename T>
RgbInt::operator Cielab<T>() const{
	return (Cielab<T>)(RgbFloat<T>)*this;
}

template <typename T>
RgbFloat<T>::operator Xyz<T>() const{
	auto r = this->r();
	auto g = this->g();
	auto b = this->b();

	fix_var_xyz(r);
	fix_var_xyz(g);
	fix_var_xyz(b);

	return Xyz<T>{
		r * (T)0.4124 + g * (T)0.3576 + b * (T)0.1805,
		r * (T)0.2126 + g * (T)0.7152 + b * (T)0.0722,
		r * (T)0.0193 + g * (T)0.1192 + b * (T)0.9505,
	};
}

template <typename T>
RgbFloat<T>::operator Cielab<T>() const{
	return (Cielab<T>)(Xyz<T>)*this;
}

template <typename T>
RgbFloat<T>::operator RgbInt() const{
	auto temp = this->saturate_channels() * 255;
	return RgbInt{
		(byte_t)temp.r(),
		(byte_t)temp.g(),
		(byte_t)temp.b(),
	};
}

//Xyz

template <typename T>
Xyz<T>::operator RgbFloat<T>() const{
	auto x = this->x[0] * (T)(1.0 / 100.0);
	auto y = this->x[1] * (T)(1.0 / 100.0);
	auto z = this->x[2] * (T)(1.0 / 100.0);

	auto r = x * (T) 3.2406 + y * (T)-1.5372 + z * (T)-0.4986;
	auto g = x * (T)-0.9689 + y * (T) 1.8758 + z * (T) 0.0415;
	auto b = x * (T) 0.0557 + y * (T)-0.2040 + z * (T) 1.0570;

	unfix_var_xyz(r);
	unfix_var_xyz(g);
	unfix_var_xyz(b);

	return RgbFloat<T>{
		saturate<T>(r, 0, 1),
		saturate<T>(g, 0, 1),
		saturate<T>(b, 0, 1),
	};
}

template <typename T>
Xyz<T>::operator Cielab<T>() const{
	auto x = this->x[0] * (T)(1.0 / 95.047);
	auto y = this->x[1] * (T)(1.0 / 100.0);
	auto z = this->x[2] * (T)(1.0 / 108.883);

	fix_var_cielab(x);
	fix_var_cielab(y);
	fix_var_cielab(z);

	return Cielab<T>{
		(T)116.0 * y - 16, //not a paretheses error
		(T)500.0 * (x - y),
		(T)200.0 * (y - z),
	};
}

//Cielab

template <typename T>
Cielab<T>::operator RgbFloat<T>() const{
	return (RgbFloat<T>)(Xyz<T>)*this;
}

template <typename T>
Cielab<T>::operator Xyz<T>() const{
	auto y = (this->L() + (T)16.0) * (T)(1.0 / 116.0);
	auto x = this->a() * (T)(1.0 / 500.0) + y;
	auto z = y - this->b() * (T)(1.0 / 200.0);

	unfix_var_cielab(x);
	unfix_var_cielab(y);
	unfix_var_cielab(z);

	return Xyz<T>{
		(T)95.047 * x,
		(T)100.0 * y,
		(T)108.883 * z,
	};
}

#endif
