/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "PersistentSettingsStore.h"
#include "Settings.h"
#include "sqlite/sqlitepp.hpp"
#include <QFile>
#include <QJsonValue>
#include <QJsonDocument>
#include <QJsonObject>
#include <boost/cast.hpp>
#include <mutex>

using namespace sqlite3pp;

const char * const db_schema = R"sql(
begin;

create table images(
	id integer primary key,
	hash text not null unique,
	settings string not null
);

commit;
)sql";

class SqlitePersistentSettingsStore : public PersistentSettingsStore{
	DB db;
	Statement select;
	std::mutex mutex;

public:
	SqlitePersistentSettingsStore(const std::filesystem::path &path);
	std::optional<PreferredWindowPosition> get_preferred_position(const std::string &hash) override;
	void set_preferred_position(const std::string &hash, const PreferredWindowPosition &) override;
	void set_preferred_positions(const std::map<std::string, PreferredWindowPosition> &) override;
};

#ifdef WIN32
#define NATIVE_STRING_CONVERSION_FUNCTION toStdWString
#else
#define NATIVE_STRING_CONVERSION_FUNCTION toStdString
#endif

auto to_path(const QString &path){
	return std::filesystem::path(path.NATIVE_STRING_CONVERSION_FUNCTION());
}

std::unique_ptr<PersistentSettingsStore> PersistentSettingsStore::create_from_settings_directory(QString path0, bool create){
	auto path = to_path(path0) / "persistent_settings.sqlite";
	if (!create && !std::filesystem::exists(path))
		return {};
	return std::make_unique<SqlitePersistentSettingsStore>(path);
}

SqlitePersistentSettingsStore::SqlitePersistentSettingsStore(const std::filesystem::path &path): db(path){
	int count;
	this->db << "select count(*) from sqlite_master where type = 'table' and name = 'images';" << step >> count;
	if (!count)
		this->db.exec(db_schema);
	this->select = this->db << "select settings from images where hash = ?;";
}

std::optional<PreferredWindowPosition> SqlitePersistentSettingsStore::get_preferred_position(const std::string &hash){
	std::string json;
	std::optional<PreferredWindowPosition> ret;
	{
		std::lock_guard lg(this->mutex);
		this->select << reset << hash;
		if (!this->select.read())
			return ret;
		this->select >> json;
	}
	auto doc = QJsonDocument::fromJson(QByteArray(json.data(), json.size()));
	ret.emplace(doc.object());
	return ret;
}

Statement get_insert(DB &db){
	return db << "insert into images (hash, settings) values (?1, ?2) on conflict do update set settings = ?2;";
}

std::string to_json(const WindowPosition &pos){
	QJsonDocument doc;
	doc.setObject(pos.serialize().toObject());
	auto data = doc.toJson(QJsonDocument::Compact);
	return { data.data(), boost::numeric_cast<size_t>(data.size()) };
}

void SqlitePersistentSettingsStore::set_preferred_position(const std::string &hash, const PreferredWindowPosition &pos){
	auto s = to_json(pos);
	std::lock_guard lg(this->mutex);
	get_insert(this->db) << hash << s << step;
}

void SqlitePersistentSettingsStore::set_preferred_positions(const std::map<std::string, PreferredWindowPosition> &map){
	std::lock_guard lg(this->mutex);
	auto insert = get_insert(this->db);
	Transaction tx(this->db);
	for (auto &[k, v] : map)
		insert << reset << k << to_json(v) << step;
}
