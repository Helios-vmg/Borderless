#ifndef GENERIC_EXCEPTION
#define GENERIC_EXCEPTION

#include <exception>

#if defined _MSC_VER
#if _MSC_VER >= 1900
#define NOEXCEPT noexcept
#else
#define NOEXCEPT
#endif
#else
#define NOEXCEPT noexcept
#endif

class GenericException : public std::exception{
	const char *cmessage;
public:
	GenericException(const char *message): cmessage(message){}
	virtual ~GenericException(){}
	virtual const char *what() const NOEXCEPT{
		return this->cmessage;
	}
};


#endif
