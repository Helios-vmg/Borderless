/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#pragma once

#include "sqlite3.h"
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <type_traits>
#include <stdexcept>
#include <array>
#include <cstring>
#include <filesystem>
#include <atomic>

#ifdef USE_BOOST
#include <boost/filesystem.hpp>
#endif
#ifdef USE_QT
#include <QString>
#endif

namespace sqlite3pp{

class Exception : public std::exception{
protected:
	std::string message;
	Exception() = default;
public:
	int error_code = 0;
	Exception(int error_code, const char *base_error_msg, sqlite3 *db);
	Exception(const Exception &) = default;
	Exception &operator=(const Exception &) = default;
	Exception(Exception &&) = default;
	Exception &operator=(Exception &&) = default;
    virtual char const * what() const noexcept{
		return this->message.c_str();
    }
};

class BusyException : public Exception{
public:
	BusyException() = default;
	BusyException(const BusyException &) = default;
	BusyException &operator=(const BusyException &) = default;
	BusyException(BusyException &&) = default;
	BusyException &operator=(BusyException &&) = default;
    virtual char const * what() const noexcept override{
		return "database locked";
    }
};

class Statement;

enum class DbOpenMode{
	Default,
	ReadOnly,
};

class DB{
	sqlite3 *db;
	unsigned lock_count;
public:
	DB(const std::filesystem::path &path, bool Throw = true);
	DB(const std::filesystem::path &path, DbOpenMode mode, bool Throw = true);
	~DB(){
		if (this->good())
			sqlite3_close(this->db);
	}
	bool good() const{
		return !!this->db;
	}
	operator sqlite3 *() const{
		return this->db;
	}
	Statement operator<<(const char *s);
	void exec(const char *s);
	sqlite3_int64 last_insert_rowid();
	sqlite3_int64 changes();
	void begin_transaction(){
		if (!this->lock_count)
			this->exec("begin transaction;");
		this->lock_count++;
	}
	void commit(){
		if (this->lock_count == 1)
			this->exec("commit;");
		if (this->lock_count)
			this->lock_count--;
	}
	void rollback(){
		if (this->lock_count){
			this->exec("rollback;");
			this->lock_count=0;
		}
	}
};

void throw_sqlite_error(int error, sqlite3 *db);

class Transaction{
public:
	enum class ClearResult{
		None = 0,
		Committed,
		Rollbacked,
	};
private:
	DB *db = nullptr;
	bool commit_tx = true;
	ClearResult commit_on_destruction(){
		if (!this->db || !this->commit_tx)
			return ClearResult::None;
		if (!std::uncaught_exceptions()){
			this->db->commit();
			return ClearResult::Committed;
		}
		this->db->rollback();
		return ClearResult::Rollbacked;
	}
public:
	Transaction() = default;
	Transaction(DB &db): db(&db){
		db.begin_transaction();
	}
	Transaction(Transaction &&other){
		*this = std::move(other);
	}
	Transaction &operator=(Transaction &&other){
		this->commit_on_destruction();
		this->db = other.db;
		other.db = nullptr;
		this->commit_tx = other.commit_tx;
		other.commit_tx = false;
		return *this;
	}
	Transaction(const Transaction &) = delete;
	Transaction &operator=(const Transaction &) = delete;
	~Transaction(){
		this->commit_on_destruction();
	}
	void commit(){
		this->db->commit();
		this->db->begin_transaction();
	}
	void rollback(){
		this->db->rollback();
		this->commit_tx = false;
	}
	ClearResult clear(){
		auto ret = this->commit_on_destruction();
		this->db = nullptr;
		this->commit_tx = false;
		return ret;
	}
};

class NullType{};
class StepType{};
class ResetType{};
class UnusedType{};

inline const NullType null;
inline const StepType step;
inline const ResetType reset;
inline const UnusedType unused;

struct Buffer{
	const void *data;
	size_t size;
};

class OwningBuffer{
	std::string data;
	Buffer buffer;
public:
	OwningBuffer(): buffer{ nullptr, 0 }{}
	OwningBuffer(std::string s)
		: data(std::move(s))
		, buffer{ this->data.data(), this->data.size() }{}
	OwningBuffer(std::optional<std::string> s): buffer{nullptr, 0}{
		if (s){
			this->data = std::move(*s);
			this->buffer = { this->data.data(), this->data.size() };
		}
	}
	OwningBuffer(const OwningBuffer &) = delete;
	OwningBuffer &operator=(const OwningBuffer &) = delete;
	OwningBuffer(OwningBuffer &&) = default;
	OwningBuffer &operator=(OwningBuffer &&) = default;
	const auto &get_buffer() const{
		return this->buffer;
	}
};

class Statement{
	sqlite3 *db = nullptr;
	sqlite3_stmt *statement = nullptr;
	unsigned bind_index = 0,
		get_index = 0;
	int readonly = -1;

