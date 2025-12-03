/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "LoadedImage.h"
#include "DirectoryListing.h"
#include "ProtocolModule.h"
#include <QImage>
#include <QtConcurrent/QtConcurrentRun>
#include <QLabel>
#include <tuple>
#include <QFile>
#include <QCryptographicHash>

extern const char *supported_extensions[];

std::string hash_image(QImage src);

LoadedImage::LoadedImage(ImageViewerApplication &app, std::unique_ptr<QIODevice> &&dev, const QString &path, bool will_need_hash){
	auto image_with_metadata = app.load_image(std::move(dev), path);
	auto img = image_with_metadata.get_image();
	if ((this->null = img.isNull()))
		return;
	auto for_processing = img;
	if (will_need_hash)
		for_processing = for_processing.convertToFormat(QImage::Format_RGBA8888);
	this->compute_average_color(for_processing);
	this->image = QtConcurrent::run([](QImage img){ return QPixmap::fromImage(img); }, img);
	this->size = img.size();
	this->alpha = img.hasAlphaChannel();
	this->info = std::move(image_with_metadata.get_metadata());
	if (will_need_hash)
		this->hash = hash_image(for_processing);
}

LoadedImage::LoadedImage(const QImage &image){
	this->compute_average_color(image);
	this->image = QtConcurrent::run([](QImage img){ return QPixmap::fromImage(img); }, image);
	this->size = image.size();
	this->alpha = image.hasAlphaChannel();
}

LoadedImage::~LoadedImage(){
	this->background_color.cancel();
}

std::string hash_image(QImage src){
	quint64 avg[3] = {0};
	QCryptographicHash hash(QCryptographicHash::Md5);
	auto pitch = src.width() * 4;
	for (auto y = src.height() * 0; y < src.height(); y++)
		hash.addData((const char *)src.constScanLine(y), pitch);
	return hash.result().toHex().toStdString();
}

QColor get_average_color(QImage src){
	if (src.depth() < 32)
		src = src.convertToFormat(QImage::Format_RGBA8888);
	quint64 avg[3] = {0};
	unsigned pixel_count = 0;
	for (auto y = src.height() * 0; y < src.height(); y++){
		auto p = src.constScanLine(y);
		for (auto x = src.width() * 0; x < src.width(); x++){
			avg[0] += quint64(p[0]) * quint64(p[3]);
			avg[1] += quint64(p[1]) * quint64(p[3]);
			avg[2] += quint64(p[2]) * quint64(p[3]);
			p += 4;
			pixel_count++;
		}
	}
	for (int a = 0; a < 3; a++)
		avg[a] /= pixel_count * 255;
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
	label.setPixmap(this->image.result());
}

QImage LoadedImage::get_QImage() const{
	return this->image.result().toImage();
}

