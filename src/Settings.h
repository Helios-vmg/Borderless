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

#ifndef SETTINGS_H
#define SETTINGS_H
#include "Rational.h"
#include "Misc.h"
#include <exception>
#include <map>
#include <memory>
#include <QVariant>
#include <QString>
#include <QPoint>
#include <QSize>
#include <QDebug>
#include <QMatrix>
#include <QKeySequence>
#include <type_traits>
#include <sstream>

class SettingsException : public std::exception{
public:
	SettingsException(const char *what): std::exception(what){}
};

class SettingsItem{
	bool m_value;
	bool m_array;
public:
	SettingsItem(bool val, bool arr = false): m_value(val), m_array(arr){}
	virtual ~SettingsItem(){}
	bool is_value() const{
		return this->m_value;
	}
	bool is_array() const{
		return this->m_array;
	}
};

class SettingsValue : public SettingsItem{
	QVariant value;
	bool unset;

	template <typename T>
	static typename std::enable_if<std::is_enum<T>::value>::type conditional_cast_to_int(SettingsValue &dst, const T &src){
		dst = (int)src;
	}

	template <typename T>
	static typename std::enable_if<!std::is_enum<T>::value>::type conditional_cast_to_int(SettingsValue &dst, const T &src){
		dst = src;
	}

public:
	SettingsValue(): SettingsItem(true), unset(true){}
	template <typename T>
	SettingsValue(const T &val): SettingsItem(true), unset(false){
		conditional_cast_to_int(*this, val);
	}
	template <typename T>
	void operator=(const T &val){
		this->value = QVariant(val);
	}
	void operator=(bool val);
	void operator=(const QPoint &val){
		*this = ::to_pair(val);
	}
	void operator=(const QSize &val){
		*this = ::to_pair(val);
	}
	template <typename T>
	void operator=(const basic_rational<T> &val){
		*this = val.to_pair();
	}
	template <typename T>
	void operator=(const std::pair<T, T> &val){
		std::stringstream stream;
		stream << val.first << ' ' << val.second;
		*this = stream.str().c_str();
	}
	void operator=(const QMatrix &val){
		std::stringstream stream;
		stream
			<< val.m11() << ' '
			<< val.m21() << ' '
			<< val.m12() << ' '
			<< val.m22();
		*this = stream.str().c_str();
	}
	void operator=(const QKeySequence &seq){
		*this = seq.toString();
	}
	bool is_unset() const{
		return this->unset;
	}
	QString to_string() const{
		return this->value.toString();
	}
	int to_int() const{
		return this->value.toInt();
	}
	bool to_bool() const{
		return this->value.toBool();
	}
	double to_double() const{
		return this->value.toDouble();
	}
	QPoint to_QPoint() const{
		return ::to_QPoint(this->to_pair<int>());
	}
	QSize to_QSize() const{
		return ::to_QSize(this->to_pair<int>());
	}
	QMatrix to_QMatrix() const{
		std::stringstream stream(this->to_string().toStdString());
		double m[4];
		stream
			>> m[0]
			>> m[1]
			>> m[2]
			>> m[3];
		return QMatrix(m[0], m[2], m[1], m[3], 0, 0);
	}
	template <typename T>
	basic_rational<T> to_basic_rational() const{
		return basic_rational<T>(this->to_pair<T>());
	}
	template <typename T>
	std::pair<T, T> to_pair() const{
		std::stringstream stream(this->to_string().toStdString());
		std::pair<T, T> ret;
		stream >> ret.first >> ret.second;
		return ret;
	}
	template <typename T>
	void to(T &dst) const{
		if (std::is_enum<T>::value)
			dst = (T)this->to_int();
	}
	void to(QKeySequence &dst) const{
		dst = this->to_string();
	}
	void to(QString &dst) const{
		dst = this->to_string();
	}
	void to(int &dst) const{
		dst = this->to_int();
	}
	void to(bool &dst) const{
		dst = this->to_bool();
	}
	void to(double &dst) const{
		dst = this->to_double();
	}
	void to(QPoint &dst) const{
		dst = this->to_QPoint();
	}
	void to(QSize &dst) const{
		dst = this->to_QSize();
	}
	template <typename T>
	void to(basic_rational<T> &dst) const{
		dst = this->to_basic_rational<T>();
	}
	template <typename T>
	void to(std::pair<T, T> &dst) const{
		dst = this->to_pair<T>();
	}
	void to(QMatrix &dst) const{
		dst = this->to_QMatrix();
	}
};

class SettingsTree;
class SettingsArray;

