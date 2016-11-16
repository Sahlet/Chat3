#pragma once

#include <My\WinSocket.h>
#include <My\thread_pool.h>
#include "mysqlWrap.h"
#include "Log.h"
#include <iostream>
#include <map>
#include <list>

class user {
	struct user_context {
		typedef std::list< std::shared_ptr< std::pair< std::string, std::string > > > events_t;
		struct individual_user_context {
			int64_t n_unread_chats = 0;
			int64_t n_requests = 0;

			events_t events;

			individual_user_context() : n_unread_chats(), n_requests() {}
			individual_user_context(const user_context& context);
		};

		unsigned short individual_number = 0;
		std::map< std::string, individual_user_context > individuals;

		std::string login;
		std::string user_name;
		std::string status;
		int64_t n_unread_chats = 0;
		int64_t n_requests = 0;

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

	std::mutex context_mutex;
	std::unique_ptr< user_context > context;
	bool init(mysqlWrap& connection, std::string& authResponse) throw (mysqlException);

public:
	static bool existUser(mysqlWrap& connection, const std::string& login) throw (mysqlException);
	static bool regUser(mysqlWrap& connection, const std::string& login, const std::string& password) throw (mysqlException);
	static bool authUser(mysqlWrap& connection, const std::string& login, const std::string& password, std::string& authResponse) throw (mysqlException);
	static bool send_message(mysqlWrap& connection, const int64_t& user_id, const int64_t& chat_id, const std::string& message) throw (mysqlException);
	static bool send_request_to_friend(mysqlWrap& connection, const int64_t& sender_user_id, const int64_t& receiver_user_id, const std::string& message) throw (mysqlException);

	user();
	~user();
	void clear_old_contexts() {}
};


extern const unsigned int max_user_count;
user* users();
void add_task(std::function< void(mysqlWrap& connection) > funk);

void test(std::string login, std::string password = "");

void query_processor(My::WinSocket sock, mysqlWrap& connection) throw (My::Exception);