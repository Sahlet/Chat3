#pragma once
#include <WinSock2.h>
#include <mysql.h>
#include <exception>
#include <string>
#include <memory>

class mysqlException : public std::exception {
	int errorCode;
	std::string errorCause;
public:
	mysqlException(int errorCode = 0, std::string errorCause = "");
	mysqlException(std::string errorCause, int errorCode = 0);
	inline int get_errorCode() { return errorCode; }
	inline const std::string& get_errorCause() { return errorCause; }
};

class mysqlWrap : public std::unique_ptr< MYSQL > {
	typedef std::unique_ptr< MYSQL > base;
public:
	mysqlWrap() = default;
	mysqlWrap(const mysqlWrap&) = delete;
	mysqlWrap(mysqlWrap&&) = default;
	mysqlWrap& operator = (mysqlWrap&&) = default;
	mysqlWrap(const char * host, unsigned int port, const char * user, const char * password) throw (mysqlException);
	virtual ~mysqlWrap();
};

