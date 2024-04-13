/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "InfoDialog.h"
#include <sstream>
#include <QLocale>

static QString to_string(std::uint64_t n){
	std::stringstream stream;
	stream << n;
	return QString::fromStdString(stream.str());
}

QString format_size(std::uint64_t size){
	std::array<char, 32> ret;
	int ret_size = 0;

	int prefix = 0;
	while (size > 1 << 30){
		size /= 1024;
		prefix++;
	}
	auto dsize = (double)size;
	while (dsize >= 1024)
	{
		dsize /= 1024;
		prefix++;
	}
	dsize = floor(dsize * 10);

	{
		auto integer_part = (int)(dsize / 10);
		bool written = false;
		for (int power = 1000; power; power /= 10){
			if (written || integer_part >= power){
				ret[ret_size++] = '0' + integer_part / power;
				integer_part %= power;
				written = true;
			}
		}
		if (!written)
			ret[ret_size++] = '0';
	}

	size = (int)dsize % 10;
	if (size){
		ret[ret_size++] = '.';
		ret[ret_size++] = '0' + (int)size;
	}
	ret[ret_size++] = ' ';

	static const char *const size_prefixes[] = {
		"",
		"Ki",
		"Mi",
		"Gi",
		"Ti",
		"Pi",
		"Ei",
		"Zi",
		"Yi",
		"Xi",
		"Wi",
		"Vi",
		"Ui",
	};

	for (auto p = size_prefixes[prefix]; *p; p++)
		ret[ret_size++] = *p;
	ret[ret_size++] = 'B';

	ret[ret_size] = 0;
	return QString::fromUtf8(ret.data());
}

InfoDialog::InfoDialog(QWidget &parent, ImageViewerApplication &app, const ImageMetadata &metadata)
		: QDialog(&parent)
		, ui(std::make_unique<Ui::InfoDialog>())
		, app(&app)
{
	this->ui->setupUi(this);
	this->path = metadata.get_full_path();

	this->ui->filename_box->setText(metadata.get_filename());
	this->ui->path_box->setText(this->path);
	this->initialize_size(metadata.get_size());
	if (metadata.get_size().second < 2)
		this->ui->colors_box->setText(to_string(metadata.get_color_count().first) + " (counted in " + to_string(metadata.get_color_count().second) + " ms)");
	else
		this->ui->colors_box->setText("Not counted for animations.");
	auto size = metadata.get_filesize();
	this->ui->filesize_box->setText(format_size(size) + " (" + to_string(size) + " bytes)");
	{
		auto &date = metadata.get_date();
		QString date_string;
		if (date.isValid())
			date_string = QLocale::system().toString(date.toLocalTime());
		else
			date_string = "Unknown";
		this->ui->date_box->setText(date_string);
	}
	this->initialize_exif(metadata);

	connect(this->ui->close_btn, SIGNAL(clicked(bool)), this, SLOT(close()));
	connect(this->ui->show_folder_btn, SIGNAL(clicked(bool)), this, SLOT(show_in_folder()));
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

void InfoDialog::show_in_folder(){
	this->app->show_file_in_folder(this, this->path);
}
