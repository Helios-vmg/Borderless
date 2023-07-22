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
#include <mutex>
#include <thread>

class ProtocolFileEnumerator;

class ProtocolModule : public std::enable_shared_from_this<ProtocolModule>{
	QLibrary lib;
	bool ok;
	std::string protocol;
	static inline int next_id = 1;
	int id = next_id++;

	class protocol_module_t;
	class protocol_client_t;
	class unknown_stream_t;
	class file_enumerator_t;
	friend class ProtocolFileEnumerator;

	typedef const char *(*get_protocol_f)();
	typedef protocol_module_t *(*initialize_module_f)(const wchar_t *, const wchar_t *);
	typedef void (*terminate_module_f)(protocol_module_t *);
	typedef protocol_client_t *(*initialize_client_f)(protocol_module_t *);
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
	typedef const wchar_t *(*begin_restore_f)(protocol_module_t *);
	typedef const wchar_t *(*end_restore_f)(protocol_module_t *);
	typedef get_filename_from_url_f get_unique_filename_from_url_f;
#define DECLARE_FUNCTION_POINTER(x) x##_f x##_p
	DECLARE_FUNCTION_POINTER(get_protocol);
	DECLARE_FUNCTION_POINTER(initialize_module);
	DECLARE_FUNCTION_POINTER(terminate_module);
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
	protocol_module_t *module;

	class Stream : public QIODevice{
		qint64 length;
		std::unique_ptr<char[]> data;
	public:
		Stream(ProtocolModule &module, unknown_stream_t *stream);
		~Stream();
		qint64 readData(char *data, qint64 maxSize) override;
		bool isSequential() const override{
			return false;
		}
		qint64 writeData(const char *data, qint64 maxSize) override{
			return 0;
		}
		qint64 size() const override{
			return this->length;
		}
		bool atEnd() const override{
			return this->pos() >= this->length;
		}
	};

public:
	ProtocolModule(const QString &filename, const QString &config_location, const QString &plugins_location);
	~ProtocolModule();
	ProtocolModule(const ProtocolModule &) = delete;
	ProtocolModule &operator=(const ProtocolModule &) = delete;
	ProtocolModule(ProtocolModule &&) = delete;
	ProtocolModule &operator=(ProtocolModule &&) = delete;
	operator bool() const{
		return this->ok;
	}
	const std::string &get_protocol_string() const{
		return this->protocol;
	}
	void begin_restore();
	void end_restore();

	class Client{
	protected:
		std::shared_ptr<ProtocolModule> mod;
		protocol_client_t *client = nullptr;
		
		QString get_filename(get_filename_from_url_f, const QString &);
		Client() = default;
	public:
		Client(std::shared_ptr<ProtocolModule> &&);
		virtual ~Client();
		Client(const Client &) = delete;
		Client &operator=(const Client &) = delete;
		Client(Client &&) = delete;
		Client &operator=(Client &&) = delete;
		virtual std::unique_ptr<QIODevice> open(const QString &);
		virtual ProtocolFileEnumerator enumerate_siblings(const QString &);
		virtual QString get_parent(const QString &);
		virtual bool are_paths_in_same_directory(const QString &, const QString &);
		virtual QString get_filename(const QString &);
		virtual QString get_unique_filename(const QString &);
	};
	class DummyClient : public Client{
	public:
		std::unique_ptr<QIODevice> open(const QString &) override{
			return {};
		}
		ProtocolFileEnumerator enumerate_siblings(const QString &) override;
		QString get_parent(const QString &) override{
			return {};
		}
		bool are_paths_in_same_directory(const QString &, const QString &) override{
			return false;
		}
		QString get_filename(const QString &) override{
			return {};
		}
		QString get_unique_filename(const QString &) override{
			return {};
		}
	};
	
	std::unique_ptr<Client> create_client();
	int get_id() const{
		return this->id;
	}
};

class CustomProtocolHandler{
	std::mutex modules_mutex;
	std::unordered_map<std::string, std::shared_ptr<ProtocolModule>> modules;
	std::mutex clients_mutex;
	std::map<std::pair<int, std::thread::id>, std::shared_ptr<ProtocolModule::Client>> clients;
	std::shared_ptr<ProtocolModule::DummyClient> dummy;
public:
	CustomProtocolHandler(const QString &config_location);
	std::shared_ptr<ProtocolModule::Client> get_client(const QString &, bool reuse = true);
	static bool is_url(const QString &);
	void begin_restore();
	void end_restore();
};

class ProtocolFileEnumerator{
	std::shared_ptr<ProtocolModule> mod;
	ProtocolModule::file_enumerator_t *handle;
public:
	ProtocolFileEnumerator(): handle(nullptr){}
	ProtocolFileEnumerator(std::shared_ptr<ProtocolModule> mod, ProtocolModule::file_enumerator_t &handle): mod(std::move(mod)), handle(&handle){}
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
