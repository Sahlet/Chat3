#include "communication_data.h"

QUERY_RESPONSE_STATUS EnumSerializer< QUERY_RESPONSE_STATUS >::from_string(const std::string& str) {
	if (str == "OK") return QUERY_RESPONSE_STATUS::OK;

	return QUERY_RESPONSE_STATUS(0);
}
std::string EnumSerializer< QUERY_RESPONSE_STATUS >::to_string(const QUERY_RESPONSE_STATUS& enm) {
	if (enm == QUERY_RESPONSE_STATUS::OK) return "OK";
	return "ERROR";
}

CHAT_ACCESS EnumSerializer< CHAT_ACCESS >::from_string(const std::string& str) {
	if (str == "'u'") return CHAT_ACCESS::USER;
	else if (str == "'p'") return CHAT_ACCESS::PRIVILEGED_USER;
	else if (str == "'a'") return CHAT_ACCESS::ADMIN;

	return CHAT_ACCESS(0);
}
std::string EnumSerializer< CHAT_ACCESS >::to_string(const CHAT_ACCESS& enm) {
	if (enm == CHAT_ACCESS::USER) return "'u'";
	else if (enm == CHAT_ACCESS::PRIVILEGED_USER) return "'p'";
	else if (enm == CHAT_ACCESS::ADMIN) return "'a'";
	
	return "NULL";
}

command EnumSerializer< command >::from_string(const std::string& str) {
	return comand_bite_interpretation(str.c_str());
}
std::string EnumSerializer< command >::to_string(const command& enm) {
	return comand_bite_interpretation(enm);
}