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

	s1 <<
	json11::Json(json11::Json::object{
		{ "request_key", 1 },
		{ "cmd", to_string(password_size ? command::authorization : command::registration) },
	}).dump()
	<<
	json11::Json(json11::Json::object{
		{ "login", std::move(login) },
		{ "password", password.size() ? std::move(password) : "p" }
	}).dump();

	std::string response_str;

	s1 >> response_str;
	responseJSON response;
	response.parse(response_str);

	if (password_size) {
		s1 >> response_str;
		AuthResponse authResponse;
		authResponse.parse(response_str);
	}
}

void query_processor(WinSocket sock, mysqlWrap& connection) throw (My::Exception) {
	std::string request_str;
	sock >> request_str;
	server_Request request;
	if (!request.parse(request_str)) return;

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
		static bool online_check(My::Socket& sock, const user_connection_identifier& identifier, int request_key) {
			if (identifier.user_id >= max_user_count) return false;
			if (!users()[identifier.user_id].is_online(identifier.connection_identifier)) {
				sock
				<<
				json11::Json(json11::Json::object {
					{ "request_key", request_key },
					{ "response_status", to_string(QUERY_RESPONSE_STATUS::ERR) },
					{ "response", "user with id \"" + std::to_string(identifier.user_id) + "\" and key \"" + identifier.connection_identifier + "\" is not online" }
				}).dump();

				return false;
			}

			return true;
		}
	};

