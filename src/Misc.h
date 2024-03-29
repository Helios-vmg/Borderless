/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef MISC_H
#define MISC_H

#include "GenericException.h"
#include <QSize>
#include <QPoint>
#include <QStringList>
#include <QDataStream>
#include <sstream>
#include <string>
#include <iomanip>
#include <QIODevice>

template <typename T1, typename T2>
std::basic_string<T1> itoa(const T2 &x, unsigned w = 0){
	std::basic_stringstream<T1> stream;
	if (w)
		stream << std::setw(w) << std::setfill('0');
	stream << x;
	return stream.str();
}
template <typename T2>
std::string itoac(const T2 &x, unsigned w = 0){
	return itoa<char>(x, w);
}

template <typename T>
void zero_struct(T &s){
	memset(&s, 0, sizeof(s));
}

template <typename T>
T intabs(T x){
	return x < 0 ? -x : x;
}

template <typename T1, typename T2>
typename std::enable_if<!(std::is_enum<T1>::value || std::is_enum<T2>::value), bool>::type check_flag(const T1 &testee, const T2 &tester){
	return (testee & tester) == tester;
}

template <typename T1, typename T2>
typename std::enable_if<std::is_enum<T1>::value || std::is_enum<T2>::value, bool>::type check_flag(const T1 &testee, const T2 &tester){
	return ((unsigned)testee & (unsigned)tester) == (unsigned)tester;
}

template <typename T>
typename std::enable_if<std::is_enum<T>::value || std::is_enum<T>::value, T>::type disable_flag(const T &value, const T &flag){
	return (T)((unsigned)value & ~(unsigned)flag);
}

template <typename T, size_t N>
size_t array_length(T (&array)[N]){
	return N;
}

template <typename T>
T closest(T *v, size_t n, T x){
	T min = std::numeric_limits<T>::max();
	T ret = x;
	if (!n)
		return ret;
	for (auto i = n; i--; ){
		if (intabs(v[i] - x) < min){
			min = intabs(v[i] - x);
			ret = v[i];
		}
	}
	return ret;
}

template <typename T, size_t N>
T closest(T (&v)[N], T x){
	return closest(v, N, x);
}

inline QSize to_QSize(const QPoint &p){
	return QSize(p.x(), p.y());
}

inline QPoint to_QPoint(const QSize &s){
	return QPoint(s.width(), s.height());
}

inline QPointF to_QPointF(const QSize &s){
	return QPointF(s.width(), s.height());
}

inline QSize to_QSize(const std::pair<int, int> &p){
	return QSize(p.first, p.second);
}

inline QPoint to_QPoint(const std::pair<int, int> &p){
	return QPoint(p.first, p.second);
}

inline std::pair<int, int> to_pair(const QSize &p){
	return std::make_pair(p.width(), p.height());
}

inline std::pair<int, int> to_pair(const QPoint &p){
	return std::make_pair(p.x(), p.y());
}

inline QByteArray to_QByteArray(const QStringList &list){
	QByteArray ret;
	QDataStream stream(&ret, QIODevice::WriteOnly);
	stream << list;
	return ret;
}

inline QStringList to_QStringList(const QByteArray &array){
	QStringList ret;
	//QDataStream stream(&array, QIODevice::ReadOnly);
	QDataStream stream(array);
	stream >> ret;
	return ret;
}

inline void split_path(QString &directory, QString &file, const QString &path){
	int i = path.lastIndexOf('/');
	if (i < 0)
		i = path.lastIndexOf('\\');
	if (i < 0){
		directory.clear();
		file = path;
	}
	directory = path.mid(0, i);
	file = path.mid(i + 1);
}

template <typename T>
void set_min(T &a, const T &b){
	a = std::min(a, b);
}

template <typename T>
void set_max(T &a, const T &b){
	a = std::max(a, b);
}

class OptionalNotSetException : public GenericException{
public:
	OptionalNotSetException(): GenericException("Attempting to access the object of an uninitialized Optional."){}
};

template <typename T>
class Optional{
	bool set;
	T data;
public:
	Optional(): set(false){}
	Optional(const T &data): set(true), data(data){}
	Optional(const Optional<T> &existing): set(existing.set), data(existing.data){}
	Optional(Optional<T> &&existing): set(existing.set), data(existing.data){
		existing.clear();
	}
	const Optional<T> &operator=(const Optional<T> &existing){
		this->set = existing.set;
		this->data = existing.data;
		return *this;
	}
	const Optional<T> &operator=(Optional<T> &&existing){
		this->set = existing.set;
		this->data = existing.data;
		existing.clear();
		return *this;
	}
	const Optional<T> &operator=(const T &data){
		this->set = true;
		this->data = data;
		return *this;
	}
	const Optional<T> &operator=(T &&data){
		this->set = true;
		this->data = data;
		return *this;
	}
	void clear(){
		this->set = false;
	}
	operator bool() const{
		return this->set;
	}
	T &operator*(){
		if (!*this)
			throw OptionalNotSetException();
		return this->data;
	}
	const T &operator*() const{
		if (!*this)
			throw OptionalNotSetException();
		return this->data;
	}
	T *operator->(){
		return &this->data;
	}
	const T *operator->() const{
		return &this->data;
	}
};

inline void to_lower(std::string &s){
	std::transform(s.begin(), s.end(), s.begin(), tolower);
}

#endif // MISC_H
