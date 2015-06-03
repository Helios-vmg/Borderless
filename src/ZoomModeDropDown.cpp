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
