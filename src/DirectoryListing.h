/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef DIRECTORYLISTING_H
#define DIRECTORYLISTING_H

#include "ProtocolModule.h"
#include <QString>
#include <QStringList>
#include <QFuture>
#include <vector>
#include <QtCore/qatomic.h>
#include <memory>
#include <unordered_map>

void initialize_supported_extensions();
class DirectoryIterator;

bool check_and_clean_path(QString &path);

class DirectoryListing{
protected:
	bool ok;
	QString base_path;
	QFuture<QStringList> entries;
public:
	virtual ~DirectoryListing(){}
	DirectoryIterator begin();
	virtual size_t size() = 0;
	virtual QString operator[](size_t) = 0;
	virtual bool find(size_t &, const QString &) = 0;
	operator bool() const{
		return this->ok;
	}
	virtual bool operator==(const QString &path) = 0;
	virtual bool is_local() const = 0;
	virtual QString get_filename(size_t) = 0;
	virtual QString get_unique_filename(size_t i){
		return this->get_filename(i);
	}
	virtual void sync(){}
};

class LocalDirectoryListing : public DirectoryListing{
	QFuture<QStringList> entries;
public:
	LocalDirectoryListing(const QString &path, CustomProtocolHandler &): LocalDirectoryListing(path){}
	LocalDirectoryListing(const QString &path);
	size_t size() override;
	QString operator[](size_t) override;
	bool find(size_t &, const QString &) override;
	bool operator==(const QString &path) override;
	bool is_local() const override{
		return true;
	}
	QString get_filename(size_t) override;
};

class ProtocolDirectoryListing : public DirectoryListing{
public:
	typedef std::shared_ptr<std::vector<QString>> list_t;
private:
	list_t future_result;
	std::unordered_map<size_t, QString> unique_filenames;
	std::unordered_map<size_t, QString> filenames;
	ProtocolFileEnumerator enumerator;
	QFuture<list_t> future;
	std::shared_ptr<ProtocolModule::Client> client;

	list_t get_result();
public:
	ProtocolDirectoryListing(const QString &path, CustomProtocolHandler &);
	size_t size() override;
	QString operator[](size_t) override;
	bool find(size_t &, const QString &) override;
	bool operator==(const QString &path) override;
	bool is_local() const override{
		return false;
	}
	QString get_filename(size_t) override;
	QString get_unique_filename(size_t i) override;
	void sync() override;
};

class DirectoryIterator{
	DirectoryListing *dl;
	size_t position;
	bool in_position;
public:
	DirectoryIterator(DirectoryListing &dl): dl(&dl), position(0), in_position(false){}
	bool advance_to(const QString &name){
		if (this->in_position)
			return true;
		return this->in_position = this->dl->find(this->position, name);
	}
	QString operator*() const{
		return (*this->dl)[this->position];
	}
	void operator++(){
		this->position = (this->position + 1) % this->dl->size();
	}
	void operator--(){
		auto n = this->dl->size();
		this->position = (this->position + n - 1) % n;
	}
	DirectoryListing *get_listing() const{
		return this->dl;
	}
	size_t pos() const{
		return this->position;
	}
	void to_start(){
		this->position = 0;
	}
	void to_end(){
		this->to_start();
		--*this;
	}
	bool get_is_local() const{
		return this->dl->is_local();
	}
	QString get_directory();
	QString get_current_filename() const{
		return this->dl->get_unique_filename(this->position);
	}
};

class ExtensionIterator{
	std::unique_ptr<void, void (*)(void *)> pimpl;
	ExtensionIterator(const void *);
	static void release_pointer(void *);
public:
	ExtensionIterator(const ExtensionIterator &);
	const ExtensionIterator &operator=(const ExtensionIterator &other);
	static ExtensionIterator begin();
	static ExtensionIterator end();
	const ExtensionIterator &operator++();
	bool operator==(const ExtensionIterator &other) const;
	bool operator!=(const ExtensionIterator &other) const{
		return !(*this != other);
	}
	QString operator*() const;
};

class SupportedExtensions{
public:
	ExtensionIterator begin(){
		return ExtensionIterator::begin();
	}
	ExtensionIterator end(){
		return ExtensionIterator::end();
	}
};

#endif // DIRECTORYLISTING_H
