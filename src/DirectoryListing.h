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

#ifndef DIRECTORYLISTING_H
#define DIRECTORYLISTING_H

#include <QString>
#include <QStringList>
#include <QFuture>
#include <vector>

bool check_and_clean_path(QString &path);

class DirectoryIterator;

class DirectoryListing{
	bool ok;
	QString base_path;
	QFuture<QStringList> entries;
public:
	DirectoryListing(const QString &path);
	DirectoryIterator begin();
	size_t size();
	QString operator[](size_t) const;
	bool find(size_t &, const QString &) const;
	operator bool() const{
		return this->ok;
	}
	bool operator==(const QString &path) const;
};

class DirectoryIterator{
	DirectoryListing *dl;
	size_t position;
	bool in_position;
public:
	DirectoryIterator(DirectoryListing &dl): dl(&dl), position(0), in_position(false){}
	bool advance_to(const QString &name){
		if (this->in_position)
			return true;
		return this->in_position = this->dl->find(this->position, name);
	}
	QString operator*() const{
		return (*this->dl)[this->position];
	}
	void operator++(){
		this->position = (this->position + 1) % this->dl->size();
	}
	void operator--(){
		auto n = this->dl->size();
		this->position = (this->position + n - 1) % n;
	}
	DirectoryListing *get_listing() const{
		return this->dl;
	}
	size_t pos() const{
		return this->position;
	}
	void to_start(){
		this->position = 0;
	}
	void to_end(){
		this->to_start();
		--*this;
	}
};

#endif // DIRECTORYLISTING_H
