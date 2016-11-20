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
#include "communication_data.h"

using namespace My;
using namespace std;

user::user() {}
user::~user() {}

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

template< typename T >
bool get_list(mysqlWrap& connection, command cmd, const int64_t& user_id, const T& params, std::string& Response) { return false; }

template<>
bool get_list< get_chat_recordsRequest >(mysqlWrap& connection, command cmd, const int64_t& user_id, const get_chat_recordsRequest& params, std::string& Response) {
	char query[120], *end;

	if (cmd != command::get_chat_members && cmd != command::get_messages) return false;

	end = strmov(query, cmd == command::get_chat_members ? "CALL my_chat.GET_CHAT_MEMBERS(" : "CALL my_chat.GET_MESSAGES(");	
	sprintf_s(end, sizeof(query) - (end - query), "%d", int(user_id));
	end += strlen(end);
	*end++ = ',';
	sprintf_s(end, sizeof(query) - (end - query), "%d", int(params.chat_id));
	end += strlen(end);
	*end++ = ',';
	sprintf_s(end, sizeof(query) - (end - query), "%d", int(params.offset));
	end += strlen(end);
	*end++ = ',';
	sprintf_s(end, sizeof(query) - (end - query), "%d", int(params.count));
	end += strlen(end);
	*end++ = ')';

	if (mysql_real_query(connection.get(), query, (unsigned int)(end - query))) throw connection.get_mysqlException(__FUNCTION__);

	auto query_res = connection.store_result();
	connection.clean_query();

	if (!query_res) return false;

	auto rows_number = mysql_num_rows(query_res.get());
	MYSQL_ROW row;
	unsigned long* length;

	if (cmd == command::get_chat_members) {
		JSONList< chat_memberJSON > res;
		res.items.reserve(rows_number);
		while ((row = mysql_fetch_row(query_res.get())) && (length = mysql_fetch_lengths(query_res.get()))) {
			chat_memberJSON chat_member;
			chat_member.user_id = atoll(row[0]);
			chat_member.unix_time_last_tick = atoll(row[1]);
			chat_member.user_login = maker::make_std_string(row[2], length[2]);
			chat_member.user_name = maker::make_std_string(row[3], length[3]);

			res.items.push_back(std::move(chat_member));
		}
		Response = json11::Json(res).dump();
	} else if (cmd == command::get_messages) {
		JSONList< messageJSON > res;
		res.items.reserve(rows_number);
		while ((row = mysql_fetch_row(query_res.get())) && (length = mysql_fetch_lengths(query_res.get()))) {
			messageJSON message;
			message.message_id = atoll(row[0]);
			message.unread = bool(atoi(row[1]));
			message.user_id = atoll(row[2]);
			message.message = maker::make_std_string(row[3], length[3]);
			message.unix_time_last_tick = atoll(row[4]);

			res.items.push_back(std::move(message));
		}
		Response = json11::Json(res).dump();
	}

	return true;
}
template<>
bool get_list< get_my_recordsRequest >(mysqlWrap& connection, command cmd, const int64_t& user_id, const get_my_recordsRequest& params, std::string& Response) {
	char query[120], *end;

	if (cmd != command::get_requests_for_friendship_from_me && cmd != command::get_requests_for_friendship_to_me && cmd != command::get_my_friends && cmd != command::get_my_chats) return false;

	end = strmov(query, cmd == command::get_requests_for_friendship_to_me ? "CALL my_chat.GET_REQUESTS_TO_ME(" : cmd == command::get_requests_for_friendship_from_me ? "CALL my_chat.GET_REQUESTS_FROM_ME(" : cmd == command::get_my_friends ? "CALL my_chat.GET_MY_FRIENDS(" : "CALL my_chat.GET_CHATS(");
	sprintf_s(end, sizeof(query) - (end - query), "%d", int(user_id));
	end += strlen(end);
	*end++ = ',';
	sprintf_s(end, sizeof(query) - (end - query), "%d", int(params.offset));
	end += strlen(end);
	*end++ = ',';
	sprintf_s(end, sizeof(query) - (end - query), "%d", int(params.count));
	end += strlen(end);
	*end++ = ')';

	if (mysql_real_query(connection.get(), query, (unsigned int)(end - query))) throw connection.get_mysqlException(__FUNCTION__);

	auto query_res = connection.store_result();
	connection.clean_query();

	if (!query_res) return false;

	auto rows_number = mysql_num_rows(query_res.get());
	MYSQL_ROW row;
	unsigned long* length;

	if (cmd == command::get_requests_for_friendship_to_me || cmd == command::get_requests_for_friendship_from_me) {
		JSONList< request_to_friendJSON > res;
		res.items.reserve(rows_number);
		while ((row = mysql_fetch_row(query_res.get())) && (length = mysql_fetch_lengths(query_res.get()))) {
			request_to_friendJSON request_to_friend;
			request_to_friend.info.user_id = atoll(row[0]);
			request_to_friend.info.unix_time_last_tick = atoll(row[1]);
			request_to_friend.info.user_login = maker::make_std_string(row[2], length[2]);
			request_to_friend.info.user_name = maker::make_std_string(row[3], length[3]);
			request_to_friend.request_message = maker::make_std_string(row[4], length[4]);

			res.items.push_back(std::move(request_to_friend));
		}
		Response = json11::Json(res).dump();
	}
	else if (cmd == command::get_my_friends) {
		JSONList< friendJSON > res;
		res.items.reserve(rows_number);
		while ((row = mysql_fetch_row(query_res.get())) && (length = mysql_fetch_lengths(query_res.get()))) {
			friendJSON _friend;
			_friend.info.user_id = atoll(row[0]);
			_friend.info.unix_time_last_tick = atoll(row[1]);
			_friend.info.user_login = maker::make_std_string(row[2], length[2]);
			_friend.info.user_name = maker::make_std_string(row[3], length[3]);
			_friend.chat_id = atoll(row[4]);

			res.items.push_back(std::move(_friend));
		}
		Response = json11::Json(res).dump();
	} else if (cmd == command::get_my_chats) {
		JSONList< chatJSON > res;
		res.items.reserve(rows_number);
		while ((row = mysql_fetch_row(query_res.get())) && (length = mysql_fetch_lengths(query_res.get()))) {
			chatJSON chat;
			chat.chat_id = atoll(row[0]);
			chat.chat_name = maker::make_std_string(row[1], length[1]);
			chat.chat_avatar = maker::make_std_string(row[2], length[2]);
			chat.my_chat_access = EnumSerializer<CHAT_ACCESS>::from_string(maker::make_std_string(row[3], length[3]));
			chat.n_unread_messages = atoll(row[4]);

			res.items.push_back(std::move(chat));
		}
		Response = json11::Json(res).dump();
	}

	return true;
}
template<>
bool get_list< get_user_friendsRequest >(mysqlWrap& connection, command cmd, const int64_t& user_id, const get_user_friendsRequest& params, std::string& Response) {
	char query[120], *end;

	if (cmd != command::get_user_friends) return false;

	end = strmov(query, "CALL my_chat.GET_USER_FRIENDS(");
	sprintf_s(end, sizeof(query) - (end - query), "%d", int(params.user_id));
	end += strlen(end);
	*end++ = ',';
	sprintf_s(end, sizeof(query) - (end - query), "%d", int(params.offset));
	end += strlen(end);
	*end++ = ',';
	sprintf_s(end, sizeof(query) - (end - query), "%d", int(params.count));
	end += strlen(end);
	*end++ = ')';

	if (mysql_real_query(connection.get(), query, (unsigned int)(end - query))) throw connection.get_mysqlException(__FUNCTION__);

	auto query_res = connection.store_result();
	connection.clean_query();

	if (!query_res) return false;

	auto rows_number = mysql_num_rows(query_res.get());
	MYSQL_ROW row;
	unsigned long* length;

	
	JSONList< min_user_infoJSON > res;
	res.items.reserve(rows_number);
	while ((row = mysql_fetch_row(query_res.get())) && (length = mysql_fetch_lengths(query_res.get()))) {
		min_user_infoJSON min_user_info;
		min_user_info.user_id = atoll(row[0]);
		min_user_info.unix_time_last_tick = atoll(row[1]);
		min_user_info.user_login = maker::make_std_string(row[2], length[2]);
		min_user_info.user_name = maker::make_std_string(row[3], length[3]);

		res.items.push_back(std::move(min_user_info));
	}
	Response = json11::Json(res).dump();

	return true;
}
template<>
bool get_list< find_my_recordsRequest >(mysqlWrap& connection, command cmd, const int64_t& user_id, const find_my_recordsRequest& params, std::string& Response) {
	std::string query;
	query.resize(params.regular_str.size() * 2 + 1 + 100);
	char *end;

	if (cmd != command::find_my_friends && cmd != command::find_my_chats && cmd != command::find_users) return false;

	end = strmov(const_cast< char* >(query.c_str()), cmd == command::find_my_friends ? "CALL my_chat.FIND_MY_FRIENDS(" : cmd == command::find_my_chats ? "CALL my_chat.FIND_CHAT(" : "CALL my_chat.FIND_USER(");
	if (cmd != command::find_users) {
		sprintf_s(end, query.size() - (end - query.c_str()), "%d", int(user_id));
		end += strlen(end);
		*end++ = ',';
	}
	*end++ = '\'';
	end += mysql_real_escape_string(connection.get(), end, params.regular_str.c_str(), params.regular_str.size());
	*end++ = '\'';
	*end++ = ',';
	sprintf_s(end, query.size() - (end - query.c_str()), "%d", int(params.offset));
	end += strlen(end);
	*end++ = ',';
	sprintf_s(end, query.size() - (end - query.c_str()), "%d", int(params.count));
	end += strlen(end);
	*end++ = ')';

	if (mysql_real_query(connection.get(), query.c_str(), (unsigned int)(end - query.c_str()))) throw connection.get_mysqlException(__FUNCTION__);

	auto query_res = connection.store_result();
	connection.clean_query();

	if (!query_res) return false;

	auto rows_number = mysql_num_rows(query_res.get());
	MYSQL_ROW row;
	unsigned long* length;

	if (cmd == command::find_my_friends) {
		JSONList< friendJSON > res;
		res.items.reserve(rows_number);
		while ((row = mysql_fetch_row(query_res.get())) && (length = mysql_fetch_lengths(query_res.get()))) {
			friendJSON _friend;
			_friend.info.user_id = atoll(row[0]);
			_friend.info.unix_time_last_tick = atoll(row[1]);
			_friend.info.user_login = maker::make_std_string(row[2], length[2]);
			_friend.info.user_name = maker::make_std_string(row[3], length[3]);
			_friend.chat_id = atoll(row[4]);

			res.items.push_back(std::move(_friend));
		}
		Response = json11::Json(res).dump();
	} else if (cmd == command::find_my_chats) {
		JSONList< chatJSON > res;
		res.items.reserve(rows_number);
		while ((row = mysql_fetch_row(query_res.get())) && (length = mysql_fetch_lengths(query_res.get()))) {
			chatJSON chat;
			chat.chat_id = atoll(row[0]);
			chat.chat_name = maker::make_std_string(row[1], length[1]);
			chat.chat_avatar = maker::make_std_string(row[2], length[2]);
			chat.my_chat_access = EnumSerializer<CHAT_ACCESS>::from_string(maker::make_std_string(row[3], length[3]));
			chat.n_unread_messages = atoll(row[4]);

			res.items.push_back(std::move(chat));
		}
		Response = json11::Json(res).dump();
	} else if (cmd == command::find_users) {
		JSONList< min_user_infoJSON > res;
		res.items.reserve(rows_number);
		while ((row = mysql_fetch_row(query_res.get())) && (length = mysql_fetch_lengths(query_res.get()))) {
			min_user_infoJSON min_user_info;
			min_user_info.user_id = atoll(row[0]);
			min_user_info.unix_time_last_tick = atoll(row[1]);
			min_user_info.user_login = maker::make_std_string(row[2], length[2]);
			min_user_info.user_name = maker::make_std_string(row[3], length[3]);

			res.items.push_back(std::move(min_user_info));
		}
		Response = json11::Json(res).dump();
	}

	return true;
}
template<>
bool get_list< find_user_friendsRequest >(mysqlWrap& connection, command cmd, const int64_t& user_id, const find_user_friendsRequest& params, std::string& Response) {
	std::string query;
	query.resize(params.regular_str.size() * 2 + 1 + 100);
	char *end;

	if (cmd != command::find_user_friends) return false;

	end = strmov(const_cast< char* >(query.c_str()), "CALL my_chat.FIND_USER_FRIENDS(");
	sprintf_s(end, query.size() - (end - query.c_str()), "%d", int(params.user_id));
	end += strlen(end);
	*end++ = ',';
	*end++ = '\'';
	end += mysql_real_escape_string(connection.get(), end, params.regular_str.c_str(), params.regular_str.size());
	*end++ = '\'';
	*end++ = ',';
	sprintf_s(end, query.size() - (end - query.c_str()), "%d", int(params.offset));
	end += strlen(end);
	*end++ = ',';
	sprintf_s(end, query.size() - (end - query.c_str()), "%d", int(params.count));
	end += strlen(end);
	*end++ = ')';

	if (mysql_real_query(connection.get(), query.c_str(), (unsigned int)(end - query.c_str()))) throw connection.get_mysqlException(__FUNCTION__);

	auto query_res = connection.store_result();
	connection.clean_query();

	if (!query_res) return false;

	auto rows_number = mysql_num_rows(query_res.get());
	MYSQL_ROW row;
	unsigned long* length;

	JSONList< min_user_infoJSON > res;
	res.items.reserve(rows_number);
	while ((row = mysql_fetch_row(query_res.get())) && (length = mysql_fetch_lengths(query_res.get()))) {
		min_user_infoJSON min_user_info;
		min_user_info.user_id = atoll(row[0]);
		min_user_info.unix_time_last_tick = atoll(row[1]);
		min_user_info.user_login = maker::make_std_string(row[2], length[2]);
		min_user_info.user_name = maker::make_std_string(row[3], length[3]);

		res.items.push_back(std::move(min_user_info));
	}
	Response = json11::Json(res).dump();

	return true;
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
				std::string cause;
				if (!reg_request.login.size()) cause += "too few login symbols in login";
				else if (reg_request.login.size() > max_login_len) cause += "too many symbols in login";

				if (!reg_request.password.size()) cause += "\ntoo few symbols in password";
				else if (reg_request.password.size() > max_password_len) cause += "too many symbols in password";

				sock <<
					responseJSON(
						request_key,
						QUERY_RESPONSE_STATUS::ERR,
						std::move(cause)
					).dump();

				return false;
			}

			return true;
		}
		static bool online_check(My::Socket& sock, const user_connection_identifier& identifier, int request_key) {
			if (identifier.user_id >= max_user_count) return false;
			if (!users()[identifier.user_id].is_online(identifier.connection_identifier)) {
				sock <<
					responseJSON(
						request_key,
						QUERY_RESPONSE_STATUS::ERR,
						"user with id \"" + std::to_string(identifier.user_id) + "\" and key \"" + identifier.connection_identifier + "\" is not online"
					).dump();				

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
			sock <<
			responseJSON(
				request.request_key,
				QUERY_RESPONSE_STATUS::ERR,
				"user with this loigin already exists"
			).dump();

			return;
		}


		sock <<
		responseJSON(
			request.request_key,
			QUERY_RESPONSE_STATUS::OK
		).dump();

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
			sock <<
			responseJSON(
				request.request_key,
				QUERY_RESPONSE_STATUS::ERR,
				"user with this loigin and password is not exist"
			).dump();

			return;
		}

		sock <<
		responseJSON(
			request.request_key,
			QUERY_RESPONSE_STATUS::OK
		).dump()
		<< authResponse;

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
			sock <<
				responseJSON(
					request.request_key,
					QUERY_RESPONSE_STATUS::ERR,
					"could not add request to friend"
				).dump();

			return;
		}

		sock <<
			responseJSON(
				request.request_key,
				QUERY_RESPONSE_STATUS::OK
			).dump();

		return;
	}
