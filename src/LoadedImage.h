/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef LOADEDIMAGE_H
#define LOADEDIMAGE_H

#include "ImageViewerApplication.h"
#include <QString>
#include <QPixmap>
#include <QMovie>
#include <QFuture>
#include <memory>

class QLabel;

class LoadedGraphics{
protected:
	QSize size;
	bool alpha;
	bool null;
public:
	virtual ~LoadedGraphics(){}
	virtual bool is_animation() const = 0;
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
	static std::shared_ptr<LoadedGraphics> create(ImageViewerApplication &app, const QString &path);
};

class LoadedImage : public LoadedGraphics{
	QFuture<QPixmap> image;
	QFuture<QColor> background_color;

	void compute_average_color(QImage);
public:
	LoadedImage(ImageViewerApplication &app, std::unique_ptr<QIODevice> &&dev, const QString &path);
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
};

class LoadedAnimation : public LoadedGraphics{
	std::unique_ptr<QIODevice> device;
	std::unique_ptr<QMovie> animation;

public:
	LoadedAnimation(ImageViewerApplication &app, std::unique_ptr<QIODevice> &&dev, const QString &path);
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
};

#endif // LOADEDIMAGE_H
