#pragma once

enum class command : int {
	unknown,
	authorization,
	registration,
	//---------------------
	polling, //опрос изменений
	//---------------------
	set_status,
	set_avatar,
	//---------------------
	send_message,
	send_request_for_friendship,
	accept_request_for_friendship,
	creat_chat,
	//---------------------
	// chat modifies
	add_chat_member,
	set_chat_member_access,
	//---------------------
	get_user_data,
	get_last_user_tick,
	//---------------------
	get_chat_members,
	get_requests_for_friendship,
	get_messages,
	get_friens,
	get_chats,
	get_users
};

enum class QUERY_RESPONSE_STATUS : int { ERR, OK };
enum class CHAT_ACCESS : int { NO_ACCESS, USER, PRIVILEGED_USER, ADMIN };

const char* comand_bite_interpretation(command cmd);
command comand_bite_interpretation(const char* cmd);

//максимальное количество байт для логина, максимальное количество байт для пароля.
const int max_login_len = 40, max_password_len = 40;
