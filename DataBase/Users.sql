DROP USER chat;

CREATE USER chat
	IDENTIFIED BY 'chat';

GRANT ALL ON my_chat.* TO 'chat';

/*
show grants for chat_server;

тут я хотел запретить пользоваетлю chat_server смотреть на пароли и удалять строки в my_chat.users,
но пишет, что не было назначено GRANT для (SELECT(pass) ON my_chat.users) и (DELETE ON my_chat.users) и пожтому отменять нечего...
REVOKE SELECT(pass)
	ON my_chat.users
	FROM 'chat_server';


REVOKE DELETE
	ON my_chat.users
	FROM chat_server;
*/