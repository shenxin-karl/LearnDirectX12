#pragma once
#include <exception>
#include <string>

namespace com {

class ExceptionBase : public std::exception {
public:
	ExceptionBase(const char *file, int line);
	const char *what() const noexcept override;
	virtual const char *getType() const noexcept;
	int getLine() const noexcept;
	const std::string &getFile() const noexcept;
	std::string getOriginString() const noexcept;
private:
	std::string	file_;
	int			line_;
protected:
	mutable std::string whatBuffer_;
};

}