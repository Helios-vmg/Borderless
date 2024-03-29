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

#define INIT_FUNCTION(x) this->x##_p = nullptr
#define RESOLVE_FUNCTION(x) {                   \
	this->x##_p = (x##_f)this->lib.resolve(#x); \
	if (!this->x##_p)                           \
		return;                                 \
}
#define RESOLVE_FUNCTION_OPT(x) {               \
	this->x##_p = (x##_f)this->lib.resolve(#x); \
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
	INIT_FUNCTION(create_sibling_enumerator);
	INIT_FUNCTION(sibling_enumerator_next);
	INIT_FUNCTION(sibling_enumerator_find);
	INIT_FUNCTION(destroy_sibling_enumerator);
	INIT_FUNCTION(release_returned_string);
	INIT_FUNCTION(get_parent_directory);
	INIT_FUNCTION(paths_in_same_directory);
	INIT_FUNCTION(get_filename_from_url);
	INIT_FUNCTION(get_unique_filename_from_url);
	INIT_FUNCTION(seek_file);
	INIT_FUNCTION(file_length);
	INIT_FUNCTION(begin_restore);
	INIT_FUNCTION(end_restore);

	RESOLVE_FUNCTION(get_protocol);
	RESOLVE_FUNCTION(initialize_client);
	RESOLVE_FUNCTION(terminate_client);
	RESOLVE_FUNCTION_OPT(open_file_utf8);
	RESOLVE_FUNCTION_OPT(open_file_utf16);
	RESOLVE_FUNCTION(close_file);
	RESOLVE_FUNCTION(read_file);
	RESOLVE_FUNCTION(create_sibling_enumerator);
	RESOLVE_FUNCTION(sibling_enumerator_next);
	RESOLVE_FUNCTION(sibling_enumerator_find);
	RESOLVE_FUNCTION(destroy_sibling_enumerator);
	RESOLVE_FUNCTION(release_returned_string);
	RESOLVE_FUNCTION(get_parent_directory);
	RESOLVE_FUNCTION(paths_in_same_directory);
	RESOLVE_FUNCTION(get_filename_from_url);
	RESOLVE_FUNCTION_OPT(get_unique_filename_from_url);
	RESOLVE_FUNCTION(seek_file);
	RESOLVE_FUNCTION(file_length);
	RESOLVE_FUNCTION_OPT(begin_restore);
	RESOLVE_FUNCTION_OPT(end_restore);
	if (!this->open_file_utf8_p && !this->open_file_utf16_p)
		return;
	if (!!this->begin_restore_p != !!this->end_restore_p)
		return;

	auto cl = config_location.toStdWString();
	auto pl = plugins_location.toStdWString();
	this->client = this->initialize_client_p(cl.c_str(), pl.c_str());
	if (!this->client)
		return;
	this->protocol = this->get_protocol_p();
	this->ok = true;
}

ProtocolModule::~ProtocolModule(){
	if (!this->lib.isLoaded())
		return;
	if (this->client && this->terminate_client_p)
		this->terminate_client_p(this->client);
}

ProtocolModule::Stream::Stream(ProtocolModule *module, unknown_stream_t *stream){
	this->length = module->file_length_p(stream);
	this->data.reset(new char[this->length]);
	module->read_file_p(stream, this->data.get(), this->length);
	module->close_file_p(stream);
}

ProtocolModule::Stream::~Stream(){}

qint64 ProtocolModule::Stream::readData(char *data, qint64 maxSize){
	auto pos = this->pos();
	if (pos >= this->length)
		maxSize = 0;
	else
		maxSize = std::min(this->length - pos, maxSize);
	memcpy(data, this->data.get() + pos, maxSize);
	return maxSize;
}

std::unique_ptr<QIODevice> ProtocolModule::open(const QString &path){
	unknown_stream_t *stream;
	if (this->open_file_utf16_p){
		auto temp = path.toStdWString();
		stream = this->open_file_utf16_p(this->client, temp.c_str());
	}else{
		auto temp = path.toStdString();
		stream = this->open_file_utf8_p(this->client, temp.c_str());
	}
	if (!stream)
		return nullptr;
	auto ret = std::make_unique<Stream>(this, stream);
	ret->open(QIODeviceBase::ReadOnly);
	return ret;
}

ProtocolFileEnumerator ProtocolModule::enumerate_siblings(const QString &path){
	auto temp = path.toStdWString();
	auto enumerator = this->create_sibling_enumerator_p(this->client, temp.c_str());
	if (!enumerator)
		return {};
	return ProtocolFileEnumerator(*this, *enumerator);
}

QString ProtocolModule::get_parent(const QString &path){
	auto temp = path.toStdWString();
	auto wc = this->get_parent_directory_p(this->client, temp.c_str());
	std::shared_ptr<const wchar_t> shared_p(wc, this->release_returned_string_p);
	if (!wc)
		return {};
	return QString::fromWCharArray(wc);
}

