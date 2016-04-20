/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "ZoomModeDropDown.h"

struct ZoomPair{
	const char *display_name;
	ZoomMode value;
};

static const ZoomPair display_pairs[] = {
	{ "Fit nothing", ZoomMode::Normal },
	{ "Fit to screen", ZoomMode::AutoFit },
	{ "Fill screen", ZoomMode::AutoFill },
};

ZoomModeDropDown::ZoomModeDropDown(QWidget *parent): QComboBox(parent){
	for (auto &dp : display_pairs)
		this->addItem(dp.display_name, (int)dp.value);
}

void ZoomModeDropDown::set_selected_item(ZoomMode mode){
	for (int i = 0; i < this->count(); i++){
		auto value = this->itemData(i);
		auto zm = (ZoomMode)value.toInt();
		if (zm == mode){
			this->setCurrentIndex(i);
			return;
		}
	}
}

ZoomMode ZoomModeDropDown::get_selected_item() const{
	return (ZoomMode)this->itemData(this->currentIndex()).toInt();
}
