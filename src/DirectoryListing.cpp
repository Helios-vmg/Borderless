/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "DirectoryListing.h"
#include <QDir>
#include <QtConcurrent/QtConcurrentRun>
#include <algorithm>

const Qt::CaseSensitivity platorm_case = Qt::CaseInsensitive;

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
	"*.tiff",
	"*.jp2",
};

template <Qt::CaseSensitivity CS>
bool strcmpci(const QString &a, const QString &b){
	return a.compare(a, b, CS) < 0;
}

QStringList get_entries(QString path){
	QDir directory(path);
	directory.setFilter(QDir::Files | QDir::Hidden);
	directory.setSorting(QDir::Name);
	QStringList filters;
	for (auto p : supported_extensions)
		filters << p;
	directory.setNameFilters(filters);
	auto ret = directory.entryList();
	auto f = strcmpci<platorm_case>;
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

DirectoryListing::DirectoryListing(const QString &path){
	this->base_path = path;
	this->ok = check_and_clean_path(this->base_path);
	if (!this->ok)
		return;
	this->entries = QtConcurrent::run(get_entries, path);
}

bool DirectoryListing::operator==(const QString &path) const{
	return !this->base_path.compare(this->base_path, path, platorm_case);
}

DirectoryIterator DirectoryListing::begin(){
	return DirectoryIterator(*this);
}

size_t DirectoryListing::size(){
	return this->entries.result().size();
}

QString DirectoryListing::operator[](size_t i) const{
	auto ret = this->base_path;
	ret += QDir::separator();
	ret += this->entries.result()[i];
	return ret;
}

bool DirectoryListing::find(size_t &dst, const QString &s) const{
	auto f = strcmpci<platorm_case>;
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
