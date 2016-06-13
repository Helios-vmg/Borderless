/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef STREAMREDIRECTOR_H
#define STREAMREDIRECTOR_H

#include <string>
#include <array>

class StdStreamRedirector{
	enum PIPES { READ, WRITE };
	std::array<int, 2> pipes = {{0, 0}};
	int old_stdout = 0;
	int old_stderr = 0;
	bool capturing = false;
	bool initialized = false;
	std::string capture;
public:
	StdStreamRedirector();
	~StdStreamRedirector();
	void begin_capture();
	bool end_capture();
	const std::string &get_capture() const{
		return this->capture;
	}
};

class StdStreamRedirectionGuard{
	StdStreamRedirector redir;
	std::string &dst;
public:
	StdStreamRedirectionGuard(std::string &dst): dst(dst){
		this->redir.begin_capture();
	}
	~StdStreamRedirectionGuard(){
		this->redir.end_capture();
		this->dst = this->redir.get_capture();
	}
};

#endif
