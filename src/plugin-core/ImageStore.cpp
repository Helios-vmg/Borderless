/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "ImageStore.h"
#include <QImage>
#include <QFile>

ImageStore global_store;

Image::Image(const QString &path): alphaed(false){
	if (!QFile::exists(path))
		throw ImageOperationResult("File not found.");
	this->bitmap = QImage(path);
	if (this->bitmap.isNull())
		throw ImageOperationResult("Unknown error.");
	this->w = this->bitmap.width();
	this->h = this->bitmap.height();
}

Image::Image(int w, int h): alphaed(false){
	this->bitmap = QImage(w, h, QImage::Format_RGBA8888);
	if (this->bitmap.isNull())
		throw ImageOperationResult("Unknown error.");
	this->w = this->bitmap.width();
	this->h = this->bitmap.height();
}

Image::Image(const QImage &image): alphaed(false){
	this->bitmap = image;
	if (this->bitmap.isNull())
		throw ImageOperationResult("Unknown error.");
	this->w = this->bitmap.width();
	this->h = this->bitmap.height();
}

void Image::traverse(traversal_callback cb){
	this->to_alpha();

	auto pixels = this->bitmap.bits();
	for (int y = 0; y < this->h; y++){
		//auto scanline = pixels + this->pitch * (this->h - 1 - y);
		auto scanline = pixels + this->pitch * y;
		for (int x = 0; x < this->w; x++){
			auto pixel = scanline + x * this->stride;
			int r = pixel[0];
			int g = pixel[1];
			int b = pixel[2];
			int a = pixel[3];
			auto prev = global_store.get_current_traversal_image();
			auto prev_pixel = this->current_pixel;
			global_store.set_current_traversal_image(this);
			this->current_pixel = pixel;
			cb(r, g, b, a, x, y);
			global_store.set_current_traversal_image(prev);
			this->current_pixel = prev_pixel;
		}
	}
}

ImageTraversalIterator Image::get_iterator(){
	this->to_alpha();
	return ImageTraversalIterator(this->bitmap);
}

void Image::to_alpha(){
	if (this->alphaed)
		return;
	if (this->bitmap.format() != QImage::Format_RGBA8888)
		this->bitmap = this->bitmap.convertToFormat(QImage::Format_RGBA8888);
	this->pitch = this->bitmap.bytesPerLine();
	this->alphaed = true;
}

void Image::set_current_pixel(const pixel_t &rgba){
	for (int i = 0; i < 4; i++)
		this->current_pixel[i] = rgba[i];
}

ImageOperationResult Image::save(const QString &path, SaveOptions opt){
	ImageOperationResult ret;
	ret.success = this->bitmap.save(path, opt.format.size() ? opt.format.c_str() : nullptr, opt.compression);
	if (!ret.success)
		ret.message = "Unknown error.";
	return ret;
}

ImageOperationResult Image::get_pixel(unsigned x, unsigned y){
	if (x >= this->w || y >= this->h)
		return "Invalid coordinates.";
	this->to_alpha();
	auto pixels = this->bitmap.bits();
	auto pixel = pixels + this->pitch * y + this->stride * x;
	ImageOperationResult ret;
	ret.success = 1;
	for (int i = 0; i < 4; i++)
		ret.results[i] = pixel[i];
	return ret;
}

ImageOperationResult Image::get_dimensions(){
	ImageOperationResult ret;
	ret.success = true;
	ret.results[0] = this->w;
	ret.results[1] = this->h;
	return ret;
}

ImageOperationResult ImageStore::load(const char *path){
	return this->load(QString::fromUtf8(path));
}

ImageOperationResult ImageStore::load(const QString &path){
	decltype(this->images)::mapped_type img;
	try{
		img.reset(new Image(path));
	}catch (ImageOperationResult &ior){
		return ior;
	}
	ImageOperationResult ret;
	ret.results[0] = this->next_index++;
	this->images[ret.results[0]] = img;
	return ret;
}

ImageOperationResult ImageStore::unload(int handle){
	auto it = this->images.find(handle);
	if (it == this->images.end())
		return "Image handle doesn't exist.";
	this->images.erase(it);
	return ImageOperationResult();
}

