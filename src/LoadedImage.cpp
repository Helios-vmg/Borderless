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

#include "LoadedImage.h"
#include <QImage>
#include <QtConcurrent/QtConcurrentRun>
#include <QLabel>

LoadedImage::LoadedImage(const QString &path){
	QImage img(path);
	if (this->null = img.isNull())
		return;
	this->compute_average_color(img);
	this->image = QtConcurrent::run([](QImage img){ return QPixmap::fromImage(img); }, img);
	this->size = img.size();
	this->alpha = img.hasAlphaChannel();
}

LoadedImage::~LoadedImage(){
	this->background_color.cancel();
}

QColor get_average_color(QImage src){
	if (src.depth() < 32)
		src = src.convertToFormat(QImage::Format_ARGB32);
	quint64 avg[3] = {0};
	unsigned pixel_count=0;
	for (auto y = src.height() * 0; y < src.height(); y++){
		const uchar *p = src.constScanLine(y);
		for (auto x = src.width() * 0; x < src.width(); x++){
			avg[0] += quint64(p[2]) * quint64(p[3]) / 255;
			avg[1] += quint64(p[1]) * quint64(p[3]) / 255;
			avg[2] += quint64(p[0]) * quint64(p[3]) / 255;
			p += 4;
			pixel_count++;
		}
	}
	for (int a = 0; a < 3; a++)
		avg[a] /= pixel_count;
	return QColor(avg[0], avg[1], avg[2]);
}

QColor background_color_parallel_function(QImage img){
	QColor avg = get_average_color(img),
		negative = avg,
		background;
	negative.setRedF(1 - negative.redF());
	negative.setGreenF(1 - negative.greenF());
	negative.setBlueF(1 - negative.blueF());
	if (negative.saturationF() <= .05 && negative.valueF() >= .45 && negative.valueF() <= .55)
		background = Qt::white;
	else
		background = negative;
	return background;
}

void LoadedImage::compute_average_color(QImage img){
	this->background_color = QtConcurrent::run(background_color_parallel_function, img);
}

void LoadedImage::assign_to_QLabel(QLabel &label){
	label.setPixmap(this->image);
}

LoadedAnimation::LoadedAnimation(const QString &path): animation(path){
	this->null = !this->animation.isValid();
	if (!this->null){
		bool ok = this->animation.jumpToNextFrame();
		this->size = this->animation.currentPixmap().size();
		this->alpha = true;
	}
}

void LoadedAnimation::assign_to_QLabel(QLabel &label){
	label.setMovie(&this->animation);
	this->animation.start();
}

std::shared_ptr<LoadedGraphics> LoadedGraphics::create(const QString &path){
	if (path.endsWith(".gif", Qt::CaseInsensitive))
		return std::shared_ptr<LoadedGraphics>(new LoadedAnimation(path));
	return std::shared_ptr<LoadedGraphics>(new LoadedImage(path));
}
