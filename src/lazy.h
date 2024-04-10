/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef LAZY_H
#define LAZY_H

#include <optional>
#include <functional>

template <typename T>
class Dynamic{
public:
	virtual ~Dynamic(){}
	virtual const T &operator*() = 0;
};

template <typename T>
class Lazy : public Dynamic<T>{
	std::optional<T> value;
	std::function<T()> getter;
public:
	Lazy() = default;
	Lazy(std::function<T()> getter): getter(std::move(getter)){}
	Lazy(const Lazy &) = default;
	Lazy &operator=(const Lazy &) = default;
	Lazy(Lazy &&) = default;
	Lazy &operator=(Lazy &&) = default;
	const T &operator*() override{
		if (!this->value){
			if (this->getter){
				this->value = this->getter();
				this->getter = {};
			}else
				this->value.emplace();
		}
		return *this->value;
	}
};

template <typename T>
class Eager : public Dynamic<T>{
	T value;
public:
	Eager(T value): value(std::move(value)){}
	Eager(const Eager &) = default;
	Eager &operator=(const Eager &) = default;
	Eager(Eager &&) = default;
	Eager &operator=(Eager &&) = default;
	const T &operator*() override{
		return this->value;
	}
};

#endif
