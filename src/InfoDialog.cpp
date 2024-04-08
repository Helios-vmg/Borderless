/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "InfoDialog.h"

InfoDialog::InfoDialog(QWidget &parent, const ImageMetadata &metadata): QDialog(&parent), ui(std::make_unique<Ui::InfoDialog>()){
	this->ui->setupUi(this);
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
