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
	create_chat,
	//---------------------
	// chat modifies
	add_chat_member,
	set_chat_member_access,
	//---------------------
	get_user_info,
	get_last_user_tick,
	//---------------------
	get_chat_members,
	get_requests_for_friendship_to_me,
	get_requests_for_friendship_from_me,
	get_messages,
	get_my_friends,
	get_user_friends,
	get_my_chats,
	find_my_friends,
	find_my_chats,
	find_users,
	find_user_friends
};

enum class QUERY_RESPONSE_STATUS : int { ERR, OK };
enum class CHAT_ACCESS : int { NO_ACCESS, USER, PRIVILEGED_USER, ADMIN };

const char* comand_bite_interpretation(command cmd);
command comand_bite_interpretation(const char* cmd);

//максимальное количество байт для логина, максимальное количество байт для пароля.
const int max_login_len = 40, max_password_len = 40;

#define END_OF_QUERY_PROCESSING "___END_OF_QUERY_PROCESSING___"
