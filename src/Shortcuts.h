/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef SHORTCUTS_H
#define SHORTCUTS_H

#include "ShortcutInfo.h"
#include "Settings.h"
#include <QString>
#include <QKeySequence>
#include <map>
#include <vector>
#include <memory>

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
DECLARE_COMMAND_INTERNAL_NAME(show_transparent_background);
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
	void restore_settings(const Shortcuts &);
	void reset_settings();
	std::shared_ptr<Shortcuts> save_settings() const;
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
