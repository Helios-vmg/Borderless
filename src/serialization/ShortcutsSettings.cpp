/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "Settings.h"
#include "../ShortcutInfo.h"
#include <QString>

#define DEFINE_COMMAND_INTERNAL_NAME(x) const char *x##_command = #x

DEFINE_COMMAND_INTERNAL_NAME(back);
DEFINE_COMMAND_INTERNAL_NAME(background_swap);
DEFINE_COMMAND_INTERNAL_NAME(close);
DEFINE_COMMAND_INTERNAL_NAME(cycle_zoom_mode);
DEFINE_COMMAND_INTERNAL_NAME(down);
DEFINE_COMMAND_INTERNAL_NAME(down_big);
DEFINE_COMMAND_INTERNAL_NAME(flip_h);
DEFINE_COMMAND_INTERNAL_NAME(flip_v);
DEFINE_COMMAND_INTERNAL_NAME(go_to_end);
DEFINE_COMMAND_INTERNAL_NAME(go_to_start);
DEFINE_COMMAND_INTERNAL_NAME(left);
DEFINE_COMMAND_INTERNAL_NAME(left_big);
DEFINE_COMMAND_INTERNAL_NAME(minimize);
DEFINE_COMMAND_INTERNAL_NAME(minimize_all);
DEFINE_COMMAND_INTERNAL_NAME(next);
DEFINE_COMMAND_INTERNAL_NAME(quit);
DEFINE_COMMAND_INTERNAL_NAME(quit2);
DEFINE_COMMAND_INTERNAL_NAME(reset_zoom);
DEFINE_COMMAND_INTERNAL_NAME(right);
DEFINE_COMMAND_INTERNAL_NAME(right_big);
DEFINE_COMMAND_INTERNAL_NAME(rotate_left);
DEFINE_COMMAND_INTERNAL_NAME(rotate_left_fine);
DEFINE_COMMAND_INTERNAL_NAME(rotate_right);
DEFINE_COMMAND_INTERNAL_NAME(rotate_right_fine);
DEFINE_COMMAND_INTERNAL_NAME(show_options);
DEFINE_COMMAND_INTERNAL_NAME(toggle_fullscreen);
DEFINE_COMMAND_INTERNAL_NAME(toggle_lock_zoom);
DEFINE_COMMAND_INTERNAL_NAME(up);
DEFINE_COMMAND_INTERNAL_NAME(up_big);
DEFINE_COMMAND_INTERNAL_NAME(zoom_in);
DEFINE_COMMAND_INTERNAL_NAME(zoom_out);

#define SETUP_DEFAULT_SHORTCUT1(x, y, z) ret[y] = ShortcutInfo(x, y, z)
#define SETUP_DEFAULT_SHORTCUT2(x, y, z, z2) ret[y] = ShortcutInfo(x, y, z, z2)

std::map<QString, ShortcutInfo> get_default_shortcuts() {
	std::map<QString, ShortcutInfo> ret;
	SETUP_DEFAULT_SHORTCUT1("Quit", quit_command, "Q");
	SETUP_DEFAULT_SHORTCUT1("Quit without saving windows", quit2_command, "Ctrl+Q");
	SETUP_DEFAULT_SHORTCUT2("Next image", next_command, "Space", "Right");
	SETUP_DEFAULT_SHORTCUT2("Previous image", back_command, "Backspace", "Left");
	SETUP_DEFAULT_SHORTCUT1("Change background mode", background_swap_command, "B");
	SETUP_DEFAULT_SHORTCUT1("Close current", close_command, "Escape");
	SETUP_DEFAULT_SHORTCUT1("Zoom in", zoom_in_command, "+");
	SETUP_DEFAULT_SHORTCUT1("Zoom out", zoom_out_command, "-");
	SETUP_DEFAULT_SHORTCUT1("Reset image transform", reset_zoom_command, "Shift+R");
	SETUP_DEFAULT_SHORTCUT1("Move up", up_command, "Ctrl+Up");
	SETUP_DEFAULT_SHORTCUT1("Move down", down_command, "Ctrl+Down");
	SETUP_DEFAULT_SHORTCUT1("Move left", left_command, "Ctrl+Left");
	SETUP_DEFAULT_SHORTCUT1("Move right", right_command, "Ctrl+Right");
	SETUP_DEFAULT_SHORTCUT1("Move up (big step)", up_big_command, "Page Up");
	SETUP_DEFAULT_SHORTCUT1("Move down (big step)", down_big_command, "Page Down");
	SETUP_DEFAULT_SHORTCUT1("Move left (big step)", left_big_command, "Ctrl+Page Up");
	SETUP_DEFAULT_SHORTCUT1("Move right (big step)", right_big_command, "Ctrl+Page Down");
	SETUP_DEFAULT_SHORTCUT1("Change fit mode", cycle_zoom_mode_command, "F");
	SETUP_DEFAULT_SHORTCUT1("Toggle zoom lock", toggle_lock_zoom_command, "Shift+L");
	SETUP_DEFAULT_SHORTCUT1("Go to first image", go_to_start_command, "Home");
	SETUP_DEFAULT_SHORTCUT1("Go to last image", go_to_end_command, "End");
	SETUP_DEFAULT_SHORTCUT1("Show options dialog", show_options_command, "O");
	SETUP_DEFAULT_SHORTCUT2("Toggle fullscreen", toggle_fullscreen_command, "Return", "Enter");
	SETUP_DEFAULT_SHORTCUT1("Mirror horizontally", flip_h_command, "H");
	SETUP_DEFAULT_SHORTCUT1("Mirror vertically", flip_v_command, "V");
	SETUP_DEFAULT_SHORTCUT1("Minimize current window", minimize_command, "M");
	SETUP_DEFAULT_SHORTCUT1("Minimize all windows", minimize_all_command, "Shift+M");
	SETUP_DEFAULT_SHORTCUT1("Rotate left", rotate_left_command, "L");
	SETUP_DEFAULT_SHORTCUT1("Rotate right", rotate_right_command, "R");
	return ret;
}

void Shortcuts::initialize_to_defaults(){
	auto shortcuts = get_default_shortcuts();
	for (auto &s : shortcuts) {
		auto key = s.second.internal_name;
		auto &v = this->shortcuts[key];
		for (size_t i = 0; i < s.second.sequences_count; i++) {
			auto str = s.second.default_sequences_str[i];
			v.push_back(QString::fromUtf8(str));
		}
	}
}
