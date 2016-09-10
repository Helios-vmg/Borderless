/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "DirectoryListing.h"
#include "ImageViewerApplication.h"
#include "ProtocolModule.h"
#include <QDir>
#include <QtConcurrent/QtConcurrentRun>
#include <algorithm>

const Qt::CaseSensitivity platform_case = Qt::CaseInsensitive;

static const char *supported_extensions[] = {
	"*.bmp",
	"*.jpg",
	"*.jpeg",
	"*.png",
	"*.gif",
	"*.svg",
	"*.webp",
	"*.ico",
	"*.tga",
	"*.tif",
	"*.tiff",
	"*.jp2",
};

template <Qt::CaseSensitivity CS>
bool strcmpci(const QString &a, const QString &b){
	return a.compare(a, b, CS) < 0;
}

QStringList get_local_entries(QString path){
	QDir directory(path);
	directory.setFilter(QDir::Files | QDir::Hidden);
	directory.setSorting(QDir::Name);
	QStringList filters;
	for (auto p : supported_extensions)
		filters << p;
	directory.setNameFilters(filters);
	auto ret = directory.entryList();
	auto f = strcmpci<platform_case>;
	std::sort(ret.begin(), ret.end(), f);
	return ret;
}

bool check_and_clean_path(QString &path){
	QDir directory(path);
	if (!directory.exists()){
		return false;
	}
	path = directory.cleanPath(path);
	path = directory.toNativeSeparators(path);
	return true;
}

LocalDirectoryListing::LocalDirectoryListing(const QString &path){
	this->base_path = path;
	this->ok = check_and_clean_path(this->base_path);
	if (!this->ok)
		return;
	this->entries = QtConcurrent::run(get_local_entries, path);
}

bool LocalDirectoryListing::operator==(const QString &path){
	return !this->base_path.compare(this->base_path, path, platform_case);
}

DirectoryIterator DirectoryListing::begin(){
	return DirectoryIterator(*this);
}

size_t LocalDirectoryListing::size(){
	return this->entries.result().size();
}

QString LocalDirectoryListing::operator[](size_t i){
	auto ret = this->base_path;
	ret += QDir::separator();
	ret += this->entries.result()[i];
	return ret;
}

bool LocalDirectoryListing::find(size_t &dst, const QString &s){
	auto f = strcmpci<platform_case>;
	auto entries = this->entries.result();
	auto it = std::lower_bound(entries.begin(), entries.end(), s, f);
	if (it == entries.end())
		return false;
	qDebug() << s;
	qDebug() << *it;
	if (f(s, *it))
		return false;
	dst = it - entries.begin();
	return true;
}

QString LocalDirectoryListing::get_filename(size_t i){
	return this->entries.result()[i];
}

ProtocolDirectoryListing::list_t get_protocol_entries(QString path, CustomProtocolHandler *handler){
	typedef ProtocolDirectoryListing::list_t::element_type t;

	ProtocolDirectoryListing::list_t ret;
	auto list = handler->enumerate_siblings(path);
	ret.reset(new t);
	ret->reserve(list.size());
	for (auto &s : list)
		ret->push_back(t::value_type(s, handler->get_filename(s)));
	return ret;
}

ProtocolDirectoryListing::list_t::element_type &ProtocolDirectoryListing::get_result(){
	if (!this->future_result)
		this->future_result = this->future.result();
	return *this->future_result;
}

ProtocolDirectoryListing::ProtocolDirectoryListing(const QString &path, CustomProtocolHandler &handler): handler(&handler){
	this->ok = false;
	this->base_path = handler.get_parent_directory(path);
	this->future = QtConcurrent::run(get_protocol_entries, path, &handler);
	this->ok = true;
}

size_t ProtocolDirectoryListing::size(){
	return this->get_result().size();
}

QString ProtocolDirectoryListing::operator[](size_t i){
	return this->get_result()[i].first;
}

bool ProtocolDirectoryListing::find(size_t &dst, const QString &s){
	const auto &v = this->get_result();
	for (size_t i = 0; i < v.size(); i++){
		if (v[i].second == s){
			dst = i;
			return true;
		}
	}
	return false;
}

bool ProtocolDirectoryListing::operator==(const QString &path){
	return this->get_result().size() && this->handler->paths_in_same_directory(this->get_result().front().first, path);
}

QString ProtocolDirectoryListing::get_filename(size_t i){
	return this->get_result()[i].second;
}
