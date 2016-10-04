/*

How to store binary data to MySQL
http://stackoverflow.com/questions/1071120/how-do-i-use-mysql-c-connector-for-storing-binary-data
http://stackoverflow.com/questions/1108700/how-to-set-binary-data-using-setblob-in-c-connector
http://stackoverflow.com/questions/21234999/read-binary-data-in-c-and-write-to-mysql

*/

#include "usingClasses.h"
#pragma comment(lib, "libmysql.lib") // подключаем библиотеку
#include <mysql.h>
#include<iostream>

#pragma region TheProgramm

//возможные команды
enum command {authorization, registration};
enum key {check, sender_private_message, sender_chat_message, sender_request, accepter_request, creater_chat, adder_part, line, seter_access, get_user_data, get_last_user_tick, geter_n_m_partners_from, geter_n_m_requests, geter_n_m_messages_p, geter_n_m_messages_c, geter_n_m_friens, geter_n_m_confs, geter_n_m_users, geter_freand_unread_last_tick, geter_conf_unread_last_tick, get_friend_id};
const char* buf;
string sbuf;

const unsigned short
	//количество команд, количество ответов, количество ключей команды авторизации.
	n_commands = 2, n_answers = 2, n_aut_keys = 21,
	//максимальное количество байт для логина, максимальное количество байт для пароля.
	max_login_len = 40, max_password_len = 40;


const charPtr
	commands[n_commands] = {"aut", "reg"},//массив команд
	answers[n_answers] = {"0", "1"},//массив ответов //0 - OK, 1 - ERROR
	aut_keys[n_aut_keys ] = {"check", "smp", "smc", "sr", "ar", "cc", "ap", "line", "sa", "gud", "glut", "gnmp", "gnmr", "gnmmp", "gnmc", "gnmf", "gnmco", "gnmu", "gful", "gcul", "gfid"},//массив ключей команды авторизации
	authorized("authorized."),
	registered("registered."),
	error("ERROR: "),
	warning("WARNING: "),
	key("key \' "),
	user("user \' "),
	authorization_error("incorrect autorisation parametres."),
	authorization_key_error(" \' is unknown authorization key."),
	registration_error(" \' is already exist."),
	wasnt_authorizated("You are not logged in."),
	unknown_command(" is unknown command."),
	null_symbol("\0", 1),
	login_len_error((string("\nYoure login is to long. max length of login is ") + to_string(max_login_len) + ".").c_str()),
	password_len_error((string("\nYoure password is to long. max length of password is ") + to_string(max_password_len) + ".").c_str());

 /**
	общение с клиентом:
		-клиент подключается к серверу и выполняет либо регистрацию, либо авторизацию.
		-при авторизации клиент указывае ключ, который определяет, за какое действие будет отвечать это соединение.
	смысл заключается в том, что клиент должен авторизроваться для того, чтоб получить соеденение, которое будет отвечать за конкретное действие, а действий может быть много и для полноценной работы клиента необходимо иметь хотя бы по одному соединению для каждого действия (так проще писать программу)
	клиент знает количество команд, значение кажной команды (смысл), и порядок, в котором они должны придти.
		-каждый запрос - команда и некоторые параметры.(параметры присылаются отдельно от запроса - не склеянные в одно сообщение, а разделенные байтами, которые отвечают за длину)
 **/

 /**
	смысл команд:

		-authorization login password key: авторизация
	(length of login <= max_login_len и length of password <= max_password_len)
	при удачной авторизации клиенту вернется сообщение соответствующего содержания; сервер переводит соеденение с клиентом в состояние соответствующие ключу key.

		-registration login password: регистрация
	(length of login <= max_login_len и length of password <= max_password_len)
	при удачной регистрации клиенту вернется сообщение соответствующего содержания.
 **/

#pragma region UserBaseManager

typedef MYSQL UBD;//UserBaseDescriptor

UBD *MD;//MainDescriptor
string Mhost, Muser, Mpassword;
unsigned int Mport;

//возвращает объект типа charPtr с удвоенными апострофами.
//если непоместится - вернет пустой объект типа charPtr.
charPtr mono_to_di_apostrophe(charPtr ptr){
	unsigned short n = ptr.getLen();
	unsigned int new_n = 0;
	char *source_ptr = ptr.getPtr(), *new_ptr;
	for (unsigned short i = 0; i < n; i++){ if(source_ptr[i] == '\'') new_n++; }
	new_n += n;
	if(new_n > 64000) return charPtr();
	new_ptr = new char[new_n];
	for (unsigned short i = 0, j = 0; i < n; i++, j++){
		new_ptr[j] = source_ptr[i];
		if(source_ptr[i] == '\'') new_ptr[++j] = '\'';
	}
	return charPtr(new_ptr, new_n);
}
//возвращает объект типа charPtr в котором удвоенные апострофы стали одиназными.
//если есть одинарный апостроф - вернет пустой объект типа charPtr.
charPtr di_to_mono_apostrophe(charPtr ptr){
	unsigned short n = ptr.getLen();
	unsigned int new_n = 0;
	char *source_ptr = ptr.getPtr(), *new_ptr;
	for (unsigned short i = 0; i < n; i++){
		if(source_ptr[i] == '\''){
			if(((i + 1) != n) && (source_ptr[i + 1] == '\'')){
				new_n++;
				i++;
			} else return charPtr();
		}
	}
	new_n = n - new_n / 2;
	new_ptr = new char[new_n];
	for (unsigned short i = 0, j = 0; i < n; i++, j++){
		if(source_ptr[i] == '\'') {
			new_ptr[j] = '\''; 
			i++;
		}
		else new_ptr[j] = source_ptr[i];
	}
	return charPtr(new_ptr, new_n);
}
charPtr insert_first_bifor_second(charPtr ptr, char first, char second){
	unsigned short n = ptr.getLen();
	unsigned int new_n = 0;
	char *source_ptr = ptr.getPtr(), *new_ptr;
	for (unsigned short i = 0; i < n; i++){ if(source_ptr[i] == second) new_n++; }
	new_n += n;
	if(new_n > 64000) return charPtr();
	new_ptr = new char[new_n];
	for (unsigned short i = 0, j = 0; i < n; i++, j++){
		if(source_ptr[i] == second) new_ptr[j++] = first;
		new_ptr[j] = source_ptr[i];
	}
	return charPtr(new_ptr, new_n);
}

// вернет ID пользователя, если такой есть, и charPtr с nullptr указателем на строку, если такого нет
charPtr authorize(charPtr login, charPtr passwodr) throw (MyException){
	charPtr res;
	UBD *desc = mysql_init(NULL);
	mysql_real_connect(desc, Mhost.c_str(), Muser.c_str(), Mpassword.c_str(), NULL, Mport, NULL, CLIENT_MULTI_STATEMENTS);
	if(mysql_query(desc, ("CALL my_chat.AUT('" + mono_to_di_apostrophe(login) + "','" + mono_to_di_apostrophe(passwodr) + "')" + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
	MYSQL_RES *query_res = mysql_store_result(desc);
    auto num_rows = mysql_num_rows(query_res); // количество строк.
	if (num_rows){
		const char * const buf = mysql_fetch_row(query_res)[0]; //const char * const потому, что тогда в res будет копироваться строка и после вызова mysql_free_result(query_res) деструктор объекта res выполнется нормально
		res.init(buf, strlen(buf));
	}
	mysql_free_result(query_res); // Очищаем результаты
	mysql_close(desc); // Закрываем соединение
	return res;
}
bool existUser(charPtr login) throw (MyException){
	bool res = false;
	UBD *desc = mysql_init(NULL);
	mysql_real_connect(desc, Mhost.c_str(), Muser.c_str(), Mpassword.c_str(), NULL, Mport, NULL, CLIENT_MULTI_STATEMENTS);
	if(mysql_query(desc, ("CALL my_chat.GET_USER_ID('" + mono_to_di_apostrophe(login) + "')" + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
	MYSQL_RES *query_res = mysql_store_result(desc);
	if (mysql_num_rows(query_res)) res = true;
	mysql_free_result(query_res);
	mysql_close(desc);
	return res;
}
void addUser(charPtr login, charPtr passwodr){
	bool res = false;
	UBD *desc = mysql_init(NULL);
	mysql_real_connect(desc, Mhost.c_str(), Muser.c_str(), Mpassword.c_str(), NULL, Mport, NULL, CLIENT_MULTI_STATEMENTS);
	if(mysql_query(desc, ("CALL my_chat.ADDUSER('" + mono_to_di_apostrophe(login) + "','" + mono_to_di_apostrophe(passwodr) + "')" + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
	mysql_close(desc);
}

//void dellUser(charPtr login){
//	
//}

////отвечает за соединение, для отправки приватных сообщений
//void sender_p_m(UBD *desc, MySocketPtr sock) throw (MyException, MySocketException){
//	charPtr friendID, message;
//	my_ulonglong n;
//	MYSQL_RES *query_res;
//	while(true){
//		//friendID - не id пользователя базы данных, а id записи в списке друзей
//		sock >> friendID >> message;
//		if(mysql_query(desc, ("CALL SEND_P_M(" + friendID + ",'" + mono_to_di_apostrophe(message) + "')" + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
//		query_res = mysql_store_result(desc);
//		n = mysql_num_rows(query_res);
//		mysql_free_result(query_res);
//		if (n) throw MyException(1, "could not send the message.");
//		else sock << answers[0];
//	}
//}
//void sender_c_m(UBD *desc, MySocketPtr sock) throw (MyException, MySocketException){
//	charPtr chatID, message;
//	my_ulonglong n;
//	MYSQL_RES *query_res;
//	while(true){
//		sock >> chatID >> message;
//		if(mysql_query(desc, ("CALL SEND_C_M(" + chatID + ",'" + mono_to_di_apostrophe(message) + "')" + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
//		query_res = mysql_store_result(desc);
//		n = mysql_num_rows(query_res);
//		mysql_free_result(query_res);
//		if (n) throw MyException(1, "could not send the message.");
//		else sock << answers[0];
//	}
//}
////если пользователю отправляется заявка не первый раз, то она обновится.
//void send_request(UBD *desc, MySocketPtr sock) throw (MyException, MySocketException){
//	charPtr userLOGIN, diuserLOGIN, message;
//	my_ulonglong n;
//	MYSQL_RES *query_res;
//	while(true){
//		sock >> userLOGIN >> message;
//		diuserLOGIN = mono_to_di_apostrophe(userLOGIN);
//		if(mysql_query(desc, ("CALL GET_USER_ID('" + diuserLOGIN + "')" + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
//		query_res = mysql_store_result(desc);
//		n = mysql_num_rows(query_res);
//		mysql_free_result(query_res);
//		if (n == 0){
//			sock << answers[1] << warning + "there is no user with login:\n" + userLOGIN;
//			continue;
//		}
//		if(mysql_query(desc, ("CALL SEND_REQUEST('" + diuserLOGIN  +"', '" + mono_to_di_apostrophe(message) + "')" + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
//		query_res = mysql_store_result(desc);
//		n = mysql_num_rows(query_res);
//		mysql_free_result(query_res);
//		if (n){
//			sock << answers[1] << warning + "user \" " + userLOGIN + " \" is your friend already.";
//			continue;
//		}
//		else sock << answers[0];
//	}
//}
//void accept_request(UBD *desc, MySocketPtr sock) throw (MyException, MySocketException){
//	charPtr login;
//	my_ulonglong n;
//	MYSQL_RES *query_res;
//	while(true){
//		sock >> login;
//		if(mysql_query(desc, ("CALL ACCEPT_REQUEST(" + mono_to_di_apostrophe(login) + ")" + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
//		query_res = mysql_store_result(desc);
//		n = mysql_num_rows(query_res);
//		mysql_free_result(query_res);
//		if (n) throw MyException(1, "could not accept the request.");
//		else sock << answers[0];
//	}
//}
//void creat_chat(UBD *desc, MySocketPtr sock) throw (MyException, MySocketException){
//	charPtr name_of_chat, DEFAULT;
//	my_ulonglong n;
//	MYSQL_RES *query_res;
//	while(true){
//		sock >> name_of_chat >> DEFAULT;
//		if (DEFAULT.getLen() != 1 || (DEFAULT.getPtr()[0] != 'u' && DEFAULT.getPtr()[0] != 'g')) throw MyException(1, "could not accept the request.");
//		if (name_of_chat.getLen() > 64){
//			sock << answers[1] << warning + "maximal length of name for chat is 64.";
//			continue;
//		}
//		if(mysql_query(desc, ("CALL CREATE_CHAT('" + mono_to_di_apostrophe(name_of_chat) + "', '" + DEFAULT + "')" + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
//		query_res = mysql_store_result(desc);
//		n = mysql_num_rows(query_res);
//		mysql_free_result(query_res);
//		if (n) throw MyException(1, "could not accept the request.");
//		else sock << answers[0];
//	}
//}
//void add_part(UBD *desc, MySocketPtr sock) throw (MyException, MySocketException){
//	charPtr friendID, chatID;
//	my_ulonglong n;
//	MYSQL_RES *query_res;
//	while(true){
//		sock >> friendID >> chatID;
//		if(mysql_query(desc, ("CALL IS_FRIEND_IN_CHAT(" + friendID + ", " + chatID + ")" + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
//		query_res = mysql_store_result(desc);
//		n = mysql_num_rows(query_res);
//		mysql_free_result(query_res);
//		if (n){
//			sock << answers[1] << warning + "your friend have been in chat allready.";
//			continue;
//		}
//		//выполнит ся при условии, что friendID - в друзьях и отправитель в chatID как минимум granted и что friendID не в chatID
//		if(mysql_query(desc, ("CALL ADD_PART(" + friendID + ", " + chatID + ")" + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
//		query_res = mysql_store_result(desc);
//		n = mysql_num_rows(query_res);
//		mysql_free_result(query_res);
//		if (n) throw MyException(1, "could not accept the request.");
//		else sock << answers[0];
//	}
//}
//void set_access(UBD *desc, MySocketPtr sock) throw (MyException, MySocketException){
//	charPtr login, chatID, acs;
//	my_ulonglong n;
//	MYSQL_RES *query_res;
//	while(true){
//		sock >> login >> chatID >> acs;
//		if(mysql_query(desc, ("CALL SET_ACCESS('" + mono_to_di_apostrophe(login) + "', " + chatID + ", '" + acs + "')" + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
//		query_res = mysql_store_result(desc);
//		n = mysql_num_rows(query_res);
//		mysql_free_result(query_res);
//		if (n) throw MyException(1, "could not accept the request.");
//		else sock << answers[0];
//	}
//}
//void online(UBD *desc, MySocketPtr sock) throw (MyException, MySocketException){
//	charPtr tmp;
//	MYSQL_RES *query_res;
//	MYSQL_ROW row;
//	while(true){
//		sock >> tmp;
//		if(mysql_query(desc, "CALL ONLINE()") > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
//		query_res = mysql_store_result(desc);
//		row = mysql_fetch_row(query_res);
//		sock << answers[0] << row[0] << row[1] << row[2];
//		mysql_free_result(query_res);
//	}
//}
//void get_n_m_partners_from(UBD *desc, MySocketPtr sock) throw (MyException, MySocketException){
//	//если все ок - вернет логины участников чата и к каждому из них 1 - (незабанин) или 0 - (забанен)
//	charPtr n, m, chatID; //n - начиная с какого (счет нач с 0), m - сколько
//	charPtr like;//для поиска по имени (будет искать имя, что начинается на значение, что в like)
//	MYSQL_RES *query_res;
//	my_ulonglong n_rows;
//	MYSQL_ROW row;
//	while(true){
//		sock >> n >> m >> like >> chatID;
//		if(mysql_query(desc, ("CALL GNM_PART_F_CHAT(" + n + ", " + m + ", '" + mono_to_di_apostrophe(like) + "%', " + chatID + ")" + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
//		query_res = mysql_store_result(desc);
//		sock << answers[0] << to_string(n_rows = mysql_num_rows(query_res)).c_str();
//		for (int i = 0; i < n_rows; i++){
//			row = mysql_fetch_row(query_res);
//			sock << di_to_mono_apostrophe((const char*)row[0]);
//			if (charPtr((const char*)row[1]) == charPtr("NULL")) sock << "0";
//			else sock << "1";
//		}
//		mysql_free_result(query_res);
//	}
//}
//void get_n_m_requests(UBD *desc, MySocketPtr sock) throw (MyException, MySocketException){
//	//смотрит приглашения в друзь
//	//если все ок - вернет логины и сообщение и сообщение к каждому из логинов
//	charPtr n, m; //n - начиная с какого (счет нач с 0), m - сколько
//	charPtr like;//для поиска по имени (будет искать имя, что начинается на значение, что в like)
//	MYSQL_RES *query_res;
//	my_ulonglong n_rows;
//	MYSQL_ROW row;
//	while(true){
//		sock >> n >> m >> like;
//		if(mysql_query(desc, ("CALL GNM_REQUESTS(" + n + ", " + m + ", '" + mono_to_di_apostrophe(like) + "%')" + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
//		query_res = mysql_store_result(desc);
//		sock << answers[0] << to_string(n_rows = mysql_num_rows(query_res)).c_str();
//		for (int i = 0; i < n_rows; i++){
//			row = mysql_fetch_row(query_res);
//			sock << di_to_mono_apostrophe((const char*)row[0]) << di_to_mono_apostrophe((const char*)row[1]);
//		}
//		mysql_free_result(query_res);
//	}
//}
////p - false, c - true;
//void get_n_m_messages(UBD *desc, MySocketPtr sock, bool p_or_c) throw (MyException, MySocketException){
//	//читает сообщения из конференции
//	//если все ок - вернет количество сообщений, {id сообщения, сообщение, логин отправителя, время}
//	charPtr n, m, ID; //n - начиная с какого (счет нач с 0), m - сколько
//	MYSQL_RES *query_res;
//	my_ulonglong n_rows;
//	MYSQL_ROW row;
//	while(true){
//		sock >> n >> m >> ID;
//		if (p_or_c){
//			if(mysql_query(desc, ("CALL GNM_MESSAGES_С(" + n + ", " + m + ", " + ID + ")" + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
//		} else {
//			if(mysql_query(desc, ("CALL GNM_MESSAGES_P(" + n + ", " + m + ", " + ID + ")" + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
//		}
//		query_res = mysql_store_result(desc);
//		sock << answers[0] << to_string(n_rows = mysql_num_rows(query_res)).c_str();
//		for (int i = 0; i < n_rows; i++){
//			row = mysql_fetch_row(query_res);
//			sock << row[0] << di_to_mono_apostrophe((const char*)row[1]) << di_to_mono_apostrophe((const char*)row[2]) << row[3];
//		}
//		mysql_free_result(query_res);
//	}
//}
////friends - false, confs - true;
//void get_n_m_chats(UBD *desc, MySocketPtr sock, bool friends_or_confs) throw (MyException, MySocketException){
//	//возвращает список конференций или друзей
//	//если все ок - вернет количество сообщений, {id чата, имя чата}
//	charPtr n, m, ch; //n - начиная с какого (счет нач с 0), m - сколько
//	charPtr like;//для поиска по имени (будет искать имя, что начинается на значение, что в like)
//	MYSQL_RES *query_res;
//	my_ulonglong n_rows;
//	MYSQL_ROW row;
//	while(true){
//		sock >> n >> m >> like;
//		if (friends_or_confs) ch = charPtr("c");
//		else ch = charPtr("p");
//		if(mysql_query(desc, ("CALL GNM_CHATS(" + n + ", " + m + ", '" + mono_to_di_apostrophe(like) + "%', '" + ch + "')" + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
//		query_res = mysql_store_result(desc);
//		sock << answers[0] << to_string(n_rows = mysql_num_rows(query_res)).c_str();
//		for (int i = 0; i < n_rows; i++){
//			row = mysql_fetch_row(query_res);
//			sock << row[0] << di_to_mono_apostrophe((const char*)row[1]);
//		}
//		mysql_free_result(query_res);
//	}
//}
//void get_n_m_users(UBD *desc, MySocketPtr sock) throw (MyException, MySocketException){
//	//возвращает пользователей
//	//если все ок - вернет количество сообщений, {логин пользователя}
//	charPtr n, m; //n - начиная с какого (счет нач с 0), m - сколько
//	charPtr like;//для поиска по имени (будет искать имя, что начинается на значение, что в like)
//	MYSQL_RES *query_res;
//	my_ulonglong n_rows;
//	MYSQL_ROW row;
//	while(true){
//		sock >> n >> m >> like;
//		if(mysql_query(desc, ("CALL GNM_USERS(" + n + ", " + m + ", '" + mono_to_di_apostrophe(like) + "%')" + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
//		query_res = mysql_store_result(desc);
//		sock << answers[0] << to_string(n_rows = mysql_num_rows(query_res)).c_str();
//		for (int i = 0; i < n_rows; i++){
//			row = mysql_fetch_row(query_res);
//			sock << row[0];
//		}
//		mysql_free_result(query_res);
//	}
//}
//void get_u_data(UBD *desc, MySocketPtr sock) throw (MyException, MySocketException){
//	charPtr login;
//	MYSQL_RES *query_res;
//	my_ulonglong n_rows;
//	while(true){
//		sock >> login;
//		if(mysql_query(desc, ("CALL GET_USER_DATA('" + mono_to_di_apostrophe(login) + "')" + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
//		query_res = mysql_store_result(desc);
//		n_rows = mysql_num_rows(query_res);
//		if (n_rows) sock << answers[0] << mysql_fetch_row(query_res)[0];
//		mysql_free_result(query_res);
//		if (n_rows == 0) throw MyException(1, "could not get user data.");
//	}
//}
//void get_last_u_tick(UBD *desc, MySocketPtr sock) throw (MyException, MySocketException){
//	charPtr login;
//	MYSQL_RES *query_res;
//	my_ulonglong n_rows;
//	while(true){
//		sock >> login;
//		if(mysql_query(desc, ("CALL GET_LAST_USER_TICK('" + mono_to_di_apostrophe(login) + "')" + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
//		query_res = mysql_store_result(desc);
//		n_rows = mysql_num_rows(query_res);
//		if (n_rows) sock << answers[0] << mysql_fetch_row(query_res)[0];
//		mysql_free_result(query_res);
//		if (n_rows == 0) throw MyException(1, "could not get last user tick.");
//	}
//}
#pragma endregion

//вернет тру, если в ptr есть символ из tabu_chars.
bool tabu_chars(charPtr ptr, charPtr tabu_chars){
	unsigned short n1 = ptr.getLen(), n2 = tabu_chars.getLen();
	char *c1 = ptr.getPtr(), *c2 = tabu_chars.getPtr();
	for (int i = 0; i < n1; i++){
		for (int j = 0; j < n2; j++){
			if (c1[i] == c2[j]) return true;
		}
	}
	return false;
}

void init_connection_for_work(UBD *&desc, charPtr userID) throw (MyException) {
	try{ if(mysql_query(desc, "USE my_chat") > 0) throw MyException(mysql_errno(desc), mysql_error(desc));//два раза, потому, что первый раз может не зайти
	}catch(MyException ex){
		if (ex.getErrorCode() != 2013){
			mysql_close(desc);
			desc = mysql_init(NULL);
			mysql_real_connect(desc, Mhost.c_str(), Muser.c_str(), Mpassword.c_str(), NULL, Mport, NULL, CLIENT_MULTI_STATEMENTS);// без флага CLIENT_MULTI_STATEMENTS не получится запускать хранимые процедуры
		}
		if(mysql_query(desc, "USE my_chat") > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
	}
	if(mysql_query(desc, ("SET @userID := " + userID + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
}

//вернуть номер команды
//если такой команды нет - вернет количество команд
unsigned short get_cmd_num(charPtr ptr){
	for(unsigned short i = 0; i < n_commands; i++){
		if (ptr == commands[i]) return i;
	}
	return n_commands;
}
unsigned short get_aut_key_num(charPtr ptr){
	for(unsigned short i = 0; i < n_aut_keys; i++){
		if (ptr == aut_keys[i]) return i;
	}
	return n_aut_keys;
}
void for_client(MySocket *sock_){
	try{
		void *tmp1 = sock_;
		void *tmp2 = sock_;
		MySocketPtr sock(sock_);
		//bool authorizated = false;//для проверки, авторизирован ли пользыватель (на этом сокете хоть раз - однозначно проверке на то, что добавлин ли сокет в socketBase)
		//for(int i = 0; i < n_commands; i++) sock << commands[i];
		//for(int i = 0; i < n_answers; i++) sock << answers[i];
		charPtr ptr1, ptr2, ptr3, ptr4;//для логина, пароля, сообщений...
		unsigned short for_cmd;
		while(true){
			sock >> ptr1; //читаем команду
			for_cmd = get_cmd_num(ptr1);
			if(for_cmd == authorization || for_cmd == registration){
				#pragma region TheProgramm
				sock >> ptr1 >> ptr2;
				if (for_cmd == authorization) sock >> ptr3;//так надо
				//проверка длины
				if (ptr1.getLen() > max_login_len || ptr2.getLen() > max_password_len) {
					ptr3.init((char*)nullptr, 0);
					if (ptr1.getLen() > max_login_len) ptr3 = login_len_error;
					if (ptr2.getLen() > max_password_len) ptr3 = ptr3 + password_len_error;
					sock << answers[1] << error + ptr3;
					continue;
				}
				//проверка на запрещенные символы
				if(tabu_chars(ptr1, charPtr("\0", 1)) || tabu_chars(ptr2, charPtr("\0", 1))){
					sock << answers[1] << error + "It is forbidden to use symbols \\0'\" for login or password.";
					continue;
				}
				if (for_cmd == authorization){
					charPtr userID(authorize(ptr1, ptr2));
					if (userID) {
						unsigned short for_key = get_aut_key_num(ptr3);
						if (for_key == n_aut_keys){
							sock << answers[1] << error + key + ptr3 + authorization_key_error;
							continue;
						}
						sock << answers[0];
						if (for_key == check) break;
						UBD *desc = mysql_init(NULL);
						mysql_real_connect(desc, Mhost.c_str(), Muser.c_str(), Mpassword.c_str(), NULL, Mport, NULL, CLIENT_MULTI_STATEMENTS);// без флага CLIENT_MULTI_STATEMENTS не получится запускать хранимые процедуры
						#pragma region TheProgramm
						try{
							MYSQL_RES *query_res;
							my_ulonglong n_rows;
							MYSQL_ROW row;
							charPtr ch;
							char *fail_message = nullptr;
							while(true){
								sock >> ptr1;
								init_connection_for_work(desc, userID);
								//init_connection_for_work(desc, userID);
								if (for_key <= get_last_user_tick){
									#pragma region TheProgramm
									///////////////////////////////////////////////////////////////////////////////////////
									if (for_key == sender_private_message || for_key == sender_chat_message){
										sock >> ptr2;//id , message
										//ptr2 = insert_first_bifor_second(ptr2, '\\', '\\');
										if (for_key == sender_private_message) ch.init("P", 1);
										if (for_key == sender_chat_message) ch.init("C", 1);
										if(mysql_query(desc, ("CALL SEND_" + ch + "_M(" + ptr1 + ",'" + mono_to_di_apostrophe(ptr2) + "')" + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
										fail_message = "could not send the message.";
									} else if (for_key == sender_request){
										sock >> ptr2;//userLOGIN , message
										ptr3 = mono_to_di_apostrophe(ptr1);
										if(mysql_query(desc, ("SELECT id FROM my_chat.users WHERE log = '" + ptr3 + "'" + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
										query_res = mysql_store_result(desc);
										n_rows = mysql_num_rows(query_res);
										mysql_free_result(query_res);
										if (n_rows == 0){ sock << answers[1] << warning + "there is no user with login:\n" + ptr1; continue;}
										else {
											init_connection_for_work(desc, userID);
											if(mysql_query(desc, ("CALL SEND_REQUEST('" + ptr3  +"', '" + mono_to_di_apostrophe(ptr2) + "')" + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
											fail_message = "user is your friend already.";
										}
									} else if(for_key == accepter_request){
										//sock >> ptr1;//login
										if(mysql_query(desc, ("CALL ACCEPT_REQUEST('" + mono_to_di_apostrophe(ptr1) + "')" + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
										fail_message = "could not accept the request.";
									} else if(for_key == creater_chat){
										sock >> ptr2;//name_of_chat, DEFAULT
										if (ptr2.getLen() != 1 || (ptr2.getPtr()[0] != 'u' && ptr2.getPtr()[0] != 'g')) throw MyException(1, "could not accept the request.");
										if (ptr1.getLen() > 64){ sock << answers[1] << warning + "maximal length of name for chat is 64."; continue;}
										else {
											if(mysql_query(desc, ("CALL CREATE_CHAT('" + mono_to_di_apostrophe(ptr1) + "', '" + ptr2 + "')" + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
											fail_message = "could not accept the request.";
										}
									} else if(for_key == adder_part){
										sock >> ptr2;//friendID , chatID
										if(mysql_query(desc, ("CALL IS_FRIEND_IN_CHAT(" + ptr1 + ", " + ptr2 + ")" + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
										query_res = mysql_store_result(desc);
										n_rows = mysql_num_rows(query_res);
										mysql_free_result(query_res);
										if (n_rows){ sock << answers[1] << warning + "your friend have been in chat allready."; continue;}
										else {
											init_connection_for_work(desc, userID);
											//выполнит ся при условии, что friendID - в друзьях и отправитель в chatID как минимум granted и что friendID не в chatID
											if(mysql_query(desc, ("CALL ADD_PART(" + ptr1 + ", " + ptr2 + ")" + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
											fail_message = "could not accept the request.";
										}
									} else if(for_key == line){
										//sock >> tmp;
										if(mysql_query(desc, "SELECT unread_private_chats, unread_chats, request FROM my_chat.users WHERE id = @userID") > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
										query_res = mysql_store_result(desc);
										row = mysql_fetch_row(query_res);
										sock << answers[0] << row[0] << row[1] << row[2];
										mysql_free_result(query_res);
										if(mysql_query(desc, "UPDATE my_chat.users SET unread_private_chats := FALSE, unread_chats := FALSE, request := FALSE, last_tick := NOW() WHERE id = @userID") > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
										continue;
									} else if(for_key == seter_access){
										sock >> ptr2 >> ptr3;//login >> chatID >> acs;
										if(mysql_query(desc, ("CALL SET_ACCESS('" + mono_to_di_apostrophe(ptr1) + "', " + ptr2 + ", '" + ptr3 + "')" + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
										fail_message = "could not accept the request.";
									} else if(for_key == get_user_data || for_key == get_last_user_tick){
										//sock >> login;
										if (for_key == get_user_data){if(mysql_query(desc, ("SELECT status FROM my_chat.users WHERE log = '" + mono_to_di_apostrophe(ptr1) + "'" + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));}
										else if(mysql_query(desc, ("SELECT last_tick FROM my_chat.users WHERE log = '" + mono_to_di_apostrophe(ptr1) + "'" + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
										query_res = mysql_store_result(desc);
										n_rows = mysql_num_rows(query_res);
										if (n_rows) sock << answers[0] << mysql_fetch_row(query_res)[0];
										mysql_free_result(query_res);
										if (n_rows == 0){ 
											if (for_key == get_user_data) throw MyException(1, "could not get user data.");
											else throw MyException(1, "could not get last user tick.");
										}
										continue;
									}
									
									if (query_res = mysql_store_result(desc)){
										n_rows = mysql_num_rows(query_res);
										mysql_free_result(query_res);
										if (n_rows) throw MyException(1, fail_message);
										else sock << answers[0];
									}	else sock << answers[0];
									continue;
									///////////////////////////////////////////////////////////////////////////////////////
									#pragma endregion
								}
								#pragma region TheProgramm
								if(for_key == geter_freand_unread_last_tick || for_key == geter_conf_unread_last_tick){
									//вернет ({0; 1}, время последнего сообщения)
									//sock >> ID;
									if (for_key == geter_conf_unread_last_tick) ch = charPtr("c");
									else ch = charPtr("p");
									if(mysql_query(desc, ("CALL GET_UNREAD_LAST_TICK(" + ptr1 + ", '" + ch + "')" + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
									query_res = mysql_store_result(desc);
									n_rows = mysql_num_rows(query_res);
									if (n_rows){
										row = mysql_fetch_row(query_res);
										sock << answers[0] << row[0] << row[1];
									}
									else{
										mysql_free_result(query_res);
										throw MyException(1, "could not get unread and last_tick.");
									}
									mysql_free_result(query_res);
									continue;
								}	
								if(for_key == get_friend_id){
									//вернет ({0; 1}, id друга в списке друзей (если у вас нет такого друга, то вернет 0, а если такого пользователя нет - вернет 1 и сообщение об этом))
									//sock >> ID;
									if(mysql_query(desc, ("CALL GET_FRIEND_ID(" + ptr1 + ")" + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
									query_res = mysql_store_result(desc);
									n_rows = mysql_num_rows(query_res);
									row = mysql_fetch_row(query_res);
									if (n_rows) sock << answers[0] << row[0];
									else{
										mysql_free_result(query_res);
										throw MyException(1, ("User ' " + ptr2 + " ' is not exsists." + null_symbol).getPtr());
									}
									mysql_free_result(query_res);
									continue;
								}	
								sock >> ptr2 >> ptr3;
								ptr3 = insert_first_bifor_second(ptr3, '\\', '_');
								ptr3 = insert_first_bifor_second(ptr3, '\\', '%');
								//ptr3 = insert_first_bifor_second(ptr3, '\\', '\\');
								if(for_key == geter_n_m_partners_from){
									//если все ок - вернет логины участников чата и к каждому из них 1 - (незабанин) или 0 - (забанен)
									//charPtr n, m, chatID; //n - начиная с какого (счет нач с 0), m - сколько
									//charPtr like;//для поиска по имени (будет искать имя, что начинается на значение, что в like)
									sock >> ptr4;//sock >> n >> m >> like >> chatID;
									if(mysql_query(desc, ("CALL GNM_PART_F_CHAT(" + ptr1 + ", " + ptr2 + ", '" + mono_to_di_apostrophe(ptr3) + "%', " + ptr4 + ")" + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
									query_res = mysql_store_result(desc);
									sock << answers[0] << to_string(n_rows = mysql_num_rows(query_res)).c_str();
									for (int i = 0; i < n_rows; i++){
										row = mysql_fetch_row(query_res);
										sock << di_to_mono_apostrophe((const char*)row[0]);
										if (charPtr((const char*)row[1]) == charPtr("NULL")) sock << "0";
										else sock << "1";
									}
									mysql_free_result(query_res);
								}
								if(for_key == geter_n_m_requests){
									//смотрит приглашения в друзь
									//если все ок - вернет логины и сообщение к каждому из логинов
									//charPtr n, m; //n - начиная с какого (счет нач с 0), m - сколько
									//charPtr like;//для поиска по имени (будет искать имя, что начинается на значение, что в like)
									//>> n >> m >> like
									if(mysql_query(desc, ("SELECT U.log, M.message FROM my_chat.r" + userID + " AS M, my_chat.users AS U WHERE U.log LIKE '" + mono_to_di_apostrophe(ptr3) + "%' AND U.id = M.user_id ORDER BY M.last_tick DESC LIMIT " + ptr1 + ", " + ptr2 + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
									query_res = mysql_store_result(desc);
									sock << answers[0] << to_string(n_rows = mysql_num_rows(query_res)).c_str();
									for (int i = 0; i < n_rows; i++){
										row = mysql_fetch_row(query_res);
										sock << di_to_mono_apostrophe((const char*)row[0]) << di_to_mono_apostrophe((const char*)row[1]);
									}
									mysql_free_result(query_res);
								}
								if(for_key == geter_n_m_messages_p || for_key == geter_n_m_messages_c){
									//читает сообщения из конференции
									//если все ок - вернет количество сообщений, {id сообщения, сообщение, логин отправителя, время}
									//charPtr n, m, ID; //n - начиная с какого (счет нач с 0), m - сколько
									//sock >> n >> m >> ID;
									if (for_key == geter_n_m_messages_c){
										if(mysql_query(desc, ("CALL GNM_MESSAGES_С(" + ptr1 + ", " + ptr2 + ", " + ptr3 + ")" + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
									}
									else if(mysql_query(desc, ("CALL GNM_MESSAGES_P(" + ptr1 + ", " + ptr2 + ", " + ptr3 + ")" + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
									query_res = mysql_store_result(desc);
									sock << answers[0] << to_string(n_rows = mysql_num_rows(query_res)).c_str();
									for (int i = 0; i < n_rows; i++){
										row = mysql_fetch_row(query_res);
										sock << row[0] << di_to_mono_apostrophe((const char*)row[2]) << di_to_mono_apostrophe((const char*)row[1]) << row[3];
									}
									mysql_free_result(query_res);
								}
								if (for_key == geter_n_m_users){
									//возвращает список пользовтелей, которые не друзья и не делали заявки в друзья данному пользователю
									//если все ок - вернет количество сообщений, {имя пользователя}
									//charPtr n, m, ch; //n - начиная с какого (счет нач с 0), m - сколько
									//charPtr like;//для поиска по имени (будет искать имя, что начинается на значение, что в like)
									//sock >> n >> m >> like;
									if(mysql_query(desc, ("CALL GNM_USERS(" + ptr1 + ", " + ptr2 + ", '" + mono_to_di_apostrophe(ptr3) + "%')" + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
									query_res = mysql_store_result(desc);
									sock << answers[0] << to_string(n_rows = mysql_num_rows(query_res)).c_str();
									for (int i = 0; i < n_rows; i++) sock << di_to_mono_apostrophe((const char*)mysql_fetch_row(query_res)[0]);
									mysql_free_result(query_res);
								}
								if(for_key == geter_n_m_friens || for_key == geter_n_m_confs){
									//возвращает список конференций или друзей
									//если все ок - вернет количество сообщений, {id чата, имя чата}
									//charPtr n, m, ch; //n - начиная с какого (счет нач с 0), m - сколько
									//charPtr like;//для поиска по имени (будет искать имя, что начинается на значение, что в like)
									//sock >> n >> m >> like;
									if (for_key == geter_n_m_confs) ch = charPtr("c");
									else ch = charPtr("p");
									if(mysql_query(desc, ("CALL GNM_CHATS(" + ptr1 + ", " + ptr2 + ", '" + mono_to_di_apostrophe(ptr3) + "%', '" + ch + "')" + null_symbol).getPtr()) > 0) throw MyException(mysql_errno(desc), mysql_error(desc));
									query_res = mysql_store_result(desc);
									sock << answers[0] << to_string(n_rows = mysql_num_rows(query_res)).c_str();
									for (int i = 0; i < n_rows; i++){
										row = mysql_fetch_row(query_res);
										sock << row[0] << di_to_mono_apostrophe((const char*)row[1]);
									}
									mysql_free_result(query_res);
									//
									//query_res = mysql_store_result(desc);
									//mysql_free_result(query_res);
									//
								}
								#pragma endregion
							}
						} catch(MySocketException) {
							mysql_close(desc); // Закрываем соединение
						} catch(MyException ex) {
							sock << answers[1] << ex.toString().c_str();
							mysql_close(desc); // Закрываем соединение
						}
						#pragma endregion
						return;
					} else sock << answers[1] << error + authorization_error;
				} else {
					if (!existUser(ptr1)) {
						addUser(ptr1, ptr2);
						sock << answers[0];
					} else sock << answers[1] << error + (user + ptr1 + registration_error);
				}
				continue;
				#pragma endregion
			} else {
				sock << answers[1] << error + (ptr1 + unknown_command);
				continue;
			}
		}
	} catch(MyException e) {
		//if (e.getErrorCode() != WSAECONNRESET) throw;
		//cerr << (string)e << endl << '\a';
	}
}

int main(int argc, char *argv[]){
#pragma region Connecting to DATDBASE
	MD = mysql_init(NULL);
	if(MD == nullptr){
		cerr << error << "can not create MYSQL descriptor.\n";
		return 1;
	}
	string port_s;
	if (argc != 5){
		cout << "Enter host:\n"; cin >> Mhost;
		cout << "Enter port:\n"; cin >> port_s;
		cout << "Enter login:\n"; cin >> Muser;
		cout << "Enter password:\n"; cin >> Mpassword;
	} else { Mhost = argv[1]; port_s = argv[2]; Muser = argv[3]; Mpassword = argv[4]; }
	
	try{
		Mport = atoi(port_s.c_str());
		if (atoi(port_s.c_str()) < 0 || (Mport == 0 && port_s != "0")) throw 2;
	}
	catch(...){
		cerr << error << "bad port.\n";
		return 2;
	}
	if( !mysql_real_connect(MD, Mhost.c_str(), Muser.c_str(), Mpassword.c_str(), NULL, Mport, NULL, CLIENT_MULTI_STATEMENTS) ){
		cerr << error << mysql_error(MD) << endl;
		return 3;
	} else {
		if(mysql_query(MD, "USE my_chat") > 0){
			cerr << error << mysql_error(MD) << endl;
			return 4;
		}
		cout << "connected.\n";
	}
#pragma endregion
	try{
		MySocketPtr sock(new MyWinSocket(SOCK_STREAM));
		sock->bind((MySocketAddress*)new MyWinSocketAddress("", 30000));
		sock->listen(SOMAXCONN);
		while (true) CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)for_client, (LPVOID)sock->accept(), NULL, NULL);
	}
	catch(MyException ex){ cerr << ex.toString(); }
	catch(...){
		cerr << error << "server crashed.\n";
		return 10;
	}
	//mysql_close(MD);
	return 0;
}
#pragma endregion