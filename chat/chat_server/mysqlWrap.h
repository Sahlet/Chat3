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
	mysqlException(mysqlException&&) = default;
};

struct MYSQL_RES_deleter {
	void operator()(MYSQL_RES* res) {
		mysql_free_result(res);
	}
};


class mysqlWrap : public std::unique_ptr< MYSQL > {
	typedef std::unique_ptr< MYSQL > base;
public:
	mysqlWrap() = default;
	mysqlWrap(const mysqlWrap&) = delete;
	mysqlWrap(mysqlWrap&&) = default;
	mysqlWrap& operator = (mysqlWrap&&) = default;
	mysqlWrap(const char * host, unsigned int port, const char * user, const char * password) throw (mysqlException);
	void clean_query();
	inline std::unique_ptr< MYSQL_RES, MYSQL_RES_deleter > store_result() {
		return  std::unique_ptr< MYSQL_RES, MYSQL_RES_deleter > (mysql_store_result(base::get()));
	}
	mysqlException get_mysqlException(const std::string& user_cause = "");
	static mysqlException get_mysqlException(MYSQL* descriptor, const std::string& user_cause = "");
	virtual ~mysqlWrap();
};