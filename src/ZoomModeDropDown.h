/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef ZOOMMODEDROPDOWN_H
#define ZOOMMODEDROPDOWN_H

#include <QComboBox>
#include "ImageViewerApplication.h"

class ZoomModeDropDown : public QComboBox{
public:
	ZoomModeDropDown(QWidget *parent = nullptr);
	void set_selected_item(ZoomMode);
	ZoomMode get_selected_item() const;
};

#endif