	friend class DB;
	Statement(sqlite3 *db,const char *s);
	void uninit(bool throws = true);
	bool is_null(){
		return sqlite3_column_type(this->statement, this->get_index) == SQLITE_NULL;
	}
public:
	Statement(): statement(nullptr){}
	Statement(const Statement &) = delete;
	Statement(Statement &&);
	~Statement();
	const Statement &operator=(const Statement &) = delete;
	const Statement &operator=(Statement &&);
	bool good() const{
		return !!this->statement;
	}
	bool is_readonly();
	bool operator!() const{
		return !this->good();
	}
	void reset();
	Statement &operator<<(const NullType &);
	Statement &operator<<(const StepType &);
	Statement &operator<<(const ResetType &);
	Statement &operator<<(bool b){
		return *this << (int)b;
	}
	Statement &operator<<(int);
	Statement &operator<<(unsigned u){
		return *this << (int)u;
	}
	Statement &operator<<(std::int64_t);
	Statement &operator<<(std::uint64_t);
	Statement &operator<<(double);
	Statement &operator<<(const char *s){
		if (!s)
			return *this << null;
		return *this << (std::string)s;
	}
	Statement &operator<<(const std::string &);
	Statement &operator<<(const std::vector<unsigned char> &);
	Statement &operator<<(Buffer);
	Statement &operator<<(const OwningBuffer &buffer){
		return *this << buffer.get_buffer();
	}
	/*
		How to use:
		while (stmt.step()==SQLITE_ROW){
			//Get more data.
		}
	*/
	int step();
	int step_nothrow();
	bool read(){
		return this->step() == SQLITE_ROW;
	}
	void set_get(unsigned i){
		this->get_index = i;
	}
	Statement &operator>>(const UnusedType &){
		this->get_index++;
		return *this;
	}
	Statement &operator>>(bool &b){
		int i;
		*this >> i;
		b=!!i;
		return *this;
	}
	Statement &operator>>(int &i);
	Statement &operator>>(unsigned &u){
		int i;
		*this >> i;
		u = (unsigned)i;
		return *this;
	}
	Statement &operator>>(std::int64_t &i);
	Statement &operator>>(std::uint64_t &i);
	Statement &operator>>(double &d);
	Statement &operator>>(std::string &s);
	template <size_t N>
	Statement &operator>>(std::array<char, N> &s){
		sqlite3_column_text(this->statement, this->get_index);
		size_t size = sqlite3_column_bytes(this->statement, this->get_index);
		if (size + 1 > N)
			throw std::runtime_error("can't read DB column into std::array");
		const char *p = (const char *)sqlite3_column_text(this->statement, this->get_index++);
		memcpy(s.data(), p, size);
		s[size] = 0;
		return *this;
	}
	Statement &operator>>(std::vector<unsigned char> &v);
	template <typename T>
	Statement &operator>>(std::unique_ptr<T> &p){
		if (this->is_null()){
			p.reset();
			return *this >> unused;
		}
		p.reset(new T);
		return *this >> *p;
	}
	template <typename T>
	Statement &operator>>(std::shared_ptr<T> &p){
		if (this->is_null()){
			p.reset();
			return *this >> unused;
		}
		p.reset(new T);
		return *this >> *p;
	}
	template <typename T>
	Statement &operator>>(std::optional<T> &p){
		T temp;
		if (this->is_null()){
			p.reset();
			return *this >> unused;
		}
		*this >> temp;
		p = std::move(temp);
		return *this;
	}
	template <typename T>
	Statement &operator<<(const std::optional<T> &p){
		if (!p)
			return *this << null;
		return *this << *p;
	}
	template <typename T>
	typename std::enable_if<std::is_enum<T>::value, Statement &>::type
	operator<<(const T &e){
		return *this << (typename std::underlying_type<T>::type)e;
	}
	template <typename T>
	typename std::enable_if<std::is_enum<T>::value, Statement &>::type
	operator>>(T &e){
		typename std::underlying_type<T>::type temp;
		*this >> temp;
		e = (T)temp;
		return *this;
	}
	template <typename T>
	Statement &operator>>(std::atomic<T> &dst){
		T temp;
		*this >> temp;
		dst = std::move(temp);
		return *this;
	}
	template <typename T>
	Statement &operator<<(const std::atomic<T> &src){
		return *this << src.load();
	}
};

}