QImage LoadedImage::scale(double zoom){
	auto image = this->get_QImage();
	auto size = image.size() * zoom;
	return image.scaled(size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
}

LoadedAnimation::LoadedAnimation(ImageViewerApplication &app, std::unique_ptr<QIODevice> &&dev, const QString &path, bool will_need_hash){
	auto animation = app.load_animation(std::move(dev), path);
	this->animation = animation.get_movie();
	this->device = animation.get_device();
	this->null = !this->animation || !this->animation->isValid();
	if (this->null)
		return;
	this->size = animation.get_metadata().get_size().first;
	this->alpha = true;
	this->info = std::move(animation.get_metadata());
	this->hash.emplace();
}

void LoadedAnimation::assign_to_QLabel(QLabel &label){
	label.setMovie(this->animation.get());
	this->animation->start();
}

QImage LoadedAnimation::get_QImage() const{
	return this->animation->currentImage();
}

LoadedGraphics::create_result LoadedGraphics::create(ImageViewerApplication &app, const QString &path, bool calling_from_main, bool will_need_hash){
	auto [dev, permanent_error] = app.open_file(path);
	if (app.is_svg(path))
#ifdef ENABLE_SVG
		return {
			std::make_unique<SvgImage>(app, std::move(dev), path, will_need_hash),
			permanent_error
		};
#else
		return { nullptr, true };
#endif
	auto is_animation = app.is_animation(path);
	if (is_animation){
		if (!calling_from_main)
			return { {}, false, true };
		auto animation = std::make_unique<LoadedAnimation>(app, std::move(dev), path, will_need_hash);
		if (!animation->is_null())
			return { std::move(animation), permanent_error };
		dev = animation->get_device();
	}
	if (dev)
		dev->reset();
	return {
		std::make_unique<LoadedImage>(app, std::move(dev), path, will_need_hash),
		permanent_error
	};
}

#ifdef ENABLE_SVG

QByteArray read_file(const std::unique_ptr<QIODevice> &dev, const QString &path){
	if (dev)
		return dev->readAll();
	QFile file(path);
	file.open(QIODeviceBase::ExistingOnly | QIODeviceBase::ReadOnly);
	return file.readAll();
}

class HorribleThing{
public:
	std::unique_ptr<QIODevice> dev;
	HorribleThing(std::unique_ptr<QIODevice> &&dev): dev(std::move(dev)){}
};

SvgImage::SvgImage(ImageViewerApplication &app, std::unique_ptr<QIODevice> &&dev, const QString &path, bool will_need_hash){
	this->null = true;
	this->alpha = true;
	this->raw_data = read_file(dev, path);
	if (will_need_hash)
		this->SvgImage::compute_hash();
	auto [error, tree] = ReSvgRenderTree::create_from_data(this->raw_data.data(), this->raw_data.size(), {});
	if (error != ReSvgRenderTree::Error::NoError)
		return;
	this->tree = std::move(tree);
	this->null = this->tree.is_empty();
	if (this->null)
		return;
	auto [w, h] = this->tree.get_size_int();
	this->size = { w, h };
	auto horrible = std::make_unique<HorribleThing>(std::move(dev));
	this->image = QtConcurrent::run([this, horrible = std::move(horrible), path](){
		QImage dst(this->size, QImage::Format_RGBA8888_Premultiplied);
		memset(dst.bits(), 0, this->size.width() * this->size.height() * 4);
		this->tree.render(dst.bits());
		auto dev = std::move(horrible->dev);
		this->info = ImageMetadata::create_from_vector(dst, std::move(dev), path);
		return dst;
	});
	this->pixmap = QtConcurrent::run([this](){
		return QPixmap::fromImage(this->image.result());
	});
	this->background_color = QtConcurrent::run([this](){
		return background_color_parallel_function(this->image.result());
	});
}

SvgImage::~SvgImage(){
	if (this->image.isStarted())
		this->image.waitForFinished();
	if (this->pixmap.isStarted())
		this->pixmap.waitForFinished();
	if (this->background_color.isStarted())
		this->background_color.waitForFinished();
}

void SvgImage::assign_to_QLabel(QLabel &label){
	label.setPixmap(this->pixmap.result());
}

QImage SvgImage::get_QImage() const{
	return this->image.result();
}

const ImageMetadata *SvgImage::get_metadata() const{
	//Wait for task to complete.
	(void)this->image.result();
	return &this->info;
}

ImageMetadata *SvgImage::get_metadata(){
	//Wait for task to complete.
	(void)this->image.result();
	return &this->info;
}

QImage SvgImage::scale(double zoom){
	auto [w, h] = this->tree.get_size_int();
	w = (int)floor((double)w * zoom + 0.5);
	h = (int)floor((double)h * zoom + 0.5);
	QImage ret(QSize(w, h), QImage::Format_RGBA8888_Premultiplied);
	memset(ret.bits(), 0, w * h * 4);
	this->tree.render(ret.bits(), w, h, zoom);
	return ret;
}

#endif

ImageWithMetadata::ImageWithMetadata(QImage &&image, const QString &path)
	: image(std::move(image))
{
	if (!this->image.isNull())
		this->meta = ImageMetadata::create_from_still(this->image, path);
}

ImageWithMetadata::ImageWithMetadata(QImage &&image, const QString &path, const std::shared_ptr<ProtocolModule::Client> &client, std::unique_ptr<QIODevice> &&dev)
	: image(std::move(image))
{
	if (!this->image.isNull())
		this->meta = ImageMetadata::create_from_still(this->image, path, client, std::move(dev));
}

MovieWithMetadata::MovieWithMetadata(std::unique_ptr<QIODevice> &&device, std::unique_ptr<QMovie> &&movie, const QString &path)
	: device(std::move(device))
	, movie(std::move(movie))
{
	if (!this->movie)
		return;
	auto dev = dynamic_cast<ProtocolModule::Stream *>(this->device.get());
	if (!dev)
		this->meta = ImageMetadata::create_from_animation(*this->movie, path);
	else{
		auto client = dev->get_module()->create_client();
		this->meta = ImageMetadata::create_from_animation(*this->movie, path, std::move(client), *this->device);
	}
}

std::string LoadedGraphics::get_hash(){
	if (!this->hash)
		this->compute_hash();
	return *this->hash;
}

void LoadedImage::compute_hash(){
	this->hash = hash_image(this->get_QImage().convertToFormat(QImage::Format_RGBA8888));
}

void SvgImage::compute_hash(){
	this->hash = QCryptographicHash::hash(this->raw_data, QCryptographicHash::Md5).toHex().toStdString();
}