#pragma endregion
#pragma region send_message
	if (command::send_message == cmd) {
		sock >> request_str;
		send_messageRequest send_message_request;
		if (!send_message_request.parse(request_str)) return;

		if (!user::send_message(connection, identifier.user_id, send_message_request.chat_id, send_message_request.message)) {
			sock <<
				responseJSON(
					request.request_key,
					QUERY_RESPONSE_STATUS::ERR,
					"could not send message"
				).dump();

			return;
		}

		sock <<
			responseJSON(
				request.request_key,
				QUERY_RESPONSE_STATUS::OK
			).dump();

		return;
	}
#pragma endregion
#pragma region set_status_or_avatar
	if (command::set_status == cmd || command::set_avatar == cmd) {
		sock >> request_str;

		if (!user::set_user_str_param(connection, command::set_status == cmd ? "STATUS" : "AVATAR", identifier.user_id, request_str)) {
			sock <<
				responseJSON(
					request.request_key,
					QUERY_RESPONSE_STATUS::ERR,
					std::string("could not send ") + (command::set_status == cmd ? "status" : "avatar")
				).dump();

			return;
		}

		sock <<
			responseJSON(
				request.request_key,
				QUERY_RESPONSE_STATUS::OK
			).dump();

		return;
	}
#pragma endregion
#pragma region accept_request_for_friendship
	if (command::accept_request_for_friendship == cmd) {
		sock >> request_str;
		accept_request_for_friendRequest accept_request_for_friend;
		if (!accept_request_for_friend.parse(request_str)) return;

		int64_t friend_chat_id;

		if (!user::accept_request_for_friend(connection, identifier.user_id, accept_request_for_friend.user_id, friend_chat_id)) {
			sock <<
				responseJSON(
					request.request_key,
					QUERY_RESPONSE_STATUS::ERR,
					"could not accept request for friendship"
				).dump();

			return;
		}

		
		sock <<
			responseJSON(
				request.request_key,
				QUERY_RESPONSE_STATUS::OK
			).dump()
		<<
			json11::Json(json11::Json::object{
				{ "chat_id", friend_chat_id }
			}).dump();

		return;
	}
