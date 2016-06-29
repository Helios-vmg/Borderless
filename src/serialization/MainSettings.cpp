/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "settings.generated.h"
#include "../Enums.h"

MainSettings::MainSettings(){
	this->clamp_strength = 25;
	this->clamp_to_edges = true;
	this->use_checkerboard_pattern = true;
	this->center_when_displayed = true;
	this->keep_application_in_background = true;
	this->set_zoom_mode_for_new_windows(ZoomMode::Normal);
	this->set_fullscreen_zoom_mode_for_new_windows(ZoomMode::AutoFit);
	this->set_save_state_on_exit(true);
}

bool MainSettings::operator==(const MainSettings &other) const{
#define CHECK_EQUALITY(x) if (this->x != other.x) return false
	CHECK_EQUALITY(clamp_strength);
	CHECK_EQUALITY(clamp_to_edges);
	CHECK_EQUALITY(use_checkerboard_pattern);
	CHECK_EQUALITY(center_when_displayed);
	CHECK_EQUALITY(zoom_mode_for_new_windows);
	CHECK_EQUALITY(fullscreen_zoom_mode_for_new_windows);
	CHECK_EQUALITY(keep_application_in_background);
	CHECK_EQUALITY(save_state_on_exit);
	return true;
}
