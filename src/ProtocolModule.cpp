/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "ProtocolModule.h"
#include "Misc.h"
#include <QFile>
#include <QTextStream>
#include <QDir>
#ifdef WIN32
#include <Windows.h>
#endif

#define INIT_FUNCTION(x) this->x = nullptr
#define RESOLVE_FUNCTION(x) {               \
	this->x = (x##_f)this->lib.resolve(#x); \
	if (!this->x)                           \
		return;                             \
}

ProtocolModule::ProtocolModule(const QString &filename, const QString &config_location, const QString &plugins_location){
	this->ok = false;
	this->client = nullptr;
	this->lib.setFileName(filename);
	this->lib.load();
	if (!this->lib.isLoaded())
		return;

	INIT_FUNCTION(get_protocol);
	INIT_FUNCTION(initialize_client);
	INIT_FUNCTION(terminate_client);
	INIT_FUNCTION(open_file_utf8);
	INIT_FUNCTION(open_file_utf16);
	INIT_FUNCTION(close_file);
	INIT_FUNCTION(read_file);

	RESOLVE_FUNCTION(get_protocol);
	RESOLVE_FUNCTION(initialize_client);
	RESOLVE_FUNCTION(terminate_client);
	RESOLVE_FUNCTION(open_file_utf8);
	RESOLVE_FUNCTION(open_file_utf16);
	RESOLVE_FUNCTION(close_file);
	RESOLVE_FUNCTION(read_file);

	auto cl = config_location.toStdWString();
	auto pl = plugins_location.toStdWString();
	this->client = this->initialize_client(cl.c_str(), pl.c_str());
	if (!this->client)
		return;
	this->protocol = this->get_protocol();
	this->ok = true;
}

ProtocolModule::~ProtocolModule(){
	if (!this->lib.isLoaded())
		return;
	if (this->client && this->terminate_client)
		this->terminate_client(this->client);
}

ProtocolModule::Stream::Stream(ProtocolModule *module, unknown_stream_t *stream){
	this->module = module;
	this->stream = stream;
}

ProtocolModule::Stream::~Stream(){
	this->module->close_file(this->stream);
}

qint64 ProtocolModule::Stream::readData(char *data, qint64 maxSize){
	return this->module->read_file(this->stream, data, maxSize);
}

std::unique_ptr<QIODevice> ProtocolModule::open(const QString &s){
	auto temp = s.toStdWString();
	auto stream = this->open_file_utf16(this->client, temp.c_str());
	std::unique_ptr<QIODevice> ret;
	if (!stream)
		return ret;
	ret.reset(new Stream(this, stream));
	return ret;
}

std::vector<QString> read_all_lines_from_file(QFile &file){
	std::vector<QString> lines;
	QTextStream stream(&file);
	stream.setCodec("UTF-8");
	while (!stream.atEnd()){
		auto line = stream.readLine();
		lines.push_back(line);
	}
	return lines;
}

CustomProtocolHandler::CustomProtocolHandler(const QString &config_location){
	auto c = QDir::separator();
	auto protocols_location = config_location + "protocols" + c;
	auto protocols_list_location = protocols_location + "protocols.txt";

	QFile list_file(protocols_list_location);
	if (!list_file.open(QIODevice::ReadOnly))
		return;

	auto lines = read_all_lines_from_file(list_file);

	for (auto &line : lines){
		auto mod = std::make_unique<ProtocolModule>(protocols_location + line, config_location, protocols_location);
		if (!*mod)
			continue;
		auto proto = mod->get_protocol_string();
		to_lower(proto);
		this->modules[proto] = std::move(mod);
	}
}

// Equivalent to regex ^([A-Za-z][A-Za-z0-9+.\-]*)\://.*
bool is_url(std::string &scheme, const QString &s){
	scheme.clear();
	int state = 0;
	for (int i = 0; i < s.size(); i++){
		auto c = s[i].toLatin1();
		switch (state){
			case 0:
				if (c >= 'A' & c <= 'Z' | c >= 'a' & c <= 'z'){
					scheme.push_back(tolower(c));
					state++;
					continue;
				}
				return false;
			case 1:
				if ((c >= 'A' & c <= 'Z' | c >= 'a' & c <= 'z') || (c >= '0' & c <= '9' | c == '+' | c == '.' | c == '-')){
					scheme.push_back(tolower(c));
					continue;
				}
				if (c == ':'){
					state++;
					continue;
				}
				return false;
			case 2:
			case 3:
				if (c == '/'){
					if (++state == 4)
						return true;
					continue;
				}
				return false;
		}
	}
	return false;
}

std::unique_ptr<QIODevice> CustomProtocolHandler::open(const QString &s){
	std::unique_ptr<QIODevice> ret;
	if (!this->modules.size())
		return ret;

	std::string scheme;
	if (!is_url(scheme, s))
		return ret;
	auto it = this->modules.find(scheme);
	if (it == this->modules.end())
		return ret;
	ret = it->second->open(s);
	return ret;
}