#pragma endregion
#pragma region creat_chat
	if (command::create_chat == cmd) {
		sock >> request_str;
		create_chateRequest create_chate;
		if (!create_chate.parse(request_str)) return;

		if (create_chate.chat_name.size() > 125) {
			sock <<
				responseJSON(
					request.request_key,
					QUERY_RESPONSE_STATUS::ERR,
					"too long chat name"
				).dump();

			return;
		}

		int64_t chat_id;
		if (!user::create_chat(connection, identifier.user_id, create_chate.chat_name, create_chate.chat_avatar, chat_id)) {
			sock <<
				responseJSON(
					request.request_key,
					QUERY_RESPONSE_STATUS::ERR,
					"could not accept creat chat"
				).dump();

			return;
		}

		sock <<
			responseJSON(
				request.request_key,
				QUERY_RESPONSE_STATUS::OK
			).dump()
		<<
		json11::Json(json11::Json::object{
			{ "chat_id", chat_id }
		}).dump();

		return;
	}
#pragma endregion
#pragma region add_chat_member
	if (command::add_chat_member == cmd) {
		sock >> request_str;
		add_chat_memberRequest add_chat_member;
		if (!add_chat_member.parse(request_str)) return;

		std::string cause;
		if (add_chat_member.chat_access == CHAT_ACCESS::NO_ACCESS || !user::add_chat_member(connection, identifier.user_id, add_chat_member.chat_id, add_chat_member.user_id, add_chat_member.chat_access)) {
			sock <<
				responseJSON(
					request.request_key,
					QUERY_RESPONSE_STATUS::ERR,
					"could not add chat member"
				).dump();

			return;
		}

		sock <<
			responseJSON(
				request.request_key,
				QUERY_RESPONSE_STATUS::OK
			).dump();

		return;
	}
