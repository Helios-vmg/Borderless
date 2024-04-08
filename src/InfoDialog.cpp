/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "InfoDialog.h"
#include <sstream>

static QString to_string(std::uint64_t n){
	std::stringstream stream;
	stream << n;
	return QString::fromStdString(stream.str());
}

InfoDialog::InfoDialog(QWidget &parent, const ImageMetadata &metadata)
		: QDialog(&parent)
		, ui(std::make_unique<Ui::InfoDialog>())
{
	this->ui->setupUi(this);
	this->ui->filename_box->setText(metadata.get_filename());
	this->ui->path_box->setText(metadata.get_full_path());
	this->initialize_size(metadata.get_size());
	this->ui->colors_box->setText(to_string(metadata.get_color_count().first) + " (counted in " + to_string(metadata.get_color_count().second) + " ms)");
	this->ui->filesize_box->setText(to_string(metadata.get_filesize()));
	this->initialize_exif(metadata);

	connect(this->ui->close_btn, SIGNAL(clicked(bool)), this, SLOT(close()));
}

template <typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type
gcd(T a, T b){
	bool invert = false;
	if (a < 0){
		a = -a;
		invert = !invert;
	}
	if (b < 0){
		b = -b;
		invert = !invert;
	}
	while (b){
		auto t = b;
		b = a % b;
		a = t;
	}
	return !invert ? a : -a;
}

void InfoDialog::initialize_size(const std::pair<QSize, int> &sizes){
	if (!sizes.first.isValid()){
		this->ui->dimensions_box->setText("unknown");
		return;
	}
	std::stringstream stream;
	auto w = sizes.first.width();
	auto h = sizes.first.height();
	stream << w << "x" << h << "x" << sizes.second << " (";
	auto divisor = gcd(w, h);
	w /= divisor;
	h /= divisor;
	stream << w << ":" << h << ", " << (double)w / (double)h << ")";
	this->ui->dimensions_box->setText(QString::fromStdString(stream.str()));
}

void InfoDialog::initialize_exif(const ImageMetadata &metadata){
	auto &exif_box = *this->ui->exif_box;
	exif_box.setColumnCount(2);
	exif_box.setHeaderLabels(QStringList{
		"Field",
		"Value",
		});
	for (auto &[key, value] : metadata.get_human()){
		exif_box.insertTopLevelItem(0, new QTreeWidgetItem(&exif_box, QStringList{
			QString::fromStdString(key),
			QString::fromStdString(value),
			}));
	}
	exif_box.resizeColumnToContents(0);
	exif_box.resizeColumnToContents(1);
	exif_box.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	auto size = exif_box.size();
	size.setWidth(exif_box.columnWidth(0) + exif_box.columnWidth(1) + 32);
	exif_box.setMinimumSize(size);
}
