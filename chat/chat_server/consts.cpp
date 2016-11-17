#include "consts.h"
#include <array>

std::array< const char*, size_t(command::get_users) + 1 > comands_in_bite_interpretation = {
	"unknown",
	"authorization",
	"registration",
	//---------------------
	"polling",
	//---------------------
	"set_status",
	"set_avatar",
	//---------------------
	"send_message",
	"send_request_for_friendship",
	"accept_request_for_friendship",
	"creat_chat",
	//---------------------
	// chat modifies
	"add_chat_member",
	"set_chat_member_access",
	//---------------------
	"get_user_data",
	"get_last_user_tick",
	//---------------------
	"get_chat_members",
	"get_requests_for_friendship",
	"get_messages",
	"get_friens",
	"get_chats",
	"get_users"
};

const char* comand_bite_interpretation(command cmd) {
	return comands_in_bite_interpretation[int(cmd)];
}

command comand_bite_interpretation(const char* cmd) {
	for (command res = command::unknown; int(res) < comands_in_bite_interpretation.size(); (*((int*)(&res)))++) {
		if (!strcmp(comands_in_bite_interpretation[int(res)], cmd)) return res;
	}
	return command::unknown;
}