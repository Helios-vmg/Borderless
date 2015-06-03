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

#ifndef SHORTCUTS_H
#define SHORTCUTS_H

#include <QString>
#include <QKeySequence>
#include <map>
#include <vector>
#include <memory>
#include "Settings.h"

#define DECLARE_COMMAND_INTERNAL_NAME(x) extern const char *x##_command

DECLARE_COMMAND_INTERNAL_NAME(back);
DECLARE_COMMAND_INTERNAL_NAME(background_swap);
DECLARE_COMMAND_INTERNAL_NAME(close);
DECLARE_COMMAND_INTERNAL_NAME(cycle_zoom_mode);
DECLARE_COMMAND_INTERNAL_NAME(down);
DECLARE_COMMAND_INTERNAL_NAME(down_big);
DECLARE_COMMAND_INTERNAL_NAME(flip_h);
DECLARE_COMMAND_INTERNAL_NAME(flip_v);
DECLARE_COMMAND_INTERNAL_NAME(go_to_end);
DECLARE_COMMAND_INTERNAL_NAME(go_to_start);
DECLARE_COMMAND_INTERNAL_NAME(left);
DECLARE_COMMAND_INTERNAL_NAME(left_big);
DECLARE_COMMAND_INTERNAL_NAME(minimize);
DECLARE_COMMAND_INTERNAL_NAME(minimize_all);
DECLARE_COMMAND_INTERNAL_NAME(next);
DECLARE_COMMAND_INTERNAL_NAME(quit);
DECLARE_COMMAND_INTERNAL_NAME(quit2);
DECLARE_COMMAND_INTERNAL_NAME(reset_zoom);
DECLARE_COMMAND_INTERNAL_NAME(right);
DECLARE_COMMAND_INTERNAL_NAME(right_big);
DECLARE_COMMAND_INTERNAL_NAME(rotate_left);
DECLARE_COMMAND_INTERNAL_NAME(rotate_left_fine);
DECLARE_COMMAND_INTERNAL_NAME(rotate_right);
DECLARE_COMMAND_INTERNAL_NAME(rotate_right_fine);
DECLARE_COMMAND_INTERNAL_NAME(show_options);
DECLARE_COMMAND_INTERNAL_NAME(toggle_fullscreen);
DECLARE_COMMAND_INTERNAL_NAME(toggle_lock_zoom);
DECLARE_COMMAND_INTERNAL_NAME(up);
DECLARE_COMMAND_INTERNAL_NAME(up_big);
DECLARE_COMMAND_INTERNAL_NAME(zoom_in);
DECLARE_COMMAND_INTERNAL_NAME(zoom_out);

struct ShortcutSetting{
	QString command;
	std::vector<QKeySequence> sequences;
};

struct ShortcutInfo{
	const char *display_name;
	const char *internal_name;
	QKeySequence default_sequences[4];
	size_t sequences_count;
	ShortcutInfo(){}
	ShortcutInfo(const char *display_name, const char *internal_name) : display_name(display_name), internal_name(internal_name), sequences_count(0){}
	ShortcutInfo(const char *display_name, const char *internal_name, const char *s1) : display_name(display_name), internal_name(internal_name), sequences_count(0){
		*this << s1;
	}
	ShortcutInfo(const char *display_name, const char *internal_name, const char *s1, const char *s2) : display_name(display_name), internal_name(internal_name), sequences_count(0){
		*this << s1 << s2;
	}
	ShortcutInfo &operator<<(const char *s){
		this->sequences_count %= 4;
		this->default_sequences[this->sequences_count] = QKeySequence((QString)s);
		this->sequences_count++;
		return *this;
	}
};

struct ShortcutTriple{
	QString command;
	QString display_name;
	QKeySequence sequence;
};

class ApplicationShortcuts{
	std::map<QString, ShortcutInfo> default_shortcuts;
	std::map<QString, std::shared_ptr<ShortcutSetting> > current_shortcuts;
public:
	ApplicationShortcuts();
	void restore_settings(const SettingsTree &tree);
	void reset_settings();
	std::shared_ptr<SettingsTree> save_settings();
	const ShortcutInfo *get_shortcut_info(const QString &command) const;
	QString get_display_name(const QString &command) const;
	const ShortcutSetting *get_shortcut_setting(const QString &command) const;
	const std::map<QString, ShortcutInfo> &get_shortcut_infos() const{
		return this->default_shortcuts;
	}
	std::vector<ShortcutTriple> get_default_shortcuts() const;
	std::vector<ShortcutTriple> get_current_shortcuts() const;
	void update(const std::vector<ShortcutTriple> &);
	QKeySequence get_current_sequence(const QString &command) const{
		auto setting = this->get_shortcut_setting(command);
		return (!setting || !setting->sequences.size()) ? QKeySequence() : setting->sequences[0];
	}
};

#endif
