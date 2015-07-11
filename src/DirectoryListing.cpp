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

#include "directorylisting.h"
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
