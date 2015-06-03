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

#ifndef QUADRANGULAR_H
#define QUADRANGULAR_H

#include <QSize>
#include <QRect>
#include <QPoint>
#include <QMatrix>
#include "Misc.h"

class Quadrangular{
	QPointF corners[4];
	void set_size(const QSize &size){
		this->corners[0] = QPointF(0, 0);
		this->corners[1] = QPointF(size.width(), 0);
		this->corners[2] = QPointF(size.width(), size.height());
		this->corners[3] = QPointF(0, size.height());
		//this->move(-to_QPointF(size));
	}
	void move(const QPointF &p){
		for (auto &c : this->corners)
			c += p;
	}
public:
	Quadrangular(const QSize &size){
		this->set_size(size);
	}
	Quadrangular(const QRect &rect){
		this->set_size(rect.size());
		this->move(rect.topLeft());
	}
	const Quadrangular &operator*=(const QMatrix &transform){
		for (auto &p : this->corners)
			p = transform.map(p);
		return *this;
	}
	template <typename T>
	Quadrangular operator*(const T &x) const{
		auto ret = *this;
		ret *= x;
		return ret;
	}
	const Quadrangular &operator+=(const QPointF &shift){
		for (auto &p : this->corners)
			p += shift;
		return *this;
	}
	template <typename T>
	Quadrangular operator+(const T &x) const{
		auto ret = *this;
		ret += x;
		return ret;
	}
	const Quadrangular &operator-=(const QPointF &shift){
		return *this += -shift;
	}
	template <typename T>
	Quadrangular operator-(const T &x) const{
		auto ret = *this;
		ret -= x;
		return ret;
	}
	QRectF get_bounding_box() const{
		auto minx = this->corners[0].x();
		auto miny = this->corners[0].y();
		auto maxx = this->corners[0].x();
		auto maxy = this->corners[0].y();
		for (auto &p : this->corners){
			set_min(minx, p.x());
			set_min(miny, p.y());
			set_max(maxx, p.x());
			set_max(maxy, p.y());
		}
		return QRectF(minx, miny, maxx - minx, maxy - miny);
	}
	QPointF move_to_origin(){
		auto ret = -this->get_bounding_box().topLeft();
		*this += ret;
		return ret;
	}
};

#endif
