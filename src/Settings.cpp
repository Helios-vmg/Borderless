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

#include "Settings.h"
#include <QStandardPaths>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QStandardPaths>

//#define USE_XML
#define USE_JSON
#ifdef USE_XML
#include <QtXml/QDomDocument>
#endif
#ifdef USE_JSON
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#endif

bool get_config_location(QString &dst);

void SettingsValue::operator=(bool val){
	this->value = QVariant((int)val);
}

Settings::Settings()
{
	if (!get_config_location(this->path))
		throw SettingsException("Can't determine settings path.");
	QDir dir(this->path);
	if (!dir.mkpath(this->path))
		throw SettingsException("Can't create settings path.");
	this->path += "settings.";
#if defined USE_XML
	this->path += "xml";
#elif defined USE_JSON
	this->path += "json";
#endif
}

#ifdef USE_XML
void write(QDomDocument &doc, QDomElement &element, SettingsTree &tree){
	auto f1 = [&element, &doc](const QString &name, SettingsValue &value){
		auto el = doc.createElement(name);
		el.appendChild(doc.createTextNode(value.to_string()));
		element.appendChild(el);
	};
	auto f2 = [&element, &doc, this](const QString &name, SettingsTree &tree){
		auto el = doc.createElement(name);
		write(doc, el, tree);
		element.appendChild(el);
	};
	tree.iterate_over_values(f1);
	tree.iterate_over_trees(f2);
}
#endif

#ifdef USE_JSON
void write_tree(QJsonObject &obj, SettingsTree &tree);
void write_array(QJsonObject &obj, SettingsTree &tree);

void write_array(QJsonArray &array, SettingsArray &tree){
	tree.iterate([&](int i, SettingsItem &item){
		if (item.is_value())
			array.push_back(QJsonValue(static_cast<SettingsValue &>(item).to_string()));
		else if (item.is_array()){
			QJsonArray new_array;
			write_array(new_array, static_cast<SettingsArray &>(item));
			array.push_back(new_array);
		}else{
			QJsonObject object;
			write_tree(object, static_cast<SettingsTree &>(item));
			array.push_back(object);
		}
	});
}

void write_tree(QJsonObject &obj, SettingsTree &tree){
	tree.iterate([&](const QString &key, SettingsItem &item){
		if (item.is_value())
			obj[key] = QJsonValue(static_cast<SettingsValue &>(item).to_string());
		else if (item.is_array()){
			QJsonArray new_array;
			write_array(new_array, static_cast<SettingsArray &>(item));
			obj[key] = new_array;
		}else{
			QJsonObject object;
			write_tree(object, static_cast<SettingsTree &>(item));
			obj[key] = object;
		}
	});
}
#endif

void Settings::write(SettingsTree &tree){
#ifdef USE_XML
	QDomDocument doc;
	auto el = doc.createElement("settings");
	::write(doc, el, tree);
	doc.appendChild(el);
	QFile file(this->path);
	file.open(QFile::WriteOnly);
	if (!file.isOpen())
		return;
	QTextStream stream(&file);
	doc.save(stream, 4);
#endif
#ifdef USE_JSON
	QJsonDocument doc;
	QJsonObject root;
	::write_tree(root, tree);
	doc.setObject(root);
	QFile file(this->path);
	file.open(QFile::WriteOnly);
	if (!file.isOpen())
		return;
	file.write(doc.toJson(QJsonDocument::Indented));
#endif
}

#ifdef USE_JSON
std::shared_ptr<SettingsTree> read_obj(QJsonObject &obj);
std::shared_ptr<SettingsArray> read_array(QJsonArray &arr);

template <typename T>
std::shared_ptr<SettingsItem> add_sub(const T &key, QJsonValueRef &item){
	if (item.isObject())
		return key, read_obj(item.toObject());
	if (item.isArray())
		return key, read_array(item.toArray());
	if (item.isString())
		return std::shared_ptr<SettingsItem>(new SettingsValue(item.toString()));
	return std::shared_ptr<SettingsItem>();
}

std::shared_ptr<SettingsArray> read_array(QJsonArray &arr){
	std::shared_ptr<SettingsArray> ret(new SettingsArray);
	auto size = arr.size();
	for (int i = 0; i < size; i++){
		auto item = add_sub(i, arr[i]);
		if (!item)
			continue;
		ret->push_back(item);
	}
	return ret;
}

std::shared_ptr<SettingsTree> read_obj(QJsonObject &obj){
	std::shared_ptr<SettingsTree> ret(new SettingsTree);
	auto keys = obj.keys();
	for (auto &key : keys){
		auto item = add_sub(key, obj[key]);
		if (!item)
			continue;
		ret->add_tree(key, item);
	}
	return ret;
}
#endif

std::shared_ptr<SettingsTree> Settings::read(){
#ifdef USE_JSON
	std::shared_ptr<SettingsTree> ret;
	QFile file(this->path);
	file.open(QFile::ReadOnly);
	if (!file.isOpen())
		return ret;
	auto doc = QJsonDocument::fromJson(file.readAll());
	if (doc.isNull())
		return ret;
	return ::read_obj(doc.object());
#endif
}

bool get_config_location(QString &dst){
	auto list = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
	if (!list.size())
		return false;
	dst = list[0];
	int index = dst.lastIndexOf('/');
	if (index < 0)
		index = dst.lastIndexOf('\\');
	dst = dst.mid(0, index);
	auto c = QDir::separator();
	dst += c;
	dst += "BorderlessImageViewer";
	dst += c;
	return true;
}
