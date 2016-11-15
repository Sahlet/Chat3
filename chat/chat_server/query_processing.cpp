/*
//-------------------
MySQL
http://www.mysql.ru/docs/
//-------------------
MySQL C APIs
http://www.mysql.ru/docs/man/C.html
http://dev.mysql.com/doc/refman/5.5/en/c-api.html
//-------------------
How to store binary data to MySQL
http://zetcode.com/db/mysqlc/
http://www.mysql.ru/docs/man/mysql_real_escape_string.html
//-------------------


*/
#include "query_processing.h"
#include <My\WinSockBaseSerialization.h>
#include "consts.h"
#include "communication_data.h"

using namespace My;
using namespace std;

user::user() {}
user::~user() {}

char* strmov(char* to, const char* from) {
	while ((*to = *from) != '\0') { to++; from++; }
	return to;
}

//------------------------------------------------------------------------------
void test(std::string login, std::string password) {
	WinSocketAddress ad1("127.0.0.1", 30000);
	WinSocket s1;
	s1.connect(ad1);

	size_t password_size = password.size();

	json11::Json json = json11::Json::object{
		{ "login", std::move(login) },
		{ "password", password.size() ? std::move(password) : "p" }
	};

	json = json11::Json::object{
		{ "request_key", 1 },
		{ "cmd", to_string(password_size ? command::authorization : command::registration) },
		{ "request", json.dump() }
	};

	s1 << json.dump();

	std::string response_str;

	s1 >> response_str;

	responseJSON response;
	response.parse(response_str);

	if (password_size) {
		AuthResponse authResponse;
		authResponse.parse(response.response);
	}
}

