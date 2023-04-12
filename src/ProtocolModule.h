/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef PROTOCOLMODULE_H
#define PROTOCOLMODULE_H

#include <QString>
#include <QLibrary>
#include <QIODevice>
#include <QtCore5Compat/QRegExp>
#include <string>
#include <unordered_map>
#include <memory>

class ProtocolFileEnumerator;

class ProtocolModule{
	QLibrary lib;
	bool ok;
	std::string protocol;

	class protocol_client_t;
	class unknown_stream_t;
	class file_enumerator_t;
	friend class ProtocolFileEnumerator;

	typedef const char *(*get_protocol_f)();
	typedef protocol_client_t *(*initialize_client_f)(const wchar_t *, const wchar_t *);
	typedef void (*terminate_client_f)(protocol_client_t *);
	typedef unknown_stream_t *(*open_file_utf8_f)(protocol_client_t *, const char *);
	typedef unknown_stream_t *(*open_file_utf16_f)(protocol_client_t *, const wchar_t *);
	typedef void (*close_file_f)(unknown_stream_t *);
	typedef std::uint64_t(*read_file_f)(unknown_stream_t *, void *, std::uint64_t);
	typedef int (*seek_file_f)(unknown_stream_t *, std::uint64_t);
	typedef std::uint64_t (*file_length_f)(unknown_stream_t *);
	///////
	typedef file_enumerator_t *(*create_sibling_enumerator_f)(protocol_client_t *, const wchar_t *);
	typedef const wchar_t *(*sibling_enumerator_next_f)(file_enumerator_t *);
	typedef int (*sibling_enumerator_find_f)(file_enumerator_t *, size_t *, const wchar_t *);
	typedef void (*destroy_sibling_enumerator_f)(file_enumerator_t *);
	///////
	typedef void (*release_returned_string_f)(const wchar_t *);
	typedef const wchar_t *(*get_parent_directory_f)(protocol_client_t *, const wchar_t *);
	typedef int (*paths_in_same_directory_f)(protocol_client_t *, const wchar_t *, const wchar_t *);
	typedef const wchar_t *(*get_filename_from_url_f)(protocol_client_t *, const wchar_t *);
	typedef const wchar_t *(*begin_restore_f)(protocol_client_t *);
	typedef const wchar_t *(*end_restore_f)(protocol_client_t *);
	typedef get_filename_from_url_f get_unique_filename_from_url_f;
#define DECLARE_FUNCTION_POINTER(x) x##_f x##_p
	DECLARE_FUNCTION_POINTER(get_protocol);
	DECLARE_FUNCTION_POINTER(initialize_client);
	DECLARE_FUNCTION_POINTER(terminate_client);
	DECLARE_FUNCTION_POINTER(open_file_utf8);
	DECLARE_FUNCTION_POINTER(open_file_utf16);
	DECLARE_FUNCTION_POINTER(close_file);
	DECLARE_FUNCTION_POINTER(read_file);
	DECLARE_FUNCTION_POINTER(seek_file);
	DECLARE_FUNCTION_POINTER(file_length);
	DECLARE_FUNCTION_POINTER(create_sibling_enumerator);
	DECLARE_FUNCTION_POINTER(sibling_enumerator_next);
	DECLARE_FUNCTION_POINTER(sibling_enumerator_find);
	DECLARE_FUNCTION_POINTER(destroy_sibling_enumerator);
	DECLARE_FUNCTION_POINTER(release_returned_string);
	DECLARE_FUNCTION_POINTER(get_parent_directory);
	DECLARE_FUNCTION_POINTER(paths_in_same_directory);
	DECLARE_FUNCTION_POINTER(get_filename_from_url);
	DECLARE_FUNCTION_POINTER(get_unique_filename_from_url);
	DECLARE_FUNCTION_POINTER(begin_restore);
	DECLARE_FUNCTION_POINTER(end_restore);
	protocol_client_t *client;

	class Stream : public QIODevice{
		qint64 position, length;
		std::unique_ptr<char[]> data;
	public:
		Stream(ProtocolModule *module, unknown_stream_t *stream);
		~Stream();
		qint64 readData(char *data, qint64 maxSize) override;
		bool seek(qint64 position) override;
		bool isSequential() const override{
			return false;
		}
		qint64 writeData(const char *data, qint64 maxSize) override{
			return 0;
		}
		qint64 pos() const override{
			return this->position;
		}
		bool reset() override{
			return this->seek(0);
		}
		qint64 size() const override{
			return this->length;
		}
		bool atEnd() const override{
			return this->position >= this->length;
		}
	};

	QString get_filename(get_filename_from_url_f, const QString &);
public:
	ProtocolModule(const QString &filename, const QString &config_location, const QString &plugins_location);
	~ProtocolModule();
	operator bool() const{
		return this->ok;
	}
	const std::string &get_protocol_string() const{
		return this->protocol;
	}
	std::unique_ptr<QIODevice> open(const QString &);
	ProtocolFileEnumerator enumerate_siblings(const QString &);
	QString get_parent(const QString &);
	bool are_paths_in_same_directory(const QString &, const QString &);
	QString get_filename(const QString &);
	QString get_unique_filename(const QString &);
	void begin_restore();
	void end_restore();
};

class CustomProtocolHandler{
	std::unordered_map<std::string, std::unique_ptr<ProtocolModule>> modules;

	ProtocolModule *find_module_by_url(const QString &);
public:
	CustomProtocolHandler(const QString &config_location);
	std::unique_ptr<QIODevice> open(const QString &s);
	static bool is_url(const QString &);
	ProtocolFileEnumerator enumerate_siblings(const QString &);
	QString get_parent_directory(const QString &);
	QString get_filename(const QString &);
	QString get_unique_filename(const QString &);
	bool paths_in_same_directory(const QString &, const QString &);
	void begin_restore();
	void end_restore();
};

class ProtocolFileEnumerator{
	ProtocolModule *mod;
	ProtocolModule::file_enumerator_t *handle;
public:
	ProtocolFileEnumerator(): mod(nullptr), handle(nullptr){}
	ProtocolFileEnumerator(ProtocolModule &mod, ProtocolModule::file_enumerator_t &handle): mod(&mod), handle(&handle){}
	ProtocolFileEnumerator(ProtocolFileEnumerator &&);
	const ProtocolFileEnumerator &operator=(ProtocolFileEnumerator &&);
	ProtocolFileEnumerator(const ProtocolFileEnumerator &) = delete;
	ProtocolFileEnumerator &operator=(const ProtocolFileEnumerator &) = delete;
	~ProtocolFileEnumerator();

	QString next();
	bool find(const QString &path, size_t &dst);
	operator bool() const{
		return !!this->mod && !!this->handle;
	}
};

#endif