ImageOperationResult ImageStore::save(int handle, const QString &path, SaveOptions opt){
	auto it = this->images.find(handle);
	if (it == this->images.end())
		return "Image handle doesn't exist.";
	return it->second->save(path, opt);
}

ImageOperationResult ImageStore::traverse(int handle, traversal_callback cb){
	auto it = this->images.find(handle);
	if (it == this->images.end())
		return "Image handle doesn't exist.";
	auto img = it->second;
	img->traverse(cb);
	return ImageOperationResult();
}

ImageTraversalIterator ImageStore::get_iterator(int handle){
	auto it = this->images.find(handle);
	if (it == this->images.end())
		return ImageTraversalIterator();
	return it->second->get_iterator();
}

ImageOperationResult ImageStore::allocate(int w, int h){
	if (w < 1  || h < 1 )
		return "both width and height must be at least 1.";
	decltype(this->images)::mapped_type img;
	try{
		img.reset(new Image(w, h));
	}catch (ImageOperationResult &ior){
		return ior;
	}
	ImageOperationResult ior;
	ior.results[0] = this->next_index++;
	this->images[ior.results[0]] = img;
	return ior;
}

ImageOperationResult ImageStore::get_pixel(int handle, unsigned x, unsigned y){
	auto it = this->images.find(handle);
	if (it == this->images.end())
		return "Image handle doesn't exist.";
	return it->second->get_pixel(x, y);
}

void ImageStore::set_current_pixel(const pixel_t &rgba){
	if (!this->current_traversal_image)
		return;
	this->current_traversal_image->set_current_pixel(rgba);
}

ImageOperationResult ImageStore::get_dimensions(int handle){
	auto it = this->images.find(handle);
	if (it == this->images.end())
		return "Image handle doesn't exist.";
	return it->second->get_dimensions();
}

int ImageStore::store(const QImage &image){
	int ret = this->next_index++;
	this->images[ret] = std::make_shared<Image>(image);
	return ret;
}

ImageTraversalIterator::ImageTraversalIterator(QImage &image): image(&image){
	this->pixels = image.bits();
	this->w = image.width();
	this->h = image.height();
	this->pitch = image.bytesPerLine();
	this->reset();
}

bool ImageTraversalIterator::next(){
	if (this->state == 4){
		if (this->i >= this->n)
			return false;
		this->x = this->i % this->w;
		this->y = this->i / this->w;
		this->current_pixel = this->pixels + this->i++ * this->stride;
		//this->scanline = this->pixels + this->pitch * this->y;
		return true;
	}

	switch (this->state){
		case 0:
			break;
		case 1:
			goto state_1;
		case 2:
			this->current_pixel = nullptr;
			return false;
	}

	for (; this->y < this->h; this->y++){
		this->scanline = this->pixels + this->pitch * this->y;
		this->x = 0;
		for (; this->x < this->w; this->x++){
			this->current_pixel = this->scanline + this->x * this->stride;
			this->state = 1;
			return true;
state_1:
			;
		}
	}
	this->state = 2;
	this->current_pixel = nullptr;
	return false;
}

position_info ImageTraversalIterator::get() const{
	position_info ret;
	if (!!this->current_pixel){
		memcpy(ret.rgba, this->current_pixel, 4);
		ret.x = this->x;
		ret.y = this->y;
	}
	return ret;
}

void ImageTraversalIterator::set(unsigned char rgba[4]) const{
	memcpy(this->current_pixel, rgba, 4);
}

void ImageTraversalIterator::set(unsigned char r, unsigned char g, unsigned char b, unsigned char a) const{
	this->current_pixel[0] = r;
	this->current_pixel[1] = g;
	this->current_pixel[2] = b;
	this->current_pixel[3] = a;
}

void ImageTraversalIterator::reset(){
	this->y = 0;
	if (this->pitch != this->stride * this->w)
		this->state = 0;
	else{
		this->i = 0;
		this->n = this->w * this->h;
		this->state = 4;
	}
}