void query_processor(WinSocket sock, mysqlWrap& connection) throw (My::Exception) {
	std::string str_request;
	sock >> str_request;
	server_Request request;

	if (!request.parse(str_request)) return;

	const command cmd = request.cmd;

	if (command::unknown == cmd) {
		std::cout << "unknown command" << std::endl;
		return;
	}

	struct local_funcs {
		static bool correct_reg_and_auth_params(My::Socket& sock, const Auth_Reg_Request& reg_request, int request_key) {
			if (
				(!reg_request.login.size() || reg_request.login.size() > max_login_len) ||
				(!reg_request.password.size() || reg_request.password.size() > max_password_len)
				)
			{
				std::string resp;
				if (!reg_request.login.size()) resp += "too few login symbols in login";
				else if (reg_request.login.size() > max_login_len) resp += "too many symbols in login";

				if (!reg_request.password.size()) resp += "\ntoo few symbols in password";
				else if (reg_request.password.size() > max_password_len) resp += "too many symbols in password";

				json11::Json json = json11::Json::object({
					{ "request_key", request_key },
					{ "response_status", to_string(QUERY_RESPONSE_STATUS::ERR) },
					{ "response", std::move(resp) }
				});

				sock << json.dump();
				return false;
			}

			return true;
		}
	};

	if (command::registration == cmd) {
		Auth_Reg_Request reg_request;

		if (!reg_request.parse(request.request)) return;
		
		if (!local_funcs::correct_reg_and_auth_params(sock, reg_request, request.request_key)) return;

		if (!user::regUser(connection, reg_request.login, reg_request.password)) {
			json11::Json json = json11::Json::object{
				{ "request_key", request.request_key },
				{ "response_status", to_string(QUERY_RESPONSE_STATUS::ERR) },
				{ "response", "user with this loigin already exists" }
			};

			sock << json.dump();

			return;
		}

		json11::Json json = json11::Json::object {
			{ "request_key", request.request_key },
			{ "response_status", to_string(QUERY_RESPONSE_STATUS::OK) }
		};

		sock << json.dump();

		return;
	}

	if (command::authorization == cmd) {
		Auth_Reg_Request reg_request;

		if (!reg_request.parse(request.request)) return;

		if (!local_funcs::correct_reg_and_auth_params(sock, reg_request, request.request_key)) return;

		std::string authResponse;

		if (!user::authUser(connection, reg_request.login, reg_request.password, authResponse)) {
			json11::Json json = json11::Json::object {
				{ "request_key", request.request_key },
				{ "response_status", to_string(QUERY_RESPONSE_STATUS::ERR) },
				{ "response", "user with this loigin and password is not exist" }
			};

			sock << json.dump();

			return;
		}

		json11::Json json = json11::Json::object {
			{ "request_key", request.request_key },
			{ "response_status", to_string(QUERY_RESPONSE_STATUS::OK) },
			{ "response", std::move(authResponse) }
		};

		sock << json.dump();

		return;
	}
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
struct maker {
	static std::string random_str_make(unsigned short key) {
		std::string rand_str(10, '_');
		for (auto& ch : rand_str) { ch = '0' + rand() % 10; }
		return std::to_string(key) + "_" + rand_str;
	}

	static std::string make_std_string(const char* ch, size_t size) {
		std::string res;
		if (!size) return res;
		res.resize(size);
		char* buf = const_cast< char* >(res.c_str());
		for (auto end = ch + size; ch < end; ch++, buf++) *buf = *ch;
		return res;
	}
};
//------------------------------------------------------------------------------
bool user::existUser(mysqlWrap& connection, const std::string& login) throw (mysqlException) {
	char query[200], *end;

	end = strmov(query, "CALL my_chat.GET_USER_ID('");
	end += mysql_real_escape_string(connection.get(), end, login.c_str(), login.size());
	*end++ = '\'';
	*end++ = ')';

	if (mysql_real_query(connection.get(), query, (unsigned int)(end - query))) throw connection.get_mysqlException(__FUNCTION__);

	auto query_res = connection.store_result();

	connection.clean_query();

	return bool(mysql_num_rows(query_res.get()));
}
bool user::regUser(mysqlWrap& connection, const std::string& login, const std::string& password) throw (mysqlException) {
	char query[300], *end;

	end = strmov(query, "CALL my_chat.ADDUSER('");
	end += mysql_real_escape_string(connection.get(), end, login.c_str(), login.size());
	*end++ = '\'';
	*end++ = ',';
	*end++ = '\'';
	end += mysql_real_escape_string(connection.get(), end, password.c_str(), password.size());
	*end++ = '\'';
	*end++ = ')';

	if (mysql_real_query(connection.get(), query, (unsigned int)(end - query))) throw connection.get_mysqlException(__FUNCTION__);

	auto query_res = connection.store_result();

	connection.clean_query();

	return bool(!query_res);
}
bool user::authUser(mysqlWrap& connection, const std::string& login, const std::string& password, std::string& authResponse) throw (mysqlException) {
	char query[300], *end;

	end = strmov(query, "CALL my_chat.AUTHORIZATION('");
	end += mysql_real_escape_string(connection.get(), end, login.c_str(), login.size());
	*end++ = '\'';
	*end++ = ',';
	*end++ = '\'';
	end += mysql_real_escape_string(connection.get(), end, password.c_str(), password.size());
	*end++ = '\'';
	*end++ = ')';

	if (mysql_real_query(connection.get(), query, (unsigned int)(end - query))) throw connection.get_mysqlException(__FUNCTION__);

	auto query_res = connection.store_result();
	connection.clean_query();

	if (!mysql_num_rows(query_res.get())) return false;

	MYSQL_ROW row = mysql_fetch_row(query_res.get());
	int64_t user_id = atoll(row[0]);

	return users[user_id].init(connection, authResponse);
}
//------------------------------------------------------------------------------
bool user::init(mysqlWrap& connection, std::string& authResponse) {
	auto user_id = this - users;
	std::string avatar;
	std::string connection_identifier;

	std::lock_guard< std::mutex > lock(this->context_mutex);
	
	char query[100], *end;

	end = strmov(query, context ? "CALL my_chat.GET_USER_AVATAR(" : "CALL my_chat.GET_MY_INFO(");
	sprintf_s(end, sizeof(query) - (end - query), "%d", user_id);
	end += strlen(end);
	*end++ = ')';

	if (mysql_real_query(connection.get(), query, (unsigned int)(end - query))) throw connection.get_mysqlException(__FUNCTION__);
	else {
		auto query_res = connection.store_result();
		connection.clean_query();

		if (!mysql_num_rows(query_res.get())) return false;

		MYSQL_ROW row = mysql_fetch_row(query_res.get());
		auto length = mysql_fetch_lengths(query_res.get());

		if (!context) {

			context.reset(new user_context(//"warning" ������� ������������� ���������� �� ������� �� ������� ����������
				maker::make_std_string(row[0], length[0]),
				maker::make_std_string(row[1], length[1]),
				maker::make_std_string(row[2], length[2]),
				atoll(row[3]),
				atoll(row[4])
			));
			row += 5;
			length += 5;
		}
		avatar = maker::make_std_string(*row, *length);
	}

	connection_identifier = maker::random_str_make(context->individual_number++);

	context->individuals[connection_identifier] = user_context::individual_user_context(context.get());

	json11::Json user_info = json11::Json::object {
		{ "user_id", user_id },
		{ "user_name", context->user_name },
		{ "user_status", context->status },
		{ "avatar", std::move(avatar) }
	};

	json11::Json authInfo = json11::Json::object {
		{ "user_info", std::move(user_info) },
		{ "n_unread_chats" , context->n_unread_chats },
		{ "n_requests", context->n_requests },
		{ "connection_identifier", std::move(connection_identifier) }
	};

	authResponse = authInfo.dump();

	return true;
}
//------------------------------------------------------------------------------
user::user_context::individual_user_context::individual_user_context(const user::user_context* context) :
	n_unread_chats(context->n_unread_chats),
	n_requests(context->n_requests)
{}

//------------------------------------------------------------------------------