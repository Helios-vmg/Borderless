/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "Shortcuts.h"

std::map<QString, ShortcutInfo> get_default_shortcuts();

ApplicationShortcuts::ApplicationShortcuts(){
	this->default_shortcuts = ::get_default_shortcuts();
	this->reset_settings();
}


void ApplicationShortcuts::restore_settings(const Shortcuts &shortcuts){
	this->current_shortcuts.clear();
	for (auto &s : shortcuts.shortcuts){
		auto ss = std::make_shared<ShortcutSetting>();
		auto key = s.first;
		ss->command = key;
		this->current_shortcuts[key] = ss;

		for (auto &s2 : s.second)
			ss->sequences.emplace_back(QKeySequence(s2));
	}
}

void ApplicationShortcuts::reset_settings(){
	auto &map = this->current_shortcuts;
	map.clear();
	for (auto &p : this->default_shortcuts) {
		auto key = p.second.internal_name;
		std::shared_ptr<ShortcutSetting> ss(new ShortcutSetting);
		ss->command = key;
		map[key] = ss;
		for (size_t i = 0; i < p.second.sequences_count; i++)
			ss->sequences.push_back(p.second.default_sequences_seq[i]);
	}
}

std::shared_ptr<Shortcuts> ApplicationShortcuts::save_settings() const{
	auto ret = std::make_shared<Shortcuts>();
	for (auto &s : this->current_shortcuts){
		auto key = s.first;
		auto &shortcuts = ret->shortcuts[key];
		for (auto &seq : s.second->sequences)
			shortcuts.push_back(seq.toString());
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
			pushee.sequence = p.second.default_sequences_seq[i];
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
