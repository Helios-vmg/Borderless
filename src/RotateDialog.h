/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef ROTATE_DIALOG_H
#define ROTATE_DIALOG_H

#include <QDialog>
#include "ui_RotateDialog.h"
#include "MainWindow.h"
#include <memory>

class RotateDialog : public QDialog{
	Q_OBJECT
	std::shared_ptr<Ui_RotateDialog> ui;
	MainWindow &main_window;
	QMatrix transform;
	bool result;
	double rotation,
		original_scale,
		last_scale,
		scale;
	bool in_do_transform;

	void do_transform(bool = false);
	void set_scale();
	void set_scale_label();
public:
	RotateDialog(MainWindow &parent);
	bool accepted() const{
		return this->result;
	}

signals:

public slots :
	void rotation_slider_changed(int);
	void scale_slider_changed(int);
	void rejected_slot();
};

#endif
