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
#define min_user_infoJSON_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(int64_t, user_id, "user_id")\
		DEFINITION_NAME(int64_t, unix_time_last_tick, "unix_time_last_tick")\
		DEFINITION_NAME(std::string, user_login, "user_login")\
		DEFINITION_NAME(std::string, user_name, "user_name")
JSON_class_gen(min_user_infoJSON, min_user_infoJSON_LIST);
JSONList_gen(min_user_infoJSON, "users");
//------------------------------------------------------------------------------
#define user_infoJSON_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(min_user_infoJSON, info, "info")\
		DEFINITION_NAME(std::string, status, "user_status")\
		DEFINITION_NAME(std::string, avatar, "avatar")
JSON_class_gen(user_infoJSON, user_infoJSON_LIST);
//------------------------------------------------------------------------------
#define user_last_tickJSON_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(int64_t, user_id, "user_id")\
		DEFINITION_NAME(int64_t, unix_time_last_tick, "unix_time_last_tick")
JSON_class_gen(user_last_tickJSON, user_last_tickJSON_LIST);
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
		DEFINITION_NAME(std::string, chat_avatar, "chat_avatar")\
		DEFINITION_NAME(CHAT_ACCESS, my_chat_access, "my_chat_access")\
		DEFINITION_NAME(int64_t, n_unread_messages, "n_unread_messages")
JSON_class_gen(chatJSON, chatJSON_LIST);
JSONList_gen(chatJSON, "chats");
//------------------------------------------------------------------------------
typedef min_user_infoJSON chat_memberJSON;
//------------------------------------------------------------------------------
#define chat_member_for_adminJSON_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(chat_memberJSON, chat_member, "chat_member")\
		DEFINITION_NAME(CHAT_ACCESS, chat_access, "chat_access")
JSON_class_gen(chat_member_for_adminJSON, chat_member_for_adminJSON_LIST);
JSONList_gen(chat_member_for_adminJSON, "chat_members");
//------------------------------------------------------------------------------
#define messageJSON_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(int64_t, message_id, "message_id")\
		DEFINITION_NAME(bool, unread, "unread")\
		DEFINITION_NAME(int64_t, user_id, "user_id")\
		DEFINITION_NAME(std::string, message, "message")\
		DEFINITION_NAME(int64_t, unix_time_last_tick, "unix_time_last_tick")
JSON_class_gen(messageJSON, messageJSON_LIST);
JSONList_gen(messageJSON, "messages");
//------------------------------------------------------------------------------
#define friendJSON_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(min_user_infoJSON, info, "info")\
		DEFINITION_NAME(int64_t, chat_id, "chat_id")
JSON_class_gen(friendJSON, friendJSON_LIST);
JSONList_gen(friendJSON, "friends");
//------------------------------------------------------------------------------
#define request_to_friendJSON_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(min_user_infoJSON, info, "info")\
		DEFINITION_NAME(std::string, request_message, "request_message")
JSON_class_gen(request_to_friendJSON, request_to_friendJSON_LIST);
JSONList_gen(request_to_friendJSON, "requests_to_friend");
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
//����� ���������� ������������
#define get_user_infoRequest_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(int64_t, user_id, "user_id")
JSON_class_gen(get_user_infoRequest, get_user_infoRequest_LIST);
//------------------------------------------------------------------------------
//����� last_tick ������������
#define get_user_last_tickRequest_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(int64_t, user_id, "user_id")
JSON_class_gen(get_user_last_tickRequest, get_user_last_tickRequest_LIST);
//------------------------------------------------------------------------------
//����� ������ ����
#define get_chat_recordsRequest_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(int64_t, chat_id, "chat_id")\
		DEFINITION_NAME(int, offset, "offset")\
		DEFINITION_NAME(int, count, "count")
JSON_class_gen(get_chat_recordsRequest, get_chat_recordsRequest_LIST);
//------------------------------------------------------------------------------
//����� ������ ������������ (� ������ ��� chat_id)
#define get_user_friendsRequest_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(int64_t, user_id, "user_id")\
		DEFINITION_NAME(int, offset, "offset")\
		DEFINITION_NAME(int, count, "count")
JSON_class_gen(get_user_friendsRequest, get_user_friendsRequest_LIST);
//------------------------------------------------------------------------------
//����� ������ ������������ (� ������ ��� chat_id)
#define find_user_friendsRequest_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(int64_t, user_id, "user_id")\
		DEFINITION_NAME(std::string, regular_str, "regular_str")\
		DEFINITION_NAME(int, offset, "offset")\
		DEFINITION_NAME(int, count, "count")
JSON_class_gen(find_user_friendsRequest, find_user_friendsRequest_LIST);
//------------------------------------------------------------------------------
//����� ������ �����-�� ���� �������
#define get_my_recordsRequest_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(int, offset, "offset")\
		DEFINITION_NAME(int, count, "count")
JSON_class_gen(get_my_recordsRequest, get_my_recordsRequest_LIST);
//------------------------------------------------------------------------------
//����� ������ �����-�� ���� �������
#define find_my_recordsRequest_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(std::string, regular_str, "regular_str")\
		DEFINITION_NAME(int, offset, "offset")\
		DEFINITION_NAME(int, count, "count")
JSON_class_gen(find_my_recordsRequest, find_my_recordsRequest_LIST);
//------------------------------------------------------------------------------
//�������� ��������� � ���
#define send_messageRequest_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(int64_t, chat_id, "chat_id")\
		DEFINITION_NAME(std::string, message, "message")
JSON_class_gen(send_messageRequest, send_messageRequest_LIST);
//------------------------------------------------------------------------------
//�������� ������ � ������
#define send_request_to_friendRequest_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(int64_t, user_id, "user_id")\
		DEFINITION_NAME(std::string, message, "message")
JSON_class_gen(send_request_to_friendRequest, send_request_to_friendRequest_LIST);
//------------------------------------------------------------------------------
//������� ������ � ������
#define accept_request_for_friendRequest_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(int64_t, user_id, "user_id")
JSON_class_gen(accept_request_for_friendRequest, accept_request_for_friendRequest_LIST);
//------------------------------------------------------------------------------
//�������� ������ � ������
#define create_chatRequest_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(std::string, chat_name, "chat_name")\
		DEFINITION_NAME(std::string, chat_avatar, "chat_avatar")
JSON_class_gen(create_chateRequest, create_chatRequest_LIST);
//------------------------------------------------------------------------------
//�������� ������������ � ���
#define add_chat_memberRequest_LIST(DEFINITION_NAME)\
		DEFINITION_NAME(int64_t, chat_id, "chat_id")\
		DEFINITION_NAME(int64_t, user_id, "user_id")\
		DEFINITION_NAME(CHAT_ACCESS, chat_access, "chat_access")
JSON_class_gen(add_chat_memberRequest, add_chat_memberRequest_LIST);
//------------------------------------------------------------------------------
//������ ������ ����� ���
typedef add_chat_memberRequest set_chat_member_accessRequest;
//------------------------------------------------------------------------------

#pragma endregion