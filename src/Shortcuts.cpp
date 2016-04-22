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


void ApplicationShortcuts::restore_settings(const Settings &settings){
#if 0
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
#endif
}

void ApplicationShortcuts::reset_settings(){
	//TODO
	abort();
}

void ApplicationShortcuts::save_settings(Settings &settings){
#if 0
	std::shared_ptr<SettingsTree> ret(new SettingsTree);
	for (auto &s : this->current_shortcuts){
		std::shared_ptr<SettingsArray> array(new SettingsArray);
		int i = 0;
		for (auto &seq : s.second->sequences)
			array->push_back(std::shared_ptr<SettingsValue>(new SettingsValue(seq)));
		ret->add_tree(s.first, array);
	}
	return ret;
#endif
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
