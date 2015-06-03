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

#include "Shortcuts.h"

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

#define SETUP_DEFAULT_SHORTCUT1(x, y, z) this->default_shortcuts[y] = ShortcutInfo(x, y, z)
#define SETUP_DEFAULT_SHORTCUT2(x, y, z, z2) this->default_shortcuts[y] = ShortcutInfo(x, y, z, z2)

ApplicationShortcuts::ApplicationShortcuts(){
	SETUP_DEFAULT_SHORTCUT1("Quit", quit_command, "Q");
	SETUP_DEFAULT_SHORTCUT1("Quit without saving windows", quit2_command, "Ctrl+Q" );
	SETUP_DEFAULT_SHORTCUT2("Next image", next_command, "Space", "Right" );
	SETUP_DEFAULT_SHORTCUT2("Previous image", back_command, "Backspace", "Left" );
	SETUP_DEFAULT_SHORTCUT1("Change background mode", background_swap_command, "B" );
	SETUP_DEFAULT_SHORTCUT1("Close current", close_command, "Escape" );
	SETUP_DEFAULT_SHORTCUT1("Zoom in", zoom_in_command, "+" );
	SETUP_DEFAULT_SHORTCUT1("Zoom out", zoom_out_command, "-" );
	SETUP_DEFAULT_SHORTCUT1("Reset image transform", reset_zoom_command, "Shift+R" );
	SETUP_DEFAULT_SHORTCUT1("Move up", up_command, "Ctrl+Up" );
	SETUP_DEFAULT_SHORTCUT1("Move down", down_command, "Ctrl+Down" );
	SETUP_DEFAULT_SHORTCUT1("Move left", left_command, "Ctrl+Left" );
	SETUP_DEFAULT_SHORTCUT1("Move right", right_command, "Ctrl+Right" );
	SETUP_DEFAULT_SHORTCUT1("Move up (big step)", up_big_command, "Page Up" );
	SETUP_DEFAULT_SHORTCUT1("Move down (big step)", down_big_command, "Page Down" );
	SETUP_DEFAULT_SHORTCUT1("Move left (big step)", left_big_command, "Ctrl+Page Up" );
	SETUP_DEFAULT_SHORTCUT1("Move right (big step)", right_big_command, "Ctrl+Page Down" );
	SETUP_DEFAULT_SHORTCUT1("Change fit mode", cycle_zoom_mode_command, "F" );
	SETUP_DEFAULT_SHORTCUT1("Toggle zoom lock", toggle_lock_zoom_command, "Shift+L" );
	SETUP_DEFAULT_SHORTCUT1("Go to first image", go_to_start_command, "Home" );
	SETUP_DEFAULT_SHORTCUT1("Go to last image", go_to_end_command, "End" );
	SETUP_DEFAULT_SHORTCUT1("Show options dialog", show_options_command, "O");
	SETUP_DEFAULT_SHORTCUT2("Toggle fullscreen", toggle_fullscreen_command, "Return", "Enter");
	SETUP_DEFAULT_SHORTCUT1("Mirror horizontally", flip_h_command, "H" );
	SETUP_DEFAULT_SHORTCUT1("Mirror vertically", flip_v_command, "V" );
	SETUP_DEFAULT_SHORTCUT1("Minimize current window", minimize_command, "M" );
	SETUP_DEFAULT_SHORTCUT1("Minimize all windows", minimize_all_command, "Shift+M");
	SETUP_DEFAULT_SHORTCUT1("Rotate left", rotate_left_command, "L");
	SETUP_DEFAULT_SHORTCUT1("Rotate right", rotate_right_command, "R");
	this->reset_settings();
}

void ApplicationShortcuts::restore_settings(const SettingsTree &tree){
	auto &shortcuts = this->current_shortcuts;
	shortcuts.clear();
	tree.iterate([&shortcuts](const QString &key, const SettingsItem &subtree){
		if (subtree.is_value() || !subtree.is_array())
			return;
		auto array = static_cast<const SettingsArray &>(subtree);
		std::shared_ptr<ShortcutSetting> ss(new ShortcutSetting);
		ss->command = key;
		shortcuts[key] = ss;

		auto f = [&](int key, const SettingsItem &item){
			if (!item.is_value())
				return;
			auto value = static_cast<const SettingsValue &>(item);
			QKeySequence sequence;
			value.to(sequence);
			ss->sequences.push_back(sequence);
		};

		array.iterate(f);
	});
	
}

void ApplicationShortcuts::reset_settings(){
	for (auto &p : this->default_shortcuts){
		auto key = p.second.internal_name;
		auto &map = this->current_shortcuts;
		std::shared_ptr<ShortcutSetting> ss(new ShortcutSetting);
		ss->command = key;
		map[key] = ss;
		for (size_t i = 0; i < p.second.sequences_count; i++)
			ss->sequences.push_back(p.second.default_sequences[i]);
	}
}

std::shared_ptr<SettingsTree> ApplicationShortcuts::save_settings(){
	std::shared_ptr<SettingsTree> ret(new SettingsTree);
	for (auto &s : this->current_shortcuts){
		std::shared_ptr<SettingsArray> array(new SettingsArray);
		int i = 0;
		for (auto &seq : s.second->sequences)
			array->push_back(std::shared_ptr<SettingsValue>(new SettingsValue(seq)));
		ret->add_tree(s.first, array);
	}
	return ret;
}

const ShortcutInfo *ApplicationShortcuts::get_shortcut_info(const QString &command) const{
	auto it = this->default_shortcuts.find(command);
	if (it == this->default_shortcuts.end())
		return nullptr;
	return &it->second;
}

QString ApplicationShortcuts::get_display_name(const QString &command) const{
	auto si = this->get_shortcut_info(command);
	if (!si)
		return QString();
	return si->display_name;
}

const ShortcutSetting *ApplicationShortcuts::get_shortcut_setting(const QString &command) const{
	auto it = this->current_shortcuts.find(command);
	if (it == this->current_shortcuts.end())
		return nullptr;
	return it->second.get();
}

std::vector<ShortcutTriple> ApplicationShortcuts::get_default_shortcuts() const{
	std::vector<ShortcutTriple> ret;
	for (auto &p : this->default_shortcuts){
		if (!p.second.sequences_count)
			continue;
		ShortcutTriple pushee;
		pushee.command = p.second.internal_name;
		pushee.display_name = p.second.display_name;
		for (size_t i = 0; i < p.second.sequences_count; i++){
			pushee.sequence = p.second.default_sequences[i];
			ret.push_back(pushee);
		}
	}
	return ret;
}

std::vector<ShortcutTriple> ApplicationShortcuts::get_current_shortcuts() const{
	std::vector<ShortcutTriple> ret;
	for (auto &p : this->current_shortcuts){
		if (!p.second->sequences.size())
			continue;
		ShortcutTriple pushee;
		pushee.command = p.second->command;
		pushee.display_name = this->get_display_name(pushee.command);
		for (auto &s : p.second->sequences){
			pushee.sequence = s;
			ret.push_back(pushee);
		}
	}
	return ret;
}

void ApplicationShortcuts::update(const std::vector<ShortcutTriple> &new_shortcuts){
	this->current_shortcuts.clear();
	for (auto &shortcut : new_shortcuts){
		auto it = this->current_shortcuts.find(shortcut.command);
		std::shared_ptr<ShortcutSetting> setting;
		if (it == this->current_shortcuts.end()){
			setting.reset(new ShortcutSetting);
			setting->command = shortcut.command;
			this->current_shortcuts[shortcut.command] = setting;
		}else
			setting = it->second;
		setting->sequences.push_back(shortcut.sequence);
	}
}