bool ProtocolModule::are_paths_in_same_directory(const QString &a, const QString &b){
	auto A = a.toStdWString();
	auto B = b.toStdWString();
	return this->paths_in_same_directory_p(this->client, A.c_str(), B.c_str());
}

QString ProtocolModule::get_filename(get_filename_from_url_f f, const QString &path){
	auto temp = path.toStdWString();
	auto wc = f(this->client, temp.c_str());
	std::shared_ptr<const wchar_t> shared_p(wc, this->release_returned_string_p);
	if (!wc)
		return {};
	return QString::fromWCharArray(wc);
}

QString ProtocolModule::get_filename(const QString &path){
	return this->get_filename(this->get_filename_from_url_p, path);
}

QString ProtocolModule::get_unique_filename(const QString &path){
	if (!this->get_unique_filename_from_url_p)
		return this->get_unique_filename(path);
	return this->get_filename(this->get_unique_filename_from_url_p, path);
}

std::vector<QString> read_all_lines_from_file(QFile &file){
	std::vector<QString> lines;
	QTextStream stream(&file);
	stream.setEncoding(QStringConverter::Utf8);
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

	auto old = QDir::current();
	QDir::setCurrent(protocols_location);

	for (auto &line : lines){
		auto mod = std::make_unique<ProtocolModule>(protocols_location + line, config_location, protocols_location);
		if (!*mod)
			continue;
		auto proto = mod->get_protocol_string();
		to_lower(proto);
		this->modules[proto] = std::move(mod);
	}

	QDir::setCurrent(old.absolutePath());
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

bool CustomProtocolHandler::is_url(const QString &path){
	std::string unused;
	return ::is_url(unused, path);
}

ProtocolModule *CustomProtocolHandler::find_module_by_url(const QString &path){
	if (!this->modules.size())
		return nullptr;

	std::string scheme;
	if (!::is_url(scheme, path))
		return nullptr;

	auto it = this->modules.find(scheme);
	if (it == this->modules.end())
		return nullptr;
	return it->second.get();
}

std::unique_ptr<QIODevice>CustomProtocolHandler::open(const QString &path){
	auto mod = this->find_module_by_url(path);
	if (!mod)
		return nullptr;
	return mod->open(path);
}

ProtocolFileEnumerator CustomProtocolHandler::enumerate_siblings(const QString &path){
	auto mod = this->find_module_by_url(path);
	if (!mod)
		return ProtocolFileEnumerator();
	return mod->enumerate_siblings(path);
}

QString CustomProtocolHandler::get_parent_directory(const QString &path){
	auto mod = this->find_module_by_url(path);
	if (!mod)
		return QString();
	return mod->get_parent(path);
}

QString CustomProtocolHandler::get_filename(const QString &path){
	auto mod = this->find_module_by_url(path);
	if (!mod)
		return QString();
	return mod->get_filename(path);
}

QString CustomProtocolHandler::get_unique_filename(const QString &path){
	auto mod = this->find_module_by_url(path);
	if (!mod)
		return QString();
	return mod->get_unique_filename(path);
}

bool CustomProtocolHandler::paths_in_same_directory(const QString &a, const QString &b){
	if (!this->modules.size())
		return false;

	std::string scheme_a, scheme_b;
	if (!::is_url(scheme_a, a) || !::is_url(scheme_b, b) || scheme_a != scheme_b)
		return false;

	auto it = this->modules.find(scheme_a);
	if (it == this->modules.end())
		return false;
	return it->second->are_paths_in_same_directory(a, b);
}

ProtocolFileEnumerator::ProtocolFileEnumerator(ProtocolFileEnumerator &&other){
	*this = std::move(other);
}

const ProtocolFileEnumerator &ProtocolFileEnumerator::operator=(ProtocolFileEnumerator &&other){
	this->mod = other.mod;
	this->handle = other.handle;
	other.mod = nullptr;
	other.handle = nullptr;
	return *this;
}

ProtocolFileEnumerator::~ProtocolFileEnumerator(){
	if (!this->mod)
		return;
	this->mod->destroy_sibling_enumerator_p(this->handle);
}

QString ProtocolFileEnumerator::next(){
	auto s = this->mod->sibling_enumerator_next_p(this->handle);
	if (!s)
		return {};
	auto ret = QString::fromWCharArray(s);
	this->mod->release_returned_string_p(s);
	return ret;
}

bool ProtocolFileEnumerator::find(const QString &path, size_t &dst){
	auto temp = path.toStdWString();
	return !!this->mod->sibling_enumerator_find_p(this->handle, &dst, temp.c_str());
}

void ProtocolModule::begin_restore(){
	this->begin_restore_p(this->client);
}

void ProtocolModule::end_restore(){
	this->end_restore_p(this->client);
}

void CustomProtocolHandler::begin_restore(){
	for (auto &kv : this->modules)
		kv.second->begin_restore();
}

void CustomProtocolHandler::end_restore(){
	for (auto &kv : this->modules)
		kv.second->end_restore();
}
