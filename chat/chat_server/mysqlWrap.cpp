#include "mysqlWrap.h"
#pragma comment(lib, "libmysql.lib") // подключаем библиотеку

mysqlException::mysqlException(long long errorCode, std::string errorCause, const char* what) : Exception(errorCode, std::move(errorCause), what) {}
mysqlException::mysqlException(long long errorCode, std::string errorCause) : Exception(errorCode, std::move(errorCause), "mysqlException") {}
mysqlException::mysqlException(std::string errorCause, long long errorCode) : Exception(errorCode, std::move(errorCause), "mysqlException") {}

mysqlException mysqlWrap::get_mysqlException(const std::string& user_cause) {
	return get_mysqlException(base::get(), user_cause);
}

void mysqlWrap::clean_query() {
	int value;
	while (!(value = mysql_next_result(base::get()))) {
		store_result();
	}
}

mysqlException mysqlWrap::get_mysqlException(MYSQL* descriptor, const std::string& user_cause) {
	if (user_cause.size()) return mysqlException(mysql_errno(descriptor), user_cause + " | " + mysql_error(descriptor));
	else return mysqlException(mysql_errno(descriptor), mysql_error(descriptor));
}

mysqlWrap::mysqlWrap(const char * host, unsigned int port, const char * user, const char * password) throw (mysqlException) {
	base::reset(mysql_init(NULL));
	if (!base::get())
		throw mysqlException("can't create MYSQL descriptor.");

	if (!mysql_real_connect(base::get(), host, user, password, NULL, port, NULL, CLIENT_MULTI_STATEMENTS))
		throw mysqlException(mysql_errno(base::get()), mysql_error(base::get()));
}

mysqlWrap::~mysqlWrap() {
	if (base::get()) mysql_close(base::release());
}
