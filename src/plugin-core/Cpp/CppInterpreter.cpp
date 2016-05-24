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
#include <clang/CodeGen/CodeGenAction.h>
#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Driver/Compilation.h>
#include <clang/Driver/Driver.h>
#include <clang/Driver/Tool.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/CompilerInvocation.h>
#include <clang/Frontend/FrontendDiagnostic.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <sstream>
#endif

CppInterpreterParameters::retrieve_tls_f global_retrieve_tls = nullptr;
CppInterpreterParameters::store_tls_f global_store_tls_f = nullptr;

CppInterpreter::CppInterpreter(const CppInterpreterParameters &parameters): parameters(parameters){
	global_retrieve_tls = this->parameters.retrieve_tls;
	global_store_tls_f = this->parameters.store_tls;
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

extern "C" __declspec(dllexport) void borderless_CppInterpreter_get_state(void **state, int *image){
	auto This = (CppInterpreter *)global_retrieve_tls(nullptr);
	This->pass_main_arguments(*state, *image);
}

extern "C" __declspec(dllexport) void borderless_CppInterpreter_return_result(int return_value){
	auto This = (CppInterpreter *)global_retrieve_tls(nullptr);
	This->set_return_value(return_value);
}

static bool execute(std::unique_ptr<llvm::Module> module, CppInterpreter *cpp, std::string &error_message){
	llvm::InitializeNativeTarget();
	llvm::InitializeNativeTargetAsmPrinter();

	llvm::Module &mod = *module;
	std::unique_ptr<llvm::ExecutionEngine> EE(create_execution_engine(std::move(module), &error_message));
	if (!EE){
		error_message = "Unable to make execution engine: " + error_message;
		return false;
	}

	/*
	{
		llvm::Function *EntryFn = M.getFunction("get_some_value");
		if (!EntryFn) {
			llvm::errs() << "'main' function not found in module.\n";
			return 255;
		}

		ArrayRef<llvm::GenericValue> args;
		EE->finalizeObject();
		auto ret = EE->runFunction(EntryFn, args);
		std::cout << *ret.IntVal.getRawData() << std::endl;
	}
	*/

	{
		llvm::Function *EntryFn = mod.getFunction("__borderless_main");
		if (!EntryFn){
			error_message = "'__borderless_main' function not found in module.";
			return false;
		}
		
		ArrayRef<llvm::GenericValue> args;
		EE->finalizeObject();

		global_store_tls_f(nullptr, cpp);
		EE->runFunction(EntryFn, args);
	}

	return true;
}

CallResult CppInterpreter::execute_buffer(const char *filename){
	std::string error_message;
	std::string redirection;
	while (true){
		StdStreamRedirectionGuard guard(redirection);

		IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = new DiagnosticOptions();
		TextDiagnosticPrinter *DiagClient = new TextDiagnosticPrinter(llvm::errs(), &*DiagOpts);

		IntrusiveRefCntPtr<DiagnosticIDs> DiagID(new DiagnosticIDs());
		DiagnosticsEngine Diags(DiagID, &*DiagOpts, DiagClient);

		// Use ELF on windows for now.
		std::string TripleStr = llvm::sys::getProcessTriple();
		llvm::Triple T(TripleStr);
		if (T.isOSBinFormatCOFF())
			T.setObjectFormat(llvm::Triple::COFF);

		Driver TheDriver("driver", T.str(), Diags);
		TheDriver.setTitle("clang interpreter");
		TheDriver.setCheckInputsExist(false);

		// FIXME: This is a hack to try to force the driver to do something we can
		// recognize. We need to extend the driver library to support this use model
		// (basically, exactly one input, and the operation mode is hard wired).
		SmallVector<const char *, 16> args;
		args.push_back("clang");
		args.push_back(filename);
		args.push_back("-fsyntax-only");
		args.push_back("-fms-compatibility-version=19");
		args.push_back("-O3");
		std::unique_ptr<Compilation> C(TheDriver.BuildCompilation(args));
		if (!C){
			error_message = "Error initializing compiler.";
			break;
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
			break;
		}

		const driver::Command &command = cast<driver::Command>(*jobs.begin());
		if (llvm::StringRef(command.getCreator().getName()) != "clang") {
			//Diags.Report(diag::err_fe_expected_clang_command);
			error_message = "Expected clang command.";
			break;
		}

		// Initialize a compiler invocation object from the clang (-cc1) arguments.
		const driver::ArgStringList &ccargs = command.getArguments();
		std::unique_ptr<CompilerInvocation> invocation(new CompilerInvocation);
		CompilerInvocation::CreateFromArgs(
			*invocation,
			const_cast<const char **>(ccargs.data()),
			const_cast<const char **>(ccargs.data()) + ccargs.size(),
			Diags
		);

		// FIXME: This is copied from cc1_main.cpp; simplify and eliminate.

		// Create a compiler instance to handle the actual work.
		CompilerInstance clang;
		clang.setInvocation(invocation.release());

		// Create the compilers actual diagnostics engine.
		clang.createDiagnostics();
		if (!clang.hasDiagnostics()){
			error_message = "No diagnostics.";
			break;
		}

		// Create and execute the frontend to generate an LLVM bitcode module.
		std::unique_ptr<CodeGenAction> action(new EmitLLVMOnlyAction());
		if (!clang.ExecuteAction(*action)){
			error_message = "Code generation failed.";
			break;
		}

		std::unique_ptr<llvm::Module> module = action->takeModule();
		if (!module){
			error_message = "No module generated.";
			break;
		}
		
		if (!execute(std::move(module), this, error_message))
			break;

		return CallResult();
	}

	CallResult ret;
	ret.impl = new CallResultImpl(redirection + "\n\n" + error_message);
	ret.success = false;
	ret.error_message = ret.impl->message.c_str();
	return ret;
}

void CppInterpreter::pass_main_arguments(void *&state, int &image) const{
	state = this->parameters.state;
	image = this->parameters.caller_image;
}

void CppInterpreter::set_return_value(int){
	
}
