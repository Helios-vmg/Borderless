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

#ifndef LOADEDIMAGE_H
#define LOADEDIMAGE_H

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
	static std::shared_ptr<LoadedGraphics> create(const QString &path);
};

class LoadedImage : public LoadedGraphics{
	QFuture<QPixmap> image;
	QFuture<QColor> background_color;

	void compute_average_color(QImage);
public:
	LoadedImage(const QString &path);
	~LoadedImage();
	QColor get_background_color() override{
		return this->background_color.result();
	}
	bool is_animation() const override{
		return false;
	}
	void assign_to_QLabel(QLabel &) override;
};

class LoadedAnimation : public LoadedGraphics{
	QMovie animation;

public:
	LoadedAnimation(const QString &path);
	QColor get_background_color() override{
		return QColor(0, 0, 0, 0);
	}
	bool is_animation() const override{
		return true;
	}
	void assign_to_QLabel(QLabel &) override;
};

#endif // LOADEDIMAGE_H
