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

#include "ImageViewport.h"
#include <QPaintEvent>
#include <QPainter>

DEFINE_SETTING_STRING(transform);

ImageViewport::ImageViewport(QWidget *parent) :
		QLabel(parent),
		zoom(1){
	this->transform.reset();
	this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void ImageViewport::rotate(double delta_theta){
	this->transform *= QMatrix().rotate(delta_theta);
	this->transform_changed();
}

void ImageViewport::flip(bool hor){
	auto x = hor ? -1.0 : 1.0;
	auto y = -x;
	this->transform *= QMatrix().scale(x, y);
	this->transform_changed();
}

template <typename T>
typename std::enable_if<std::is_enum<T>::value, T>::type or_flags(const T &a, const T &b){
	return (T)((unsigned)a | (unsigned)b);
}

QMatrix translate(const QMatrix &m, const QPointF &offset){
	return QMatrix(m.m11(), m.m12(), m.m21(), m.m22(), m.dx() + offset.x(), m.dy() + offset.y());
}

void ImageViewport::paintEvent(QPaintEvent *ev){
	QRectF dst = ev->rect();

	QPainter painter(this);
	painter.setRenderHint(or_flags(QPainter::SmoothPixmapTransform, QPainter::Antialiasing));
	//painter.setClipRect(dst);

	auto transform = this->get_final_transform();
	auto src_quad = this->compute_quad();
	auto offset = src_quad.move_to_origin();
	transform = translate(transform, offset);
	painter.setMatrix(transform);
	painter.drawPixmap(this->pixmap()->rect(), *this->pixmap());
}

void ImageViewport::save_state(SettingsTree &tree){
	tree.set_value(transform_setting, this->transform);
}

void ImageViewport::load_state(const SettingsTree &tree){
	tree.get_value(this->transform, transform_setting);
	this->transform_changed();
}

void ImageViewport::transform_changed(){
	this->resize(this->get_size());
	emit this->transform_updated();
}

void ImageViewport::set_transform(const QMatrix &m){
	this->transform = m;
	this->transform_changed();
}