#pragma endregion
#pragma region set_chat_member_access
	if (command::set_chat_member_access == cmd) {
		sock >> request_str;
		set_chat_member_accessRequest set_chat_member_access;
		if (!set_chat_member_access.parse(request_str)) return;

		std::string cause;
		if (!user::set_chat_member_access(connection, identifier.user_id, set_chat_member_access.chat_id, set_chat_member_access.user_id, set_chat_member_access.chat_access)) {
			sock <<
				responseJSON(
					request.request_key,
					QUERY_RESPONSE_STATUS::ERR,
					"could not set chat member access"
				).dump();

			return;
		}

		sock <<
			responseJSON(
				request.request_key,
				QUERY_RESPONSE_STATUS::OK
			).dump();

		return;
	}
#pragma endregion
#pragma region get_user_info
	if (command::get_user_info == cmd) {
		sock >> request_str;
		get_user_infoRequest get_user_info;
		if (!get_user_info.parse(request_str)) return;

		std::string user_info_Response;

		if (!users()[get_user_info.user_id].get_user_info(connection, user_info_Response)) {
			sock <<
				responseJSON(
					request.request_key,
					QUERY_RESPONSE_STATUS::ERR,
					"could not get user info"
				).dump();

			return;
		}

		sock <<
			responseJSON(
				request.request_key,
				QUERY_RESPONSE_STATUS::OK
			).dump()
		<< user_info_Response;

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
			sock <<
				responseJSON(
					request.request_key,
					QUERY_RESPONSE_STATUS::ERR,
					"could not get user last_tick"
				).dump();

			return;
		}

		sock <<
			responseJSON(
				request.request_key,
				QUERY_RESPONSE_STATUS::OK
			).dump()
		<< user_last_tick_Response;

		return;
	}
