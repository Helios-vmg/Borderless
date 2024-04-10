/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef INFODIALOG_H
#define INFODIALOG_H

#include "ui_InfoDialog.h"
#include "exif.h"
#include "ImageViewerApplication.h"
#include <QDialog>
#include <memory>

class InfoDialog : public QDialog{
	Q_OBJECT

	std::unique_ptr<Ui::InfoDialog> ui;
	ImageViewerApplication *app;
	QString path;

	void initialize_exif(const ImageMetadata &metadata);
	void initialize_size(const std::pair<QSize, int> &);
public:
	InfoDialog(QWidget &parent, ImageViewerApplication &app, const ImageMetadata &metadata);

public slots:
	void show_in_folder();
	
};

#endif
