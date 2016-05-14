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

class Image;
class QString;

struct ImageOperationResult{
	bool success;
	int results[4];
	std::string message;
	ImageOperationResult(const char *msg = nullptr): success(!msg), message(msg){}
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

class ImageStore{
	std::unordered_map<int, std::shared_ptr<Image>> images;
	Image *current_traversal_image;
	int next_index;
public:
	ImageStore(): current_traversal_image(nullptr){}
	ImageOperationResult load(const char *path);
	ImageOperationResult load(const QString &path);
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
};

extern ImageStore global_store;

#endif