#pragma endregion

#pragma region get_chat_members get_messages
	if (command::get_chat_members == cmd || command::get_messages == cmd) {
		sock >> request_str;
		get_chat_recordsRequest get_chat_records;
		if (!get_chat_records.parse(request_str)) return;

		std::string Response;

		if (!get_list(connection, cmd, identifier.user_id, get_chat_records, Response)) {
			sock <<
				responseJSON(
					request.request_key,
					QUERY_RESPONSE_STATUS::ERR,
					"could not get chat records"
				).dump();

			return;
		}

		sock <<
			responseJSON(
				request.request_key,
				QUERY_RESPONSE_STATUS::OK
			).dump()
		<< Response;

		return;
	}
#pragma endregion
#pragma region get_requests_for_friendship_to_me get_requests_for_friendship_from_me get_my_friends get_my_chats
	if (command::get_requests_for_friendship_to_me == cmd || command::get_requests_for_friendship_from_me == cmd || command::get_my_friends == cmd || command::get_my_chats == cmd) {
		sock >> request_str;
		get_my_recordsRequest get_my_records;
		if (!get_my_records.parse(request_str)) return;

		std::string Response;

		if (!get_list(connection, cmd, identifier.user_id, get_my_records, Response)) {
			sock <<
				responseJSON(
					request.request_key,
					QUERY_RESPONSE_STATUS::ERR,
					"could not get my records"
				).dump();

			return;
		}

		sock <<
			responseJSON(
				request.request_key,
				QUERY_RESPONSE_STATUS::OK
			).dump()
		<< Response;

		return;
	}
