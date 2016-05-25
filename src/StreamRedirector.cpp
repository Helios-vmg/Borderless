/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "stdafx.h"
#include "StreamRedirector.h"
#ifndef USING_PRECOMPILED_HEADERS
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include <fcntl.h>
#include <stdio.h>
#endif

#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif

StdStreamRedirector::StdStreamRedirector(){
	this->pipes[READ] = 0;
	this->pipes[WRITE] = 0;
	if (_pipe(this->pipes.data(), 65536, O_BINARY) == -1)
		return;
	this->old_stdout = dup(fileno(stdout));
	this->old_stderr = dup(fileno(stderr));
	
	this->initialized = true;
}

StdStreamRedirector::~StdStreamRedirector(){
	this->end_capture();
	if (this->old_stdout > 0)
		close(this->old_stdout);
	if (this->old_stderr > 0)
		close(this->old_stderr);
	if (this->pipes[READ] > 0)
		close(this->pipes[READ]);
	if (this->pipes[WRITE] > 0)
		close(this->pipes[WRITE]);
}

void StdStreamRedirector::begin_capture(){
	if (!this->initialized)
		return;
	this->end_capture();
	fflush(stdout);
	fflush(stderr);
	auto temp = fileno(stdout);
	dup2(this->pipes[WRITE], fileno(stdout));
	dup2(this->pipes[WRITE], fileno(stderr));
	this->capturing = true;
}

bool StdStreamRedirector::end_capture(){
	if (!this->initialized || !this->capturing)
		return false;
	this->capturing = false;
	fflush(stdout);
	fflush(stderr);
	if (this->old_stdout >= 0)
		dup2(this->old_stdout, fileno(stdout));
	if (this->old_stderr >= 0)
		dup2(this->old_stderr, fileno(stderr));
	this->capture.clear();

	std::string buf;
	const int bufSize = 1024;
	buf.resize(bufSize);
	int bytesRead = 0;
	if (!eof(this->pipes[READ]))
		bytesRead = read(this->pipes[READ], &(*buf.begin()), bufSize);

	while (bytesRead == bufSize){
		this->capture += buf;
		bytesRead = 0;
		if (!eof(this->pipes[READ]))
			bytesRead = read(this->pipes[READ], &(*buf.begin()), bufSize);
	}

	if (bytesRead > 0){
		buf.resize(bytesRead);
		this->capture += buf;
	}
	return true;
}
