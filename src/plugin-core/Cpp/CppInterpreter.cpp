/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "stdafx.h"
#include "CppInterpreter.h"
#include "../CallResultImpl.h"
#include "../../StreamRedirector.h"
#ifndef USING_PRECOMPILED_HEADERS
#include "llvm_headers.h"
#include <sstream>
#endif

CppInterpreterParameters::retrieve_tls_f global_retrieve_tls = nullptr;
CppInterpreterParameters::store_tls_f global_store_tls_f = nullptr;

struct FailedToHash : public std::exception{
	std::string msg;
public:
	FailedToHash(const std::string &path){
		this->msg = "Unable to compute hash for \'" + path + "\'.";
	}
	const char *what() const noexcept override{
		return this->msg.c_str();
	}
};

CppInterpreter::CppInterpreter(const CppInterpreterParameters &parameters): parameters(parameters){
	global_retrieve_tls = this->parameters.retrieve_tls;
	global_store_tls_f = this->parameters.store_tls;
	auto params = this->parameters;
	this->hf = [params](const std::string &path) -> Sha1Sum{
		Sha1Sum ret;
		if (!params.get_file_sha1(params.state, path.c_str(), ret.data, sizeof(ret.data)))
			throw FailedToHash(path);
		return ret;
	};
}

CppInterpreter::~CppInterpreter(){
	
}

using namespace clang;
using namespace clang::driver;

static llvm::ExecutionEngine *create_execution_engine(std::unique_ptr<llvm::Module> M, std::string *ErrorStr){
	return llvm::EngineBuilder(std::move(M))
		.setEngineKind(llvm::EngineKind::Either)
		.setErrorStr(ErrorStr)
		.create();
}

extern "C" __declspec(dllexport) void borderless_CppInterpreter_get_state(void **state, void **image){
	auto This = (CppInterpreter *)global_retrieve_tls(nullptr);
	This->pass_main_arguments(*state, *image);
}

extern "C" __declspec(dllexport) void borderless_CppInterpreter_return_result(void *return_value){
	auto This = (CppInterpreter *)global_retrieve_tls(nullptr);
	This->set_return_value(return_value);
}

bool execute(llvm::Module &mod, llvm::ExecutionEngine &execution_engine, CppInterpreter &cpp, std::string &error_message){
	llvm::Function *EntryFn = mod.getFunction("__borderless_main");
	if (!EntryFn){
		error_message = "'__borderless_main' function not found in module.";
		return false;
	}
		
	ArrayRef<llvm::GenericValue> args;
	execution_engine.finalizeObject();

	global_store_tls_f(nullptr, &cpp);
	execution_engine.runFunction(EntryFn, args);

	cpp.display_return_value_in_current_window();

	return true;
}

void CppInterpreter::display_return_value_in_current_window(){
	this->parameters.display_in_current_window(this->parameters.state, this->return_value);

}