#pragma endregion
#pragma region get_user_friends
	if (command::get_user_friends == cmd) {
		sock >> request_str;
		get_user_friendsRequest get_user_friends;
		if (!get_user_friends.parse(request_str)) return;

		std::string Response;

		if (!get_list(connection, cmd, identifier.user_id, get_user_friends, Response)) {
			sock <<
				responseJSON(
					request.request_key,
					QUERY_RESPONSE_STATUS::ERR,
					"could not get user records"
				).dump();

			return;
		}

		sock <<
			responseJSON(
				request.request_key,
				QUERY_RESPONSE_STATUS::OK
			).dump()
		<< Response;

		return;
	}
#pragma endregion
#pragma region find_my_friends, find_my_chats, find_users
	if (command::find_my_friends == cmd || command::find_my_chats == cmd || command::find_users == cmd) {
		sock >> request_str;
		find_my_recordsRequest find_my_records;
		if (!find_my_records.parse(request_str)) return;

		std::string Response;

		if (!get_list(connection, cmd, identifier.user_id, find_my_records, Response)) {
			sock <<
				responseJSON(
					request.request_key,
					QUERY_RESPONSE_STATUS::ERR,
					"could not find my records"
				).dump();

			return;
		}

		sock <<
			responseJSON(
				request.request_key,
				QUERY_RESPONSE_STATUS::OK
			).dump()
		<< Response;

		return;
	}
#pragma endregion
#pragma region find_user_friends
	if (command::find_user_friends == cmd) {
		sock >> request_str;
		find_user_friendsRequest find_user_friends;
		if (!find_user_friends.parse(request_str)) return;

		std::string Response;

		if (!get_list(connection, cmd, identifier.user_id, find_user_friends, Response)) {
			sock <<
				responseJSON(
					request.request_key,
					QUERY_RESPONSE_STATUS::ERR,
					"could not find user friends"
				).dump();
			
			return;
		}

		sock <<
			responseJSON(
				request.request_key,
				QUERY_RESPONSE_STATUS::OK
			).dump()
		<< Response;

		return;
	}
#pragma endregion

	//polling
}

