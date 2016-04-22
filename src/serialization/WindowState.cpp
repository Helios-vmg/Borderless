/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "settings.generated.h"
#include "../Enums.h"

WindowState::WindowState(){
	this->using_checkerboard_pattern = true;
	this->zoom = 1;
	this->fullscreen = false;
	this->zoom_mode = (std::uint32_t)ZoomMode::Normal;
	this->fullscreen_zoom_mode = (std::uint32_t)ZoomMode::AutoFit;
	this->border_size = this->default_border_size;
	this->movement_size = 100;
}
