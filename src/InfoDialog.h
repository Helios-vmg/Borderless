/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef INFODIALOG_H
#define INFODIALOG_H

#include "ui_InfoDialog.h"
#include <QDialog>
#include <memory>

class InfoDialog : public QDialog{
	Q_OBJECT

	std::unique_ptr<Ui::InfoDialog> ui;

public:
	InfoDialog(QWidget *parent = nullptr);
	
};

#endif
