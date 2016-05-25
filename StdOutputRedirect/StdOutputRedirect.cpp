#include <Windows.h> 
#include <iostream>
#include <cstdio> 
#include <string>
#include <exception>
#include <type_traits>
#include <cstring>

const size_t buffer_size = 4096;

template <typename T>
typename std::enable_if<std::is_pod<T>::value, void>::type
zero_struct(T &s){
	memset(&s, 0, sizeof(s));
}

HANDLE CreateChildProcess(std::wstring program, HANDLE write_pipe_end){
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	zero_struct(pi);
	zero_struct(si);

	si.cb = sizeof(STARTUPINFO);
	si.hStdError = write_pipe_end;
	si.hStdOutput = write_pipe_end;
	si.dwFlags |= STARTF_USESTDHANDLES;

	auto success = CreateProcessW(0, &program[0], 0, 0, true, 0, 0, 0, &si, &pi);

	if (!success)
		throw std::exception("CreateProcess failed.");

	CloseHandle(pi.hThread);
	return pi.hProcess;
}

void ReadFromPipe(HANDLE process, HANDLE read_pipe_end){
	DWORD read, written;
	CHAR buffer[buffer_size];
	HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	while (true){
		DWORD exit_code;
		auto success = GetExitCodeProcess(process, &exit_code);
		if (!success || exit_code != STILL_ACTIVE)
			break;
		success = ReadFile(read_pipe_end, buffer, buffer_size, &read, nullptr);
		if (!success || !read)
			break;

		success = WriteFile(hParentStdOut, buffer, read, &written, 0);
		if (!success)
			break;
	}
}

int main(int argc, char **argv){
	try{
		FreeConsole();

		SECURITY_ATTRIBUTES saAttr;

		HANDLE read_pipe_end = nullptr;
		HANDLE write_pipe_end = nullptr;

		saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
		saAttr.bInheritHandle = true;
		saAttr.lpSecurityDescriptor = nullptr;

		if (!CreatePipe(&read_pipe_end, &write_pipe_end, &saAttr, 0))
			throw std::exception("StdoutRd CreatePipe failed.");

		if (!SetHandleInformation(read_pipe_end, HANDLE_FLAG_INHERIT, 0))
			throw std::exception("Stdout SetHandleInformation failed.");

		auto child = CreateChildProcess(L"Borderless.exe", write_pipe_end);

		ReadFromPipe(child, read_pipe_end);

		CloseHandle(child);
	}catch (std::exception &e){
		std::cerr << "ERROR: " << e.what() << std::endl;
	}
	return 0;
}
