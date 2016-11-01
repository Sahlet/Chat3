#pragma once
#include <WinSock2.h>
#include <mysql.h>
#include <mysqld_error.h>
#include <errmsg.h>
#include <My/Exception.h>
#include <memory>

class mysqlException : public My::Exception {
protected:
	mysqlException(long long errorCode, std::string errorCause, const char* what);
public:
	mysqlException(long long errorCode = 0, std::string errorCause = "");
	mysqlException(std::string errorCause, long long errorCode = 0);
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

