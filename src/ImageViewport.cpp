/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "ImageViewport.h"
#include "LoadedImage.h"
#include <QPaintEvent>
#include <QPainter>

ImageViewport::ImageViewport(QWidget *parent) :
		QLabel(parent),
		zoom(1){
	this->transform.reset();
	this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void ImageViewport::rotate(double delta_theta){
	this->transform *= QTransform().rotate(delta_theta);
	this->transform_changed();
}

void ImageViewport::override_rotation(double delta_theta){
	this->transform = QTransform().rotate(delta_theta);
	this->transform_changed();
}

void ImageViewport::flip(bool hor){
	auto x = hor ? -1.0 : 1.0;
	auto y = -x;
	this->transform *= QTransform().scale(x, y);
	this->transform_changed();
}

template <typename T>
typename std::enable_if<std::is_enum<T>::value, T>::type or_flags(const T &a, const T &b){
	return (T)((unsigned)a | (unsigned)b);
}

QTransform translate(const QTransform &m, const QPointF &offset){
	return QTransform(m.m11(), m.m12(), m.m21(), m.m22(), m.dx() + offset.x(), m.dy() + offset.y());
}

void ImageViewport::paintEvent(QPaintEvent *){
	QPainter painter(this);
	if (!this->pixmap() && !this->movie()){
		painter.setBrush(QBrush(Qt::white));
		auto font = painter.font();
		font.setPixelSize(48);
		painter.setFont(font);
		QRect rect(0, 0, 800, 600);
		painter.fillRect(rect, Qt::black);
		painter.drawText(rect, "No image");
		return;
	}

	painter.setRenderHint(or_flags(QPainter::SmoothPixmapTransform, QPainter::Antialiasing));
	painter.setClipping(false);

	auto transform = this->get_final_transform();
	auto src_quad = this->compute_quad();
	auto offset = src_quad.move_to_origin();
	transform = translate(transform, offset);
	painter.setTransform(transform);
	if (!!this->pixmap())
		painter.drawPixmap(QRect(QPoint(0, 0), this->image_size), this->pixmap());
	else
		painter.drawPixmap(QRect(QPoint(0, 0), this->image_size), this->movie()->currentPixmap());
}

void ImageViewport::save_state(WindowState &state) const{
	state.set_transform(this->transform);
}

void ImageViewport::load_state(const WindowState &state){
	this->transform = state.get_transform();
	this->transform_changed();
}

void ImageViewport::transform_changed(){
	this->resize(this->get_size());
	emit this->transform_updated();
}

void ImageViewport::set_transform(const QTransform &m){
	this->transform = m;
	this->transform_changed();
}

void ImageViewport::set_image(LoadedGraphics &li){
	this->image_size = li.get_size();
	li.assign_to_QLabel(*this);
}
