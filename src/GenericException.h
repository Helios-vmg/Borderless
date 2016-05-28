#ifndef GENERIC_EXCEPTION
#define GENERIC_EXCEPTION

#include <exception>

class GenericException : public std::exception{
	const char *cmessage;
public:
	GenericException(const char *message): cmessage(message){}
	virtual ~GenericException(){}
	virtual const char *what() const noexcept{
		return this->cmessage;
	}
};


#endif
