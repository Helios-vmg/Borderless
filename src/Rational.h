/*

Copyright (c) 2015, Helios
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef RATIONAL_H
#define RATIONAL_H
#include <QSize>
#include <QPoint>
#include <utility>

template <typename T>
class basic_rational{
	T numerator;
	T denominator;

	void normalize(){
		auto a = this->numerator;
		auto b = this->denominator;
		if (a < 0)
			a = -a;
		if (b < 0)
			b = -b;
		while (b){
			auto t = b;
			b = a % b;
			a = t;
		}
		if (!a)
			return;
		this->numerator /= a;
		this->denominator /= a;
		if (this->denominator < 0){
			this->numerator = -this->numerator;
			this->denominator = -this->denominator;
		}
	}
public:
	basic_rational(T n = 0){
		*this = n;
	}
	basic_rational(T a, T b): numerator(a), denominator(b){
		this->normalize();
	}
	template <typename T2>
	basic_rational(const std::pair<T2, T2> &p){
		*this = basic_rational<T>((T)p.first, (T)p.second);
	}
	basic_rational(const basic_rational<T> &q){
		*this = q;
	}
	std::pair<T, T> to_pair() const{
		return std::make_pair(this->numerator, this->denominator);
	}
	T get_numerator() const{
		return this->numerator;
	}
	T get_denominator() const{
		return this->denominator;
	}
	const basic_rational<T> &operator=(const basic_rational<T> &q){
		this->numerator = q.numerator;
		this->denominator = q.denominator;
		return *this;
	}
	const basic_rational<T> &operator=(T n){
		this->numerator = n;
		this->denominator = 1;
		return *this;
	}

	const basic_rational<T> &operator+=(T n){
		this->numerator += n * this->denominator;
		return *this;
	}
	const basic_rational<T> &operator+=(const basic_rational<T> &q){
		this->numerator = this->numerator * q.denominator + q.numerator * this->denominator;
		this->denominator *= q.denominator;
		this->normalize();
		return *this;
	}

	const basic_rational<T> &operator-=(T n){
		this->numerator -= n * this->denominator;
		return *this;
	}
	const basic_rational<T> &operator-=(const basic_rational<T> &q){
		this->numerator = this->numerator * q.denominator - q.numerator * this->denominator;
		this->denominator *= q.denominator;
		this->normalize();
		return *this;
	}

	const basic_rational<T> &operator*=(T n){
		this->numerator *= n;
		this->normalize();
		return *this;
	}
	const basic_rational<T> &operator*=(const basic_rational<T> &q){
		this->numerator *= q.numerator;
		this->denominator *= q.denominator;
		this->normalize();
		return *this;
	}

	const basic_rational<T> &operator/=(T n){
		this->denominator *= n;
		this->normalize();
		return *this;
	}
	const basic_rational<T> &operator/=(const basic_rational<T> &q){
		this->numerator *= q.denominator;
		this->denominator *= q.numerator;
		this->normalize();
		return *this;
	}

	bool undefined() const{
		return !this->denominator;
	}
	bool null() const{
		return !this->numerator && !this->denominator;
	}
	basic_rational<T> reciprocal() const{
		return basic_rational<T>(this->denominator, this->numerator);
	}

#define DEFINE_RATIONAL_OVERLOAD(op) \
	basic_rational<T> operator##op(T n){ \
		auto ret = *this; \
		ret op##= n; \
		return ret; \
	} \
	basic_rational<T> operator##op(const basic_rational<T> &q){ \
		auto ret = *this; \
		ret op##= q; \
		return ret; \
	}
	DEFINE_RATIONAL_OVERLOAD(+)
	DEFINE_RATIONAL_OVERLOAD(-)
	DEFINE_RATIONAL_OVERLOAD(*)
	DEFINE_RATIONAL_OVERLOAD(/)
#undef DEFINE_RATIONAL_OVERLOAD

	operator T() const{
		return this->numerator / this->denominator;
	}
#define DEFINE_RATIONAL_RELATIONAL_OVERLOAD(op) \
	bool operator##op(const basic_rational<T> &q) const{ \
		return this->numerator * q.denominator op q.numerator * this->denominator; \
	}
	DEFINE_RATIONAL_RELATIONAL_OVERLOAD(<)
	DEFINE_RATIONAL_RELATIONAL_OVERLOAD(>)
	DEFINE_RATIONAL_RELATIONAL_OVERLOAD(<=)
	DEFINE_RATIONAL_RELATIONAL_OVERLOAD(>=)
	DEFINE_RATIONAL_RELATIONAL_OVERLOAD(==)
	DEFINE_RATIONAL_RELATIONAL_OVERLOAD(!=)
};

template <typename T>
inline T operator*(T n, const basic_rational<T> &q){
	return (T)(q * n);
}

template <typename T>
inline QSize operator*(const QSize &s, const basic_rational<T> &q){
	return s * q.get_numerator() / q.get_denominator();
}

template <typename T>
inline QPoint operator*(const QPoint &p, const basic_rational<T> &q){
	T x = p.x();
	T y = p.y();
	x = x * q.get_numerator() / q.get_denominator();
	y = y * q.get_numerator() / q.get_denominator();
	return QPoint(x, y);
}

template <typename T>
inline QSizeF operator*(const QSizeF &s, const basic_rational<T> &q){
	T w = s.width();
	T h = s.height();
	w = w * q.get_numerator() / q.get_denominator();
	h = h * q.get_numerator() / q.get_denominator();
	return QSize(w, h);
}

template <typename T>
inline QPointF operator*(const QPointF &p, const basic_rational<T> &q){
	return p * q.get_numerator() / q.get_denominator();
}

typedef basic_rational<int> Rational;
typedef basic_rational<long long> RationalLL;

#endif // RATIONAL_H