//------------------------------------------------------------------------------
void request_to_friend_sended(const int64_t& sender_user_id, const int64_t& receiver_user_id) {
	add_task([sender_user_id, receiver_user_id](mysqlWrap& connection) {

	});
}
void message_sended(int64_t chat_id, int64_t user_id, int64_t message_id) {
	add_task([chat_id, user_id, message_id](mysqlWrap& connection) {

	});
}
void request_to_friend_accepted(const int64_t& user_id, const int64_t& requester_id, const int64_t& friend_chat_id) {
	add_task([user_id, requester_id, friend_chat_id](mysqlWrap& connection) {

	});
}
void chat_member_added(const int64_t& adder_user_id, const int64_t& chat_id, const int64_t& member_id) {
	add_task([adder_user_id, chat_id, member_id](mysqlWrap& connection) {

	});
}
void user_online(const int64_t& user_id, const time_t& time) {
	add_task([user_id, time](mysqlWrap& connection) {
		char query[100];
		char *end;

		end = strmov(const_cast< char* >(query), "CALL my_chat.USER_ONLINE(");
		sprintf_s(end, sizeof(query) - (end - query), "%d", int(user_id));
		end += strlen(end);
		*end++ = ',';
		sprintf_s(end, sizeof(query) - (end - query), "%d", int(time));
		end += strlen(end);
		*end++ = ')';

		if (mysql_real_query(connection.get(), query, (unsigned int)(end - query))) throw connection.get_mysqlException(__FUNCTION__);

		connection.clean_query();
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

	return bool(query_res && mysql_num_rows(query_res.get()));
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

	if (!query_res || !mysql_num_rows(query_res.get())) return false;

	MYSQL_ROW row = mysql_fetch_row(query_res.get());
	int64_t user_id = atoll(row[0]);

	return users()[user_id].init(connection, authResponse);
}
bool user::send_request_to_friend(mysqlWrap& connection, const int64_t& sender_user_id, const int64_t& receiver_user_id, const std::string& message) throw (mysqlException) {
	std::string query;
	char *end;
	query.resize(message.size()*2 + 1 + 100);

	end = strmov(const_cast< char* >(query.c_str()), "CALL my_chat.SEND_REQUEST(");
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

	if (!query_res || !mysql_num_rows(query_res.get())) return false;

	request_to_friend_sended(sender_user_id, receiver_user_id);
	return true;
}
bool user::send_message(mysqlWrap& connection, const int64_t& user_id, const int64_t& chat_id, const std::string& message) throw (mysqlException) {
	std::string query;
	char *end;
	query.resize(message.size()*2 + 1 + 100);

	end = strmov(const_cast< char* >(query.c_str()), "CALL my_chat.SEND_CHAT_MESSAGE(");
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

	if (!query_res || !mysql_num_rows(query_res.get())) return false;
	MYSQL_ROW row = mysql_fetch_row(query_res.get());
	int64_t message_id = atoll(row[0]);

	message_sended(chat_id, user_id, message_id);
	return true;
}
bool user::set_user_str_param(mysqlWrap& connection, const std::string& PARAM_NAME, const int64_t& user_id, const std::string& param) throw (mysqlException) {
	std::string query;
	char *end;
	query.resize(PARAM_NAME.size() + param.size()*2 + 1 + 100);

	end = strmov(const_cast< char* >(query.c_str()), "CALL my_chat.SET_");
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
bool user::accept_request_for_friend(mysqlWrap& connection, const int64_t& user_id, const int64_t& requester_id, int64_t& friend_chat_id) throw (mysqlException) {
	char query[100];
	char *end;

	end = strmov(const_cast< char* >(query), "CALL my_chat.ACCEPT_REQUEST(");
	sprintf_s(end, sizeof(query) - (end - query), "%d", int(user_id));
	end += strlen(end);
	*end++ = ',';
	sprintf_s(end, sizeof(query) - (end - query), "%d", int(requester_id));
	end += strlen(end);
	*end++ = ')';

	if (mysql_real_query(connection.get(), query, (unsigned int)(end - query))) throw connection.get_mysqlException(__FUNCTION__);

	auto query_res = connection.store_result();
	connection.clean_query();

	if (!query_res || !mysql_num_rows(query_res.get())) return false;
	MYSQL_ROW row = mysql_fetch_row(query_res.get());
	friend_chat_id = atoll(row[0]);

	request_to_friend_accepted(user_id, requester_id, friend_chat_id);
	return true;
}
bool user::create_chat(mysqlWrap& connection, const int64_t& user_id, const std::string& chat_name, const std::string& chat_avatar, int64_t& chat_id) throw (mysqlException) {
	std::string query;
	char *end;
	query.resize((chat_name.size() + chat_avatar.size())*2 + 1 + 100);

	end = strmov(const_cast< char* >(query.c_str()), "CALL my_chat.CREATE_CHAT(");
	sprintf_s(end, 100 - (end - query.c_str()), "%d", int(user_id));
	end += strlen(end);
	*end++ = ',';
	*end++ = '\'';
	end += mysql_real_escape_string(connection.get(), end, chat_name.c_str(), chat_name.size());
	*end++ = '\'';
	*end++ = ',';
	*end++ = '\'';
	end += mysql_real_escape_string(connection.get(), end, chat_avatar.c_str(), chat_avatar.size());
	*end++ = '\'';
	*end++ = ')';

	if (mysql_real_query(connection.get(), query.c_str(), (unsigned int)(end - query.c_str()))) throw connection.get_mysqlException(__FUNCTION__);

	auto query_res = connection.store_result();
	connection.clean_query();

	if (!query_res || !mysql_num_rows(query_res.get())) return false;
	MYSQL_ROW row = mysql_fetch_row(query_res.get());
	chat_id = atoll(row[0]);

	return true;
}
bool user::add_chat_member(mysqlWrap& connection, const int64_t& adder_user_id, const int64_t& chat_id, const int64_t& user_id, CHAT_ACCESS access) throw (mysqlException) {
	char query[150];
	char *end;

	end = strmov(const_cast< char* >(query), "CALL my_chat.ADD_MEMBER(");
	sprintf_s(end, sizeof(query) - (end - query), "%d", int(adder_user_id));
	end += strlen(end);
	*end++ = ',';
	sprintf_s(end, sizeof(query) - (end - query), "%d", int(chat_id));
	end += strlen(end);
	*end++ = ',';
	sprintf_s(end, sizeof(query) - (end - query), "%d", int(user_id));
	end += strlen(end);
	*end++ = ',';
	std::string str_access = to_string(access);
	sprintf_s(end, sizeof(query) - (end - query), "%s", str_access.c_str());
	end += strlen(end);
	*end++ = ')';

	if (mysql_real_query(connection.get(), query, (unsigned int)(end - query))) throw connection.get_mysqlException(__FUNCTION__);

	auto query_res = connection.store_result();
	connection.clean_query();

	if (!query_res) return false;

	chat_member_added(adder_user_id, chat_id, user_id);
	return true;
}
bool user::set_chat_member_access(mysqlWrap& connection, const int64_t& setter_user_id, const int64_t& chat_id, const int64_t& member_user_id, CHAT_ACCESS access) throw (mysqlException) {
	char query[150];
	char *end;

	end = strmov(const_cast< char* >(query), "CALL my_chat.SET_MEMBER_ACCESS(");
	sprintf_s(end, sizeof(query) - (end - query), "%d", int(setter_user_id));
	end += strlen(end);
	*end++ = ',';
	sprintf_s(end, sizeof(query) - (end - query), "%d", int(chat_id));
	end += strlen(end);
	*end++ = ',';
	sprintf_s(end, sizeof(query) - (end - query), "%d", int(member_user_id));
	end += strlen(end);
	*end++ = ',';
	std::string str_access = to_string(access);
	sprintf_s(end, sizeof(query) - (end - query), "%s", str_access.c_str());
	end += strlen(end);
	*end++ = ')';

	if (mysql_real_query(connection.get(), query, (unsigned int)(end - query))) throw connection.get_mysqlException(__FUNCTION__);

	auto query_res = connection.store_result();
	connection.clean_query();

	if (!query_res) return false;

	return true;
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

				context.reset(new user_context(//"warning" порядок инициализации параметров не зависит от порядка аргументов
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

	min_user_infoJSON min_user_info(
		user_id,
		context->last_tick,
		context->login,
		context->user_name
	);

	user_infoJSON user_info(
		std::move(min_user_info),
		context->status,
		std::move(avatar)
	);

	AuthResponse authInfo(
		std::move(user_info),
		context->n_unread_chats,
		context->n_requests,
		std::move(connection_identifier)
	);

	authResponse = authInfo.dump();

	return true;
}
bool user::is_online(const user_context::individual_user_key& key) {
	std::lock_guard< std::mutex > lock(this->context_mutex);
	if (!context) return false;
	auto iter = context->individuals.find(key);
	if (iter == context->individuals.end()) return false;
	iter->second.refresh_last_tick();
	user_online(this - users(), iter->second.last_tick);
	return true;
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

	min_user_infoJSON min_user_info(
		user_id,
		last_tick,
		std::move(log),
		std::move(name)
	);

	user_infoJSON user_info(
		std::move(min_user_info),
		std::move(status),
		std::move(avatar)
	);

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

	user_last_tickJSON user_last_tick(
		user_id,
		last_tick
	);

	user_last_tick_Response = user_last_tick.dump();

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