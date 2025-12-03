/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#pragma once

#include <memory>
#include <string>
#include <optional>
#include <Qstring>
#include <map>

class PreferredWindowPosition;

class PersistentSettingsStore{
public:
	virtual ~PersistentSettingsStore(){}
	static std::unique_ptr<PersistentSettingsStore> create_from_settings_directory(QString path, bool create);
	virtual std::optional<PreferredWindowPosition> get_preferred_position(const std::string &hash) = 0;
	virtual void set_preferred_position(const std::string &hash, const PreferredWindowPosition &) = 0;
	virtual void set_preferred_positions(const std::map<std::string, PreferredWindowPosition> &) = 0;
};
