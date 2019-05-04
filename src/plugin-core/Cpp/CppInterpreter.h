/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef CPPINTERPRETER_H
#define CPPINTERPRETER_H

#include "main.h"
#include <string>
#include <map>

struct Sha1Sum{
	unsigned char data[20];
	bool operator==(const Sha1Sum &b){
		return !memcmp(this->data, b.data, sizeof(this->data));
	}
};

typedef std::function<Sha1Sum(const std::string &)> hash_function;

class CachedProgram{
	CppInterpreter *interpreter;
	std::string path;
	Sha1Sum hash;
	std::shared_ptr<llvm::LLVMContext> context;
	std::shared_ptr<llvm::ExecutionEngine> execution_engine;
	llvm::Module *module;
public:
	CachedProgram(
		CppInterpreter *interpreter,
		const std::string &path,
		const std::shared_ptr<llvm::LLVMContext> &context,
		const std::shared_ptr<llvm::ExecutionEngine> &execution_engine,
		llvm::Module *module
	);
	bool equals(const std::string &path);
	CallResult execute();
};

class CppInterpreter{
	CppInterpreterParameters parameters;
	hash_function hf;
	void *return_value;
	bool already_initialized = false;
	std::map<std::string, std::shared_ptr<CachedProgram>> cached_programs;
	bool attempt_cache_reuse(CallResult &, const char *);
	void save_in_cache(const char *, const std::shared_ptr<llvm::LLVMContext> &, const std::shared_ptr<llvm::ExecutionEngine> &, llvm::Module *);
	bool compile_and_execute(const char *, std::string &, std::string &);
public:
	CppInterpreter(const CppInterpreterParameters &);
	~CppInterpreter();
	CallResult execute_path(const char *filename);
	void pass_main_arguments(void *&, void *&) const;
	void set_return_value(void *rv){
		this->return_value = rv;
	}
	hash_function *get_hash_function(){
		return &this->hf;
	}
	void reset_image(external_state image){
		this->parameters.caller_image = image;
	}
	void display_return_value_in_current_window();
};

#endif
