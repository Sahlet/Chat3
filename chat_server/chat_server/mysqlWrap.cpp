#include "mysqlWrap.h"
#pragma comment(lib, "libmysql.lib") // подключаем библиотеку

mysqlException::mysqlException(int errorCode, std::string errorCause) : std::exception("mysqlException"), errorCode(errorCode), errorCause(std::move(errorCause)) {}
mysqlException::mysqlException(std::string errorCause, int errorCode) : std::exception("mysqlException"), errorCode(errorCode), errorCause(std::move(errorCause)) {}

mysqlWrap::mysqlWrap(const char * host, unsigned int port, const char * user, const char * password) throw (mysqlException) {
	if (!mysql_init(&descriptor)) throw mysqlException("can't create MYSQL descriptor.");

	if (!mysql_real_connect(&descriptor, host, user, password, NULL, port, NULL, CLIENT_MULTI_STATEMENTS)) mysqlException(mysql_errno(&descriptor), mysql_error(&descriptor));
}

mysqlWrap::~mysqlWrap() {
	mysql_close(&descriptor);
}
