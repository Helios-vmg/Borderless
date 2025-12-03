/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef LOADEDIMAGE_H
#define LOADEDIMAGE_H

#include "config.hpp"
#include "ImageViewerApplication.h"
#include "resvg.hpp"
#include "exif.h"
#include <QString>
#include <QPixmap>
#include <QMovie>
#include <QFuture>
#include <memory>

class QLabel;

class ImageWithMetadata{
	QImage image;
	ImageMetadata meta;
public:
	ImageWithMetadata(QImage &&, const QString &);
	ImageWithMetadata(QImage &&, const QString &, const std::shared_ptr<ProtocolModule::Client> &, std::unique_ptr<QIODevice> &&);
	ImageWithMetadata(const ImageWithMetadata &) = delete;
	ImageWithMetadata &operator=(const ImageWithMetadata &) = delete;
	ImageWithMetadata(ImageWithMetadata &&) = default;
	ImageWithMetadata &operator=(ImageWithMetadata &&) = default;
	QImage get_image(){
		return this->image;
	}
	ImageMetadata &get_metadata(){
		return this->meta;
	}
	const ImageMetadata &get_metadata() const{
		return this->meta;
	}
};

class MovieWithMetadata{
	std::unique_ptr<QIODevice> device;
	std::unique_ptr<QMovie> movie;
	ImageMetadata meta;
public:
	MovieWithMetadata(std::unique_ptr<QIODevice> &&, std::unique_ptr<QMovie> &&, const QString &);
	MovieWithMetadata(const MovieWithMetadata &) = delete;
	MovieWithMetadata &operator=(const MovieWithMetadata &) = delete;
	MovieWithMetadata(MovieWithMetadata &&) = default;
	MovieWithMetadata &operator=(MovieWithMetadata &&) = default;
	auto &&get_device(){
		return std::move(this->device);
	}
	auto &&get_movie(){
		return std::move(this->movie);
	}
	ImageMetadata &get_metadata(){
		return this->meta;
	}
	const ImageMetadata &get_metadata() const{
		return this->meta;
	}
};

class LoadedGraphics{
protected:
	QSize size;
	bool alpha;
	bool null;
	ImageMetadata info;
	std::optional<std::string> hash;

	virtual void compute_hash(){}
public:
	virtual ~LoadedGraphics(){}
	virtual bool is_animation() const = 0;
	virtual bool is_vector() const = 0;
	virtual QColor get_background_color() = 0;
	QSize get_size() const{
		return this->size;
	}
	bool is_null() const{
		return this->null;
	}
	bool has_alpha() const{
		return this->alpha;
	}
	virtual void assign_to_QLabel(QLabel &) = 0;
	virtual QImage get_QImage() const = 0;
	virtual const ImageMetadata *get_metadata() const{
		return nullptr;
	}
	virtual ImageMetadata *get_metadata(){
		return nullptr;
	}
	virtual QImage scale(double zoom) = 0;
	class create_result{
	public:
		std::shared_ptr<LoadedGraphics> loaded_graphics;
		bool permanent_error = false;
		bool retry_in_main = false;

		create_result() = default;
		create_result(std::shared_ptr<LoadedGraphics> loaded_graphics, bool permanent_error, bool retry_in_main = false)
			: loaded_graphics(loaded_graphics)
			, permanent_error(permanent_error)
			, retry_in_main(retry_in_main)
		{}
		create_result(const create_result &) = default;
		create_result &operator=(const create_result &) = default;
		create_result(create_result &&) = default;
		create_result &operator=(create_result &&) = default;
	};
	static create_result create(ImageViewerApplication &app, const QString &path, bool calling_from_main, bool will_need_hash);
	std::string get_hash();
};

class RasterGraphics : public LoadedGraphics{
public:
	virtual ~RasterGraphics(){}
	bool is_vector() const override{
		return false;
	}
	ImageMetadata *get_metadata() override{
		return &this->info;
	}
};

class LoadedImage : public RasterGraphics{
	QFuture<QPixmap> image;
	QFuture<QColor> background_color;

	void compute_average_color(QImage);
	void compute_hash() override;
public:
	LoadedImage(ImageViewerApplication &app, std::unique_ptr<QIODevice> &&dev, const QString &path, bool will_need_hash);
	LoadedImage(const QImage &image);
	virtual ~LoadedImage();
	QColor get_background_color() override{
		return this->background_color.result();
	}
	bool is_animation() const override{
		return false;
	}
	void assign_to_QLabel(QLabel &) override;
	QImage get_QImage() const override;
	const ImageMetadata *get_metadata() const override{
		return &this->info;
	}
	ImageMetadata *get_metadata() override{
		return &this->info;
	}
	QImage scale(double zoom) override;
};

class LoadedAnimation : public RasterGraphics{
	std::unique_ptr<QIODevice> device;
	std::unique_ptr<QMovie> animation;

public:
	LoadedAnimation(ImageViewerApplication &app, std::unique_ptr<QIODevice> &&dev, const QString &path, bool will_need_hash);
	QColor get_background_color() override{
		return QColor(0, 0, 0, 0);
	}
	bool is_animation() const override{
		return true;
	}
	void assign_to_QLabel(QLabel &) override;
	QImage get_QImage() const override;
	std::unique_ptr<QIODevice> get_device(){
		return std::move(this->device);
	}
	QImage scale(double zoom) override{
		return {};
	}
};

class VectorGraphics : public LoadedGraphics{
public:
	virtual ~VectorGraphics(){}
	bool is_vector() const override{
		return true;
	}
	bool is_animation() const override{
		return false;
	}
};

#ifdef ENABLE_SVG

class SvgImage : public VectorGraphics{
	QFuture<QImage> image;
	QFuture<QPixmap> pixmap;
	QFuture<QColor> background_color;
	ReSvgRenderTree tree;
	QByteArray raw_data;

	void compute_hash() override;
public:
	SvgImage(ImageViewerApplication &app, std::unique_ptr<QIODevice> &&dev, const QString &path, bool will_need_hash);
	~SvgImage() override;
	QColor get_background_color() override{
		return this->background_color.result();
	}
	void assign_to_QLabel(QLabel &) override;
	QImage get_QImage() const override;
	const ImageMetadata * get_metadata() const override;
	ImageMetadata * get_metadata() override;
	QImage scale(double zoom) override;
};

#endif

#endif // LOADEDIMAGE_H
