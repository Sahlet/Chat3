#pragma once
#include"data_serialization\JSONReader.h"
#include"consts.h"

typedef std::string image_t;

#define EnumSerializer_declaration(enum_type)							\
template<>																\
struct EnumSerializer< enum_type > {									\
	static enum_type from_string(const std::string& str);				\
	static std::string to_string(const enum_type& enm);					\
};

EnumSerializer_declaration(command);
EnumSerializer_declaration(QUERY_RESPONSE_STATUS);
EnumSerializer_declaration(CHAT_ACCESS);

#pragma region CLIENT CLASSES
//------------------------------------------------------------------------------
#define responseJSON_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(int, request_key, "request_key")\
		DEFINITION_NAME(QUERY_RESPONSE_STATUS, status, "response_status")\
		DEFINITION_NAME(std::string, cause, "cause")
JSON_class_gen(responseJSON, responseJSON_LIST);
//------------------------------------------------------------------------------
#define user_infoJSON_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(int64_t, user_id, "user_id")\
		DEFINITION_NAME(std::string, user_name, "user_name")\
		DEFINITION_NAME(std::string, status, "user_status")\
		DEFINITION_NAME(std::string, avatar, "avatar")
JSON_class_gen(user_infoJSON, user_infoJSON_LIST);
//------------------------------------------------------------------------------
#define AuthResponse_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(user_infoJSON, user_info, "user_info")\
		DEFINITION_NAME(int, n_unread_chats, "n_unread_chats")\
		DEFINITION_NAME(int, n_requests, "n_requests")\
		DEFINITION_NAME(std::string, connection_identifier, "connection_identifier")
JSON_class_gen(AuthResponse, AuthResponse_LIST);
//------------------------------------------------------------------------------
#define chatJSON_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(int64_t, chat_id, "chat_id")\
		DEFINITION_NAME(std::string, chat_name, "chat_name")\
		DEFINITION_NAME(std::string, chat_avatar, "chat_avatar")
JSON_class_gen(chatJSON, chatJSON_LIST);
JSONList_gen(chatJSON, "chats");
//------------------------------------------------------------------------------
#define chat_memberJSON_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(int64_t, chat_id, "chat_id")\
		DEFINITION_NAME(int64_t, user_id, "user_id")\
		DEFINITION_NAME(std::string, user_name, "user_name")
JSON_class_gen(chat_memberJSON, chat_memberJSON_LIST);
JSONList_gen(chat_memberJSON, "chat_members");
//------------------------------------------------------------------------------
#define chat_member_for_adminJSON_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(chat_memberJSON, chat_member, "chat_member")\
		DEFINITION_NAME(CHAT_ACCESS, access, "access")
JSON_class_gen(chat_member_for_adminJSON, chat_member_for_adminJSON_LIST);
JSONList_gen(chat_member_for_adminJSON, "chat_members");
//------------------------------------------------------------------------------
#define messageJSON_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(int64_t, chat_id, "chat_id")\
		DEFINITION_NAME(int64_t, user_id, "user_id")\
		DEFINITION_NAME(int64_t, message_id, "message_id")\
		DEFINITION_NAME(std::string, message, "message")\
		DEFINITION_NAME(std::string, last_tick, "last_tick")\
		DEFINITION_NAME(bool, unread, "unread")
JSON_class_gen(messageJSON, messageJSON_LIST);
JSONList_gen(messageJSON, "messages");
//------------------------------------------------------------------------------
#define friendJSON_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(int64_t, chat_id, "chat_id")\
		DEFINITION_NAME(int64_t, user_id, "user_id")\
		DEFINITION_NAME(std::string, user_name, "user_name")
JSON_class_gen(friendJSON, friendJSON_LIST);
JSONList_gen(friendJSON, "friends");
//------------------------------------------------------------------------------
#define request_to_friendJSON_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(int64_t, user_id, "user_id")\
		DEFINITION_NAME(std::string, user_name, "user_name")\
		DEFINITION_NAME(std::string, request_message, "request_message")
JSON_class_gen(request_to_friendJSON, request_to_friendJSON_LIST);
JSONList_gen(request_to_friendJSON, "requests_to_friend");
//------------------------------------------------------------------------------
#define Response_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(int, request_key, "request_key")\
		DEFINITION_NAME(std::string, response, "response")
JSON_class_gen(Response, Response_LIST, ;);
//------------------------------------------------------------------------------
#pragma endregion

#pragma region SERVER CLASSES
//------------------------------------------------------------------------------
#define server_Request_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(int, request_key, "request_key") /*to make relation between response and request*/ \
		DEFINITION_NAME(command, cmd, "cmd")
JSON_class_gen(server_Request, server_Request_LIST);
//------------------------------------------------------------------------------
#define Auth_Reg_Request_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(std::string, login, "login")\
		DEFINITION_NAME(std::string, password, "password")
JSON_class_gen(Auth_Reg_Request, Auth_Reg_Request_LIST);
//------------------------------------------------------------------------------
#define user_connection_identifier_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(int64_t, user_id, "user_id")\
		DEFINITION_NAME(std::string, connection_identifier, "connection_identifier")
JSON_class_gen(user_connection_identifier, user_connection_identifier_LIST);
//------------------------------------------------------------------------------
//взять информацию пользователя
#define get_user_infoRequest_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(int64_t, user_id, "user_id")
JSON_class_gen(get_user_infoRequest, get_user_infoRequest_LIST);
//------------------------------------------------------------------------------
//взять друзей пользователя (в ответе нет chat_id)
#define get_user_friendsRequest_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(int64_t, user_id, "user_id")\
		DEFINITION_NAME(int, offset, "offset")\
		DEFINITION_NAME(int, count, "count")
JSON_class_gen(get_user_friendsRequest, get_user_friendsRequest_LIST);
//------------------------------------------------------------------------------
//взять моих друзей (в ответе ко всему прочему прийдут chat_id чатов с друзьями)
#define get_my_friendsRequest_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(int, offset, "offset")\
		DEFINITION_NAME(int, count, "count")
JSON_class_gen(get_my_friendsRequest, get_my_friendsRequest_LIST);
//------------------------------------------------------------------------------
//взять мои чаты
#define get_my_chatsRequest_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(int, offset, "offset")\
		DEFINITION_NAME(int, count, "count")
JSON_class_gen(get_my_chatsRequest, get_my_chatsRequest_LIST);
//------------------------------------------------------------------------------
//отослать сообщение в чат
#define send_messageRequest_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(int64_t, chat_id, "chat_id")\
		DEFINITION_NAME(std::string, message, "message")
JSON_class_gen(send_messageRequest, send_messageRequest_LIST);
//------------------------------------------------------------------------------
//отослать запрос в друзья
#define send_request_to_friendRequest_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(int64_t, user_id, "user_id")\
		DEFINITION_NAME(std::string, message, "message")
JSON_class_gen(send_request_to_friendRequest, send_request_to_friendRequest_LIST);
//------------------------------------------------------------------------------
//задать статус
#define set_statusRequest_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(std::string, status, "status")
JSON_class_gen(set_statusRequest, set_statusRequest_LIST);
//------------------------------------------------------------------------------
//задать аватар
#define set_avatarRequest_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(std::string, status, "status")
JSON_class_gen(set_avatarRequest, set_avatarRequest_LIST);
//------------------------------------------------------------------------------


#pragma endregion