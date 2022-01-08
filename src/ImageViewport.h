/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef IMAGEVIEWPORT_H
#define IMAGEVIEWPORT_H

#include "Quadrangular.h"
#include "Settings.h"
#include <QLabel>
#include <QImage>
#include <QTransform>

class LoadedGraphics;

class ImageViewport : public QLabel
{
	Q_OBJECT
	QTransform transform;
	double zoom;
	QSize image_size;

	QTransform get_final_transform() const{
		auto ret = this->transform;
		ret.scale(this->zoom, this->zoom);
		return ret;
	}
	QTransform get_final_transform(double zoom) const{
		auto ret = this->transform;
		ret.scale(zoom, zoom);
		return ret;
	}
	Quadrangular compute_quad(const QSize &size) const{
		return Quadrangular(size) * this->get_final_transform();
	}
	Quadrangular compute_quad(const QSize &size, double zoom) const{
		return Quadrangular(size) * this->get_final_transform(zoom);
	}
	Quadrangular compute_quad_no_zoom(const QSize &size) const{
		return Quadrangular(size) * this->transform;
	}
	Quadrangular compute_quad() const{
		return this->compute_quad(this->image_size);
	}
	void transform_changed();
public:
	explicit ImageViewport(QWidget *parent = 0);
	void reset_transform(){
		this->transform.reset();
	}
	void set_zoom(double x){
		this->zoom = x;
	}
	void rotate(double delta_theta);
	void override_rotation(double delta_theta);
	void update_size(){
		this->resize(this->get_size());
	}
	QSize compute_size(const QSize &size) const{
		return this->compute_quad(size).get_bounding_box().size().toSize();
	}
	QSize compute_size_no_zoom(const QSize &size) const{
		return this->compute_quad_no_zoom(size).get_bounding_box().size().toSize();
	}
	QSize compute_size(const QSize &size, double zoom) const{
		return this->compute_quad(size, zoom).get_bounding_box().size().toSize();
	}
	QSize get_size() const{
		if (!this->pixmap() && !this->movie())
			return QSize(800, 600);
		return this->compute_size(this->image_size);
	}
	QRect get_geometry() const{
		auto ret = this->geometry();
		ret.setSize(this->get_size());
		return ret;
	}
	QTransform get_transform(){
		return this->transform;
	}
	void set_transform(const QTransform &);
	void flip(bool hor);

	void save_state(WindowState &) const;
	void load_state(const WindowState &);

	void paintEvent(QPaintEvent *) override;
	void set_image(LoadedGraphics &li);

signals:
	void transform_updated();

public slots:

};

#endif // IMAGEVIEWPORT_H
