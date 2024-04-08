/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef EXIF_H
#define EXIF_H

#include "TinyEXIF/TinyEXIF.h"
#include <vector>
#include <utility>
#include <string>
#include <QString>
#include <QIODevice>
#include <QSize>

class ImageMetadata{
	std::vector<std::pair<std::string, std::string>> human_metadata;
	TinyEXIF::EXIFInfo machine_metadata;
	QString name;
	QString path;
	std::pair<std::uint64_t, std::uint64_t> color_count = {};
	std::uint64_t size;
	std::pair<QSize, int> dimensions;
public:
	ImageMetadata() = default;
	ImageMetadata(QImage &image, const QString &);
	ImageMetadata(std::unique_ptr<QIODevice> &&);
	ImageMetadata(const ImageMetadata &) = default;
	ImageMetadata &operator=(const ImageMetadata &) = default;
	ImageMetadata(ImageMetadata &&) = default;
	ImageMetadata &operator=(ImageMetadata &&) = default;
	const auto &get_machine() const{
		return this->machine_metadata;
	}
	const auto &get_human() const{
		return this->human_metadata;
	}
	std::pair<int, bool> get_orientation() const;
	QString get_filename() const{
		return this->name;
	}
	QString get_full_path() const{
		return this->path;
	}
	//first is dimensions, second is page (or frame) count
	std::pair<QSize, int> get_size() const{
		return this->dimensions;
	}
	void set_color_count(std::pair<std::uint64_t, std::uint64_t> count){
		this->color_count = count;
	}
	auto get_color_count() const{
		return this->color_count;
	}
	std::uint64_t get_filesize() const{
		return this->size;
	}
};

#endif
