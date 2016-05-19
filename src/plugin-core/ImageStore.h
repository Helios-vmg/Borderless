/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef IMAGESTORE_H
#define IMAGESTORE_H

#include <unordered_map>
#include <memory>
#include <functional>
#include <cstdint>
#include <array>
#include <QImage>
#include "capi.h"

class Image;
class QString;

struct ImageOperationResult{
	bool success;
	int results[4];
	std::string message;
	ImageOperationResult(const char *msg = nullptr): success(!msg), message(msg ? msg : ""){}
};

enum class Trinary{
	Undefined,
	False,
	True,
};

struct SaveOptions{
	int compression;
	std::string format;
	SaveOptions(): compression(-1){}
};

typedef std::function<void(int, int, int, int, int, int)> traversal_callback;
typedef std::array<std::uint8_t, 4> pixel_t;

class ImageTraversalIterator{
	QImage *image;
	unsigned char *pixels, *scanline, *current_pixel;
	unsigned x, y, w, h, pitch, i, n;
	static const unsigned stride = 4;
	unsigned state;
public:
	ImageTraversalIterator(): image(nullptr){}
	ImageTraversalIterator(QImage &);
	bool is_null() const{
		return !this->image;
	}
	// Behaves like iterator++ != end for while and for predicate.
	bool next();
	position_info get() const;
	void set(unsigned char rgba[4]) const;
	void set(unsigned char r, unsigned char g, unsigned char b, unsigned char a) const;
	void reset();
};

class ImageStore;

class Image{
	ImageStore *owner;
	int own_handle;
	QImage bitmap;
	bool alphaed;
	int w, h;
	static const unsigned stride = 4;
	unsigned pitch;
	std::uint8_t *current_pixel;

	void to_alpha();
public:
	Image(const QString &path, ImageStore &owner, int handle);
	Image(int w, int h, ImageStore &owner, int handle);
	Image(const QImage &, ImageStore &owner, int handle);
	void traverse(traversal_callback cb);
	void set_current_pixel(const pixel_t &rgba);
	ImageOperationResult save(const QString &path, SaveOptions opt);
	ImageOperationResult get_pixel(unsigned x, unsigned y);
	ImageOperationResult get_dimensions();
	QImage get_bitmap() const{
		return this->bitmap;
	}
	ImageTraversalIterator get_iterator();
	ImageStore *get_owner() const{
		return this->owner;
	}
	int get_handle() const{
		return this->own_handle;
	}
};

class ImageStore{
	std::unordered_map<int, std::shared_ptr<Image>> images;
	Image *current_traversal_image;
	int next_index;
public:
	ImageStore(): current_traversal_image(nullptr){}
	ImageOperationResult load(const char *path);
	ImageOperationResult load(const QString &path);
	int store(const QImage &);
	ImageOperationResult unload(int handle);
	ImageOperationResult save(int handle, const QString &path, SaveOptions opt);
	ImageOperationResult traverse(int handle, traversal_callback cb);
	ImageOperationResult allocate(int w, int h);
	ImageOperationResult get_pixel(int handle, unsigned x, unsigned y);
	void set_current_pixel(const pixel_t &rgba);
	ImageOperationResult get_dimensions(int handle);

	Image *get_current_traversal_image(){
		return this->current_traversal_image;
	}
	void set_current_traversal_image(Image *image){
		this->current_traversal_image = image;
	}
	std::shared_ptr<Image> get_image(int img) const{
		auto i = this->images.find(img);
		if (i == this->images.end())
			return std::shared_ptr<Image>();
		return i->second;
	}
	void clear(){
		this->images.clear();
		this->next_index = 0;
	}
	ImageTraversalIterator get_iterator(int handle);
};

#endif
