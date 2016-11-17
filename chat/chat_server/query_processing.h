#pragma once

#include <My\WinSocket.h>
#include <My\thread_pool.h>
#include "mysqlWrap.h"
#include "Log.h"
#include <iostream>
#include <map>
#include <list>

struct user {
	struct user_context {
		typedef std::shared_ptr< std::pair< std::string, std::string > > event_t;
		struct individual_user_context {
			user_context* context = nullptr;
			int64_t n_unread_chats = 0;
			int64_t n_requests = 0;
			time_t last_tick = 0;

			std::list< event_t > events;

			individual_user_context() : n_unread_chats(), n_requests(), last_tick(time(NULL)) {}
			individual_user_context(user_context* context);
			inline void refresh_last_tick() {
				time(&last_tick);
				if (context) context->last_tick = last_tick;
			}
		};

		unsigned short individual_number = 0;
		typedef std::string individual_user_key;
		std::map< individual_user_key, individual_user_context > individuals;

		std::string login;
		std::string user_name;
		std::string status;
		int64_t n_unread_chats = 0;
		int64_t n_requests = 0;
		time_t last_tick = 0;

		user_context(
			std::string login,
			std::string user_name,
			std::string status,
			int64_t n_unread_chats,
			int64_t n_requests
		) : 
			login(login),
			user_name(user_name),
			status(status),
			n_unread_chats(n_unread_chats),
			n_requests(n_requests)
		{}
	};
private:
	mutable std::mutex context_mutex;
	std::unique_ptr< user_context > context;
	bool init(mysqlWrap& connection, std::string& authResponse) throw (mysqlException);

public:
	static bool existUser(mysqlWrap& connection, const std::string& login) throw (mysqlException);
	static bool regUser(mysqlWrap& connection, const std::string& login, const std::string& password) throw (mysqlException);
	static bool authUser(mysqlWrap& connection, const std::string& login, const std::string& password, std::string& authResponse) throw (mysqlException);
	static bool send_message(mysqlWrap& connection, const int64_t& user_id, const int64_t& chat_id, const std::string& message) throw (mysqlException);
	static bool send_request_to_friend(mysqlWrap& connection, const int64_t& sender_user_id, const int64_t& receiver_user_id, const std::string& message) throw (mysqlException);
	static bool set_user_str_param(mysqlWrap& connection, const std::string& PARAM_NAME, const int64_t& user_id, const std::string& param) throw (mysqlException);
	static bool set_status(mysqlWrap& connection, const int64_t& user_id, const std::string& status) throw (mysqlException);
	static bool set_avatar(mysqlWrap& connection, const int64_t& user_id, const std::string& avatar) throw (mysqlException);
	
	bool user::get_user_info(mysqlWrap& connection, std::string& user_info_Response) const throw (mysqlException);

	user();
	~user();
	bool is_online(const user_context::individual_user_key& key);
	void clear_old_contexts() {}
};


extern const unsigned int max_user_count;
user* users();
void add_task(std::function< void(mysqlWrap& connection) > funk);

void test(std::string login, std::string password = "");

void query_processor(My::WinSocket sock, mysqlWrap& connection) throw (My::Exception);