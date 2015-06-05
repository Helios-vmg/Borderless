/*

Copyright (c) 2015, Helios
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef IMAGEVIEWPORT_H
#define IMAGEVIEWPORT_H

#include <QLabel>
#include <QImage>
#include <QMatrix>
#include "Quadrangular.h"
#include "Settings.h"

class LoadedGraphics;

class ImageViewport : public QLabel
{
	Q_OBJECT
	QMatrix transform;
	double zoom;
	QSize image_size;
	bool prevent_recursion;

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

	void save_state(SettingsTree &tree);
	void load_state(const SettingsTree &tree);

	void paintEvent(QPaintEvent *) override;
	void set_image(LoadedGraphics &li);

signals:
	void transform_updated();

public slots:

};

#endif // IMAGEVIEWPORT_H
