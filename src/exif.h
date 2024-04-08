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

class ImageMetadata{
	std::vector<std::pair<std::string, std::string>> human_metadata;
	TinyEXIF::EXIFInfo machine_metadata;
public:
	ImageMetadata() = default;
	ImageMetadata(const QString &);
	ImageMetadata(std::unique_ptr<QIODevice> &&);
	ImageMetadata(const ImageMetadata &) = default;
	ImageMetadata &operator=(const ImageMetadata &) = default;
	ImageMetadata(ImageMetadata &&) = default;
	ImageMetadata &operator=(ImageMetadata &&) = default;
	const auto &get_human() const{
		return this->human_metadata;
	}
};

#endif