template <typename KeyT, typename MapT>
class AssociativeTree : public SettingsItem{
protected:
	MapT tree;
	int get_int(const KeyT &s) const{
		auto it = this->find(s);
		if (!it)
			return 0;
		if (it->is_value()){
			auto val = *static_cast<SettingsValue *>(it->second.get());
			return val.to_int();
		}
		return 0;
	}
	virtual std::shared_ptr<SettingsItem> find(const KeyT &k) const = 0;

public:
	AssociativeTree(bool is_array): SettingsItem(false, is_array){}
	template <typename T>
	void set_value(const KeyT &key, const T &val){
		this->tree[key] = std::shared_ptr<SettingsItem>(new SettingsValue(val));
	}
	template <typename T>
	void add_tree(const KeyT &key, const T &subtree){
		this->tree[key] = std::static_pointer_cast<SettingsItem>(subtree);
	}
	/*
	template <typename T>
	void iterate_over_values(T &f) const{
		for (auto &i : this->tree){
			std::shared_ptr<SettingsItem> item = i.second;
			if (item->is_value())
				f(i.first, *static_cast<SettingsValue *>(item.get()));
		}
	}
	template <typename T>
	void iterate_over_trees(T &f) const;
	template <typename T>
	void iterate_over_arrays(T &f) const;
	*/
	template <typename T>
	void get_value(T &dst, const KeyT &key) const{
		auto it = this->find(key);
		if (!it)
			return;
		std::shared_ptr<SettingsItem> p = it;
		if (!p->is_value())
			return;
		SettingsValue &val = static_cast<SettingsValue &>(*p.get());
		val.to(dst);
	}
	std::shared_ptr<SettingsItem> operator[](const KeyT &key){
		auto it = this->find(key);
		if (!it)
			return std::shared_ptr<SettingsItem();
		return it->second;
	}
	std::shared_ptr<SettingsTree> get_tree(const KeyT &key) const;
	std::shared_ptr<SettingsArray> get_array(const KeyT &key) const;
	static std::shared_ptr<SettingsTree> create_tree();
	static std::shared_ptr<SettingsArray> create_array();
};

class SettingsTree : public AssociativeTree<QString, std::map<QString, std::shared_ptr<SettingsItem> > > {
protected:
	std::shared_ptr<SettingsItem> find(const QString &k) const override{
		auto it = this->tree.find(k);
		if (it == this->tree.end())
			return nullptr;
		return it->second;
	}

public:
	SettingsTree() : AssociativeTree<QString, std::map<QString, std::shared_ptr<SettingsItem> > >(false){}
	template <typename T>
	void iterate(T &f) const{
		for (auto &i : this->tree)
			f(i.first, *i.second);
	}
};

class SettingsArray: public AssociativeTree<int, std::vector<std::shared_ptr<SettingsItem> > > {
protected:
	std::shared_ptr<SettingsItem> find(const int &k) const override{
		if (k < 0 || k >= this->tree.size())
			return nullptr;
		return this->tree[k];
	}

public:
	SettingsArray() : AssociativeTree<int, std::vector<std::shared_ptr<SettingsItem> > >(true){}
	size_t size() const{
		return this->tree.size();
	}
	template <typename T>
	void iterate(T &f) const{
		int p = 0;
		for (auto &i : this->tree)
			f(p++, *i);
	}
	void push_back(const std::shared_ptr<SettingsItem> &item){
		this->tree.push_back(item);
	}
};

template <typename KeyT, typename MapT>
std::shared_ptr<SettingsTree> AssociativeTree<KeyT, MapT>::create_tree(){
	return std::shared_ptr<SettingsTree>(new SettingsTree);
}

template <typename KeyT, typename MapT>
std::shared_ptr<SettingsArray> AssociativeTree<KeyT, MapT>::create_array(){
	return std::shared_ptr<SettingsArray>(new SettingsArray);
}

template <typename KeyT, typename MapT>
std::shared_ptr<SettingsTree> AssociativeTree<KeyT, MapT>::get_tree(const KeyT &key) const{
	auto it = this->tree.find(key);
	return it == this->tree.end() || it->second->is_value() || it->second->is_array() ? nullptr : std::static_pointer_cast<SettingsTree >(it->second);
}

template <typename KeyT, typename MapT>
std::shared_ptr<SettingsArray> AssociativeTree<KeyT, MapT>::get_array(const KeyT &key) const{
	auto it = this->tree.find(key);
	return it == this->tree.end() || it->second->is_value() || !it->second->is_array() ? nullptr : std::static_pointer_cast<SettingsArray >(it->second);
}
/*
template <typename KeyT, typename MapT>
template <typename T>
void AssociativeTree<KeyT, MapT>::iterate_over_trees(T &f) const{
	for (auto &i : this->tree){
		std::shared_ptr<SettingsItem> item = i.second;
		if (!item->is_value() && !item->is_array())
			f(i.first, *static_cast<SettingsTree *>(item.get()));
	}
}

template <typename KeyT, typename MapT>
template <typename T>
void AssociativeTree<KeyT, MapT>::iterate_over_arrays(T &f) const{
	for (auto &i : this->tree){
		std::shared_ptr<SettingsItem> item = i.second;
		if (!item->is_value() && item->is_array())
			f(i.first, *static_cast<SettingsArray *>(item.get()));
	}
}
*/
class Settings
{
	QString path;
public:
	Settings();
	void write(SettingsTree &tree);
	std::shared_ptr<SettingsTree> read();
};

#define DEFINE_SETTING_STRING(x) static const char *x##_setting = #x
#define DEFINE_SETTING(type, name, default_value) static const type default_##name = default_value; \
	type name
#define DEFINE_SETTING2(type, name) static const type default_##name; \
	type name
#define RESET_SETTING(x) this->x = this->default_##x

#endif // SETTINGS_H
