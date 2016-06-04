/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "Streams.h"
#include <QFile>

std::streamsize QFileInputStream::read(char *s, std::streamsize n){
	std::streamsize ret = 0;
	bool bad = false;
	while (n){
		auto count = this->file->read(s, n);
		ret += count;
		s += count;
		n -= count;
		if (this->file->error() != QFileDevice::NoError || this->file->atEnd()){
			bad = true;
			break;
		}
	}
	return !ret && bad ? -1 : ret;
}

std::streamsize QFileOutputStream::write(const char *s, std::streamsize n){
	return this->file->write(s, n);
}
