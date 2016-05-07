/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef IMAGEVIEWPORT_H
#define IMAGEVIEWPORT_H

#include <QLabel>
#include <QImage>
#include <QMatrix>
#include "Quadrangular.h"
#include "serialization/settings.generated.h"

class LoadedGraphics;

class ImageViewport : public QLabel
{
	Q_OBJECT
	QMatrix transform;
	double zoom;
	QSize image_size;

	QMatrix get_final_transform() const{
		auto ret = this->transform;
		ret.scale(this->zoom, this->zoom);
		return ret;
	}
	Quadrangular compute_quad(const QSize &size) const{
		return Quadrangular(size) * this->get_final_transform();
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
	void update_size(){
		this->resize(this->get_size());
	}
	void compute_size(QSize &size) const{
		size = this->compute_quad(size).get_bounding_box().size().toSize();
	}
	void compute_size_no_zoom(QSize &size) const{
		size = this->compute_quad_no_zoom(size).get_bounding_box().size().toSize();
	}
	QSize get_size() const{
		auto ret = this->image_size;
		this->compute_size(ret);
		return ret;
	}
	QRect get_geometry() const{
		auto ret = this->geometry();
		ret.setSize(this->get_size());
		return ret;
	}
	QMatrix get_transform(){
		return this->transform;
	}
	void set_transform(const QMatrix &);
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