bool CppInterpreter::compile_and_execute(const char *filename, std::string &error_message, std::string &redirection){
#if defined(WIN32) && defined(_CONSOLE) || !defined(WIN32)
	StdStreamRedirectionGuard guard(redirection);
#endif

	IntrusiveRefCntPtr<DiagnosticOptions> diagnostic_options = new DiagnosticOptions();

	IntrusiveRefCntPtr<DiagnosticIDs> diagnostic_ids = new DiagnosticIDs();
	DiagnosticsEngine diagnostics_engine(diagnostic_ids, &*diagnostic_options, new TextDiagnosticPrinter(llvm::errs(), &*diagnostic_options));

	// Use ELF on windows for now.
	std::string TripleStr = llvm::sys::getProcessTriple();
	llvm::Triple triple(TripleStr);
	if (triple.isOSBinFormatCOFF())
		triple.setObjectFormat(llvm::Triple::COFF);

	Driver driver("driver", triple.str(), diagnostics_engine);
	driver.setTitle("clang interpreter");
	driver.setCheckInputsExist(false);

	// FIXME: This is a hack to try to force the driver to do something we can
	// recognize. We need to extend the driver library to support this use model
	// (basically, exactly one input, and the operation mode is hard wired).
	SmallVector<const char *, 16> args;
	args.push_back("clang");
	args.push_back(filename);
	args.push_back("-fsyntax-only");
	args.push_back("-fms-compatibility-version=19");
	args.push_back("-O3");
	std::unique_ptr<Compilation> C(driver.BuildCompilation(args));
	if (!C){
		error_message = "Error initializing compiler.";
		return false;
	}

	// FIXME: This is copied from ASTUnit.cpp; simplify and eliminate.

	// We expect to get back exactly one command job, if we didn't something
	// failed. Extract that job from the compilation.
	const driver::JobList &jobs = C->getJobs();
	auto jobs_size = jobs.size();
	if (jobs_size != 1 || !isa<driver::Command>(*jobs.begin())) {
		SmallString<256> Msg;
		llvm::raw_svector_ostream OS(Msg);
		jobs.Print(OS, "; ", true);
		//Diags.Report(diag::err_fe_expected_compiler_job) << OS.str();
		error_message = OS.str();
		return false;
	}

	const driver::Command &command = cast<driver::Command>(*jobs.begin());
	if (llvm::StringRef(command.getCreator().getName()) != "clang") {
		//Diags.Report(diag::err_fe_expected_clang_command);
		error_message = "Expected clang command.";
		return false;
	}

	// Initialize a compiler invocation object from the clang (-cc1) arguments.
	const driver::ArgStringList &ccargs = command.getArguments();
	auto invocation = std::make_shared<CompilerInvocation>();
	CompilerInvocation::CreateFromArgs(
		*invocation,
		const_cast<const char **>(ccargs.data()),
		const_cast<const char **>(ccargs.data()) + ccargs.size(),
		diagnostics_engine
	);

	// FIXME: This is copied from cc1_main.cpp; simplify and eliminate.

	// Create a compiler instance to handle the actual work.
	CompilerInstance clang;
	clang.setInvocation(invocation);

	// Create the compilers actual diagnostics engine.
	clang.createDiagnostics();
	if (!clang.hasDiagnostics()){
		error_message = "No diagnostics.";
		return false;
	}

	// Create and execute the frontend to generate an LLVM bitcode module.
	auto context = std::make_shared<llvm::LLVMContext>();
	auto action = std::make_unique<EmitLLVMOnlyAction>(context.get());
	if (!clang.ExecuteAction(*action)){
		error_message = "Code generation failed.";
		return false;
	}

	auto module = action->takeModule();
	if (!module){
		error_message = "No module generated.";
		return false;
	}

	auto mod = module.get();

	if (!this->already_initialized){
		llvm::InitializeNativeTarget();
		llvm::InitializeNativeTargetAsmPrinter();
	}

	std::shared_ptr<llvm::ExecutionEngine> execution_engine(create_execution_engine(std::move(module), &error_message));

	if (!execution_engine){
		error_message = "Unable to make execution engine: " + error_message;
		return false;
	}

	if (!execute(*mod, *execution_engine, *this, error_message))
		return false;

	this->save_in_cache(filename, context, execution_engine, mod);

	return true;
}

CallResult CppInterpreter::execute_path(const char *filename){
	{
		CallResult ret;
		if (this->attempt_cache_reuse(ret, filename))
			return ret;
	}

	std::string error_message;
	std::string redirection;

	if (!this->compile_and_execute(filename, error_message, redirection)){
		CallResult ret;
		ret.impl = new CallResultImpl(redirection + "\n\n" + error_message);
		ret.success = false;
		ret.error_message = ret.impl->message.c_str();
		return ret;
	}
	return CallResult();
}

void CppInterpreter::pass_main_arguments(void *&state, void *&image) const{
	state = this->parameters.state;
	image = this->parameters.caller_image;
}

bool CppInterpreter::attempt_cache_reuse(CallResult &result, const char *path){
	std::string spath = path;
	auto it = this->cached_programs.find(spath);
	if (it == this->cached_programs.end())
		return false;
	auto &program = *it->second;
	if (!program.equals(path)){
		this->cached_programs.erase(it);
		return false;
	}
	result = program.execute();
	return true;
}

void CppInterpreter::save_in_cache(const char *path, const std::shared_ptr<llvm::LLVMContext> &context, const std::shared_ptr<llvm::ExecutionEngine> &execution_engine, llvm::Module *module){
	std::string spath = path;
	this->cached_programs[spath].reset(new CachedProgram(this, spath, context, execution_engine, module));
}

CachedProgram::CachedProgram(
		CppInterpreter *interpreter,
		const std::string &path,
		const std::shared_ptr<llvm::LLVMContext> &context,
		const std::shared_ptr<llvm::ExecutionEngine> &execution_engine,
		llvm::Module *module
){
	this->interpreter = interpreter;
	this->path = path;
	this->context = context;
	this->execution_engine = execution_engine;
	this->module = module;
	this->hash = (*this->interpreter->get_hash_function())(this->path);
}

CallResult CachedProgram::execute(){
	std::string error_message;
	CallResult ret;
	if (::execute(*this->module, *this->execution_engine, *this->interpreter, error_message))
		return ret;
	ret.impl = new CallResultImpl(error_message);
	ret.success = false;
	ret.error_message = ret.impl->message.c_str();
	return ret;
}

bool CachedProgram::equals(const std::string &path){
	return (*this->interpreter->get_hash_function())(path) == this->hash;
}