#pragma region registration
	if (command::registration == cmd) {
		sock >> request_str;
		Auth_Reg_Request reg_request;

		if (!reg_request.parse(request_str)) return;
		
		if (!local_funcs::correct_reg_and_auth_params(sock, reg_request, request.request_key)) return;

		if (!user::regUser(connection, reg_request.login, reg_request.password)) {
			json11::Json json = json11::Json::object {
				{ "request_key", request.request_key },
				{ "response_status", to_string(QUERY_RESPONSE_STATUS::ERR) },
				{ "cause", "user with this loigin already exists" }
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
#pragma endregion
#pragma region authorization
	if (command::authorization == cmd) {
		sock >> request_str;
		Auth_Reg_Request reg_request;

		if (!reg_request.parse(request_str)) return;
		if (!local_funcs::correct_reg_and_auth_params(sock, reg_request, request.request_key)) return;

		std::string authResponse;

		if (!user::authUser(connection, reg_request.login, reg_request.password, authResponse)) {
			json11::Json json = json11::Json::object {
				{ "request_key", request.request_key },
				{ "response_status", to_string(QUERY_RESPONSE_STATUS::ERR) },
				{ "cause", "user with this loigin and password is not exist" }
			};

			sock << json.dump();

			return;
		}

		json11::Json json = json11::Json::object {
			{ "request_key", request.request_key },
			{ "response_status", to_string(QUERY_RESPONSE_STATUS::OK) }
		};

		sock << json.dump() << authResponse;

		return;
	}
#pragma endregion

	sock >> request_str;
	user_connection_identifier identifier;
	if (!identifier.parse(request_str)) return;
	if (!local_funcs::online_check(sock, identifier, request.request_key)) return;

#pragma region send_request_for_friendship
	if (command::send_request_for_friendship == cmd) {
		sock >> request_str;
		send_request_to_friendRequest send_request_to_friend_request;
		if (!send_request_to_friend_request.parse(request_str)) return;

		if (!user::send_request_to_friend(connection, identifier.user_id, send_request_to_friend_request.user_id, send_request_to_friend_request.message)) {
			json11::Json json = json11::Json::object{
				{ "request_key", request.request_key },
				{ "response_status", to_string(QUERY_RESPONSE_STATUS::ERR) },
				{ "cause", "could not add request to friend" }
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
#pragma endregion
#pragma region send_message
	if (command::send_message == cmd) {
		sock >> request_str;
		send_messageRequest send_message_request;
		if (!send_message_request.parse(request_str)) return;

		if (!user::send_message(connection, identifier.user_id, send_message_request.chat_id, send_message_request.message)) {
			json11::Json json = json11::Json::object{
				{ "request_key", request.request_key },
				{ "response_status", to_string(QUERY_RESPONSE_STATUS::ERR) },
				{ "cause", "could not send message" }
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
#pragma endregion
#pragma region set_status_or_avatar
	if (command::set_status == cmd || command::set_avatar == cmd) {
		sock >> request_str;

		if (!user::set_user_str_param(connection, command::set_status == cmd ? "STATUS" : "AVATAR", identifier.user_id, request_str)) {
			json11::Json json = json11::Json::object {
				{ "request_key", request.request_key },
				{ "response_status", to_string(QUERY_RESPONSE_STATUS::ERR) },
				{ "cause", std::string("could not send ") + (command::set_status == cmd ? "status" : "avatar") }
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
#pragma endregion
#pragma region get_user_info
	if (command::get_user_data == cmd) {
		sock >> request_str;
		get_user_infoRequest get_user_info;
		if (!get_user_info.parse(request_str)) return;

		std::string user_info_Response;

		if (!users()[get_user_info.user_id].get_user_info(connection, user_info_Response)) {
			json11::Json json = json11::Json::object {
				{ "request_key", request.request_key },
				{ "response_status", to_string(QUERY_RESPONSE_STATUS::ERR) },
				{ "cause", "could not get user info" }
			};

			sock << json.dump();

			return;
		}

		json11::Json json = json11::Json::object {
			{ "request_key", request.request_key },
			{ "response_status", to_string(QUERY_RESPONSE_STATUS::OK) }
		};

		sock << json.dump() << user_info_Response;

		return;
	}
#pragma endregion
#pragma region get_last_user_tick
	if (command::get_last_user_tick == cmd) {
		sock >> request_str;
		get_user_last_tickRequest get_user_last_tick;
		if (!get_user_last_tick.parse(request_str)) return;

		std::string user_last_tick_Response;

		if (!users()[get_user_last_tick.user_id].get_user_last_tick(connection, user_last_tick_Response)) {
			json11::Json json = json11::Json::object{
				{ "request_key", request.request_key },
				{ "response_status", to_string(QUERY_RESPONSE_STATUS::ERR) },
				{ "cause", "could not get user last_tick" }
			};

			sock << json.dump();

			return;
		}

		json11::Json json = json11::Json::object{
			{ "request_key", request.request_key },
			{ "response_status", to_string(QUERY_RESPONSE_STATUS::OK) }
		};

		sock << json.dump() << user_last_tick_Response;

		return;
	}
#pragma endregion
#pragma region accept_request_for_friendship
	if (command::accept_request_for_friendship == cmd) {
		sock >> request_str;
		return;
	}
#pragma endregion
#pragma region creat_chat
	if (command::creat_chat == cmd) {
		sock >> request_str;
		return;
	}
#pragma endregion
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
void request_to_friend_sended(const int64_t& sender_user_id, const int64_t& receiver_user_id) {
	add_task([sender_user_id, receiver_user_id](mysqlWrap& connection) {

	});
}
void message_sended(int64_t chat_id, int64_t user_id, int64_t message_id) {
	add_task([chat_id, user_id, message_id](mysqlWrap& connection) {

	});
}
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
	*end++ = ',';
	sprintf_s(end, sizeof(query) - (end - query), "%d", max_user_count - 1);
	end += strlen(end);
	*end++ = ')';

	if (mysql_real_query(connection.get(), query, (unsigned int)(end - query))) throw connection.get_mysqlException(__FUNCTION__);

	auto query_res = connection.store_result();

	connection.clean_query();

	return bool(query_res);
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

	return users()[user_id].init(connection, authResponse);
}
bool user::send_request_to_friend(mysqlWrap& connection, const int64_t& sender_user_id, const int64_t& receiver_user_id, const std::string& message) throw (mysqlException) {
	std::string query;
	char *end;
	query.resize(message.size() + 100);

	end = strmov(const_cast< char* >(query.c_str()), "CALL SEND_REQUEST(");
	sprintf_s(end, 100 - (end - query.c_str()), "%d", int(sender_user_id));
	end += strlen(end);
	*end++ = ',';
	sprintf_s(end, 100 - (end - query.c_str()), "%d", int(receiver_user_id));
	end += strlen(end);
	*end++ = ',';
	*end++ = '\'';
	end += mysql_real_escape_string(connection.get(), end, message.c_str(), message.size());
	*end++ = '\'';
	*end++ = ')';

	if (mysql_real_query(connection.get(), query.c_str(), (unsigned int)(end - query.c_str()))) throw connection.get_mysqlException(__FUNCTION__);

	auto query_res = connection.store_result();
	connection.clean_query();

	if (!mysql_num_rows(query_res.get())) return false;

	request_to_friend_sended(sender_user_id, receiver_user_id);
	return true;
}
bool user::send_message(mysqlWrap& connection, const int64_t& user_id, const int64_t& chat_id, const std::string& message) throw (mysqlException) {
	std::string query;
	char *end;
	query.resize(message.size() + 100);

	end = strmov(const_cast< char* >(query.c_str()), "CALL SEND_CHAT_MESSAGE(");
	sprintf_s(end, 100 - (end - query.c_str()), "%d", int(user_id));
	end += strlen(end);
	*end++ = ',';
	sprintf_s(end, 100 - (end - query.c_str()), "%d", int(chat_id));
	end += strlen(end);
	*end++ = ',';
	*end++ = '\'';
	end += mysql_real_escape_string(connection.get(), end, message.c_str(), message.size());
	*end++ = '\'';
	*end++ = ')';

	if (mysql_real_query(connection.get(), query.c_str(), (unsigned int)(end - query.c_str()))) throw connection.get_mysqlException(__FUNCTION__);

	auto query_res = connection.store_result();
	connection.clean_query();

	if (!mysql_num_rows(query_res.get())) return false;
	MYSQL_ROW row = mysql_fetch_row(query_res.get());
	int64_t message_id = atoll(row[0]);

	message_sended(chat_id, user_id, message_id);
	return true;
}
bool user::set_user_str_param(mysqlWrap& connection, const std::string& PARAM_NAME, const int64_t& user_id, const std::string& param) throw (mysqlException) {
	std::string query;
	char *end;
	query.resize(PARAM_NAME.size() + param.size() + 100);

	end = strmov(const_cast< char* >(query.c_str()), "CALL SET_");
	end = strmov(end, PARAM_NAME.c_str());
	*end++ = '(';
	sprintf_s(end, 100 - (end - query.c_str()), "%d", int(user_id));
	end += strlen(end);
	*end++ = ',';
	*end++ = '\'';
	end += mysql_real_escape_string(connection.get(), end, param.c_str(), param.size());
	*end++ = '\'';
	*end++ = ')';

	if (mysql_real_query(connection.get(), query.c_str(), (unsigned int)(end - query.c_str()))) throw connection.get_mysqlException(__FUNCTION__);

	return true;
}
bool user::set_status(mysqlWrap& connection, const int64_t& user_id, const std::string& status) throw (mysqlException) {
	return user::set_user_str_param(connection, "STATUS", user_id, status);
}
bool user::set_avatar(mysqlWrap& connection, const int64_t& user_id, const std::string& avatar) throw (mysqlException) {
	return user::set_user_str_param(connection, "AVATAR", user_id, avatar);
}
//------------------------------------------------------------------------------
bool user::init(mysqlWrap& connection, std::string& authResponse) {
	auto user_id = this - users();
	std::string avatar;
	std::string connection_identifier;

	char query[100], *end;
	
	end = strmov(query, "CALL my_chat.GET_USER_AVATAR(");
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
		
		avatar = maker::make_std_string(*row, *length);

		std::lock_guard< std::mutex > lock(this->context_mutex);
		if (!context) {
			end = strmov(query, "CALL my_chat.GET_MY_INFO(");
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

				context.reset(new user_context(//"warning" ������� ������������� ���������� �� ������� �� ������� ����������
					maker::make_std_string(row[0], length[0]),//log
					maker::make_std_string(row[1], length[1]),//name
					maker::make_std_string(row[2], length[2]),//status
					atoll(row[3]),//n_unread_chats
					atoll(row[4])//n_requests
				));
			}
		}
	}

	connection_identifier = maker::random_str_make(context->individual_number++);

	context->individuals[connection_identifier] = user_context::individual_user_context(context.get());

	json11::Json user_info = json11::Json::object{
		{ "user_id", user_id },
		{ "user_name", context->user_name },
		{ "user_status", context->status },
		{ "unix_time_last_tick", context->last_tick },
		{ "avatar", std::move(avatar) }
	};

	json11::Json authInfo = json11::Json::object{
		{ "user_info", std::move(user_info) },
		{ "n_unread_chats" , context->n_unread_chats },
		{ "n_requests", context->n_requests },
		{ "connection_identifier", std::move(connection_identifier) }
	};

	authResponse = authInfo.dump();

	return true;
}
bool user::is_online(const user_context::individual_user_key& key) {
	std::lock_guard< std::mutex > lock(this->context_mutex);
	if (!context) return false;
	auto iter = context->individuals.find(key);
	if (iter == context->individuals.end()) return false;
	iter->second.refresh_last_tick();
}
bool user::get_user_info(mysqlWrap& connection, std::string& user_info_Response) const throw (mysqlException) {
	auto user_id = this - users();
	std::string log;
	std::string name;
	std::string status;
	std::string avatar;
	int64_t last_tick = 0;

	char query[100], *end;

	end = strmov(query, "CALL my_chat.GET_USER_AVATAR(");
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

		avatar = maker::make_std_string(*row, *length);

		std::unique_lock< std::mutex > lock(this->context_mutex);
		if (!context) {
			lock.unlock();
			end = strmov(query, "CALL my_chat.GET_USER_INFO(");
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

				
				log = maker::make_std_string(row[0], length[0]);
				name = maker::make_std_string(row[1], length[1]);
				status = maker::make_std_string(row[2], length[2]);
				last_tick = atoll(row[3]);
			}
		} else {
			log = context->login;
			name = context->user_name;
			status = context->status;
			last_tick = context->last_tick;
		}
	}

	json11::Json user_info = json11::Json::object{
		{ "user_id", user_id },
		{ "user_name", std::move(name) },
		{ "user_status", std::move(status) },
		{ "unix_time_last_tick", last_tick },
		{ "avatar", std::move(avatar) }
	};

	user_info_Response = user_info.dump();

	return true;
}
bool user::get_user_last_tick(mysqlWrap& connection, std::string& user_last_tick_Response) const throw (mysqlException) {
	auto user_id = this - users();
	int64_t last_tick = 0;

	char query[100], *end;

	std::unique_lock< std::mutex > lock(this->context_mutex);
	if (!context) {
		lock.unlock();
		end = strmov(query, "CALL my_chat.GET_USER_LAST_TICK(");
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

			last_tick = atoll(row[0]);
		}
	} else last_tick = context->last_tick;

	json11::Json user_info = json11::Json::object{
		{ "user_id", user_id },
		{ "unix_time_last_tick", last_tick }
	};

	user_last_tick_Response = user_info.dump();

	return true;
}
//------------------------------------------------------------------------------
user::user_context::individual_user_context::individual_user_context(user::user_context* context) :
	n_unread_chats(context->n_unread_chats),
	n_requests(context->n_requests),
	context(context)
{
	refresh_last_tick();
}

//------------------------------------------------------------------------------