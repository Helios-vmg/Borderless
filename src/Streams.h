/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef STREAMS_H
#define STREAMS_H

#include <boost/iostreams/stream.hpp>
#include <vector>
#include <cstdint>

class QFile;

class QFileInputStream{
	QFile *file;
public:
	typedef char char_type;
	typedef boost::iostreams::source_tag category;
	QFileInputStream(QFile *file): file(file){}
	std::streamsize read(char *s, std::streamsize n);
};

class QFileOutputStream{
	QFile *file;
public:
	typedef char char_type;
	typedef boost::iostreams::sink_tag category;
	QFileOutputStream(QFile *file): file(file){}
	std::streamsize write(const char *s, std::streamsize n);
};

class MemoryStream{
public:
	std::vector<std::uint8_t> *data;
	
	typedef char char_type;
	typedef boost::iostreams::sink_tag category;
	MemoryStream(std::vector<std::uint8_t> *data): data(data){}
	std::streamsize write(const char *s, std::streamsize n);
};


#endif
