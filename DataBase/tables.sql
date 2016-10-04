USE my_chat;
-- comment
#comment
/*comment*/

-- --------------------------------------------------------------------------------------
DROP TABLE IF EXISTS my_chat.users;
CREATE TABLE IF NOT EXISTS my_chat.users (
	id SERIAL PRIMARY KEY,				-- SERIAL = BIGINT UNSIGNED NOT NULL AUTO_INCREMENT UNIQUE
	log VARCHAR(40) NOT NULL UNIQUE, -- UNIQUE - не повторяестя значение этого поля в каждой строке
	pass BLOB NOT NULL,
    name VARCHAR(40),
    -- info
    avatar BLOB DEFAULT NULL,
	status VARCHAR(255) NOT NULL DEFAULT '', -- статус
	last_tick TIMESTAMP NOT NULL,	-- если не задать значение, то в переменной будет храниться время изменения (добавления) записи в диапазоне от «1970-01-01 00:00:00» до некоторой даты в 2038 г, ДЛЯ ОБНОВЛЕНИЯ ВРЕМЕНИ ИСПОЛЬЗУЙ ФУНКЦИЮ NOW() или CURRENT_TIMESTAMP //insert into dt1 values(now());
	UNIQUE INDEX(log(40)),
    INDEX(name(40))
)ENGINE InnoDB CHARACTER SET utf8;
-- --------------------------------------------------------------------------------------

-- --------------------------------------------------------------------------------------
DROP TABLE IF EXISTS my_chat.messages;
CREATE TABLE IF NOT EXISTS my_chat.messages(
	id SERIAL PRIMARY KEY,
    message TEXT NOT NULL,
    user_id BIGINT UNSIGNED NOT NULL,  -- если в этом поле будет NULL - это сообщение от сервера (например сообщение о том, что кто-то из пользователей добавил в чат еще одного пользователя)
	last_tick TIMESTAMP NOT NULL,
    FOREIGN KEY(user_id) REFERENCES my_chat.users (id)
)ENGINE InnoDB CHARACTER SET utf8;
-- --------------------------------------------------------------------------------------

-- --------------------------------------------------------------------------------------
DROP TABLE IF EXISTS my_chat.chats;
CREATE TABLE IF NOT EXISTS my_chat.chats(
	id SERIAL PRIMARY KEY,
    chat_name VARCHAR(255) DEFAULT NULL,
    avatar BLOB DEFAULT NULL
)ENGINE InnoDB CHARACTER SET utf8;
-- --------------------------------------------------------------------------------------

-- --------------------------------------------------------------------------------------
-- поле access принимает 4 значения ('a','p','u',NULL)
-- NULL		- доступ закрыт
-- 'u'		- 'user' обычный пользователь (может читать, писать)
-- 'p'		- 'privileged_user' привилегированный пользователь (может добавлять в чат других 'u' и 'p' пользователей, менять имя чата, менять аватар и имеет права предыдущего уровня доступа)
-- 'a'		- 'admin' админ, (может назначать все предыдущие уровни доступа, может удалить чат и имеет права предыдущего уровня доступа)
DROP TABLE IF EXISTS my_chat.chat_members;
CREATE TABLE IF NOT EXISTS my_chat.chat_members(
	chat_id BIGINT UNSIGNED NOT NULL,
    user_id BIGINT UNSIGNED NOT NULL,
	access ENUM('a','p','u') DEFAULT 'p',
    UNIQUE INDEX(chat_id, user_id), -- для быстрого поиска членов чата
    UNIQUE INDEX(user_id, chat_id), -- для быстрого поиска чатов пользователя
    FOREIGN KEY(chat_id) REFERENCES my_chat.chats (id),
    FOREIGN KEY(user_id) REFERENCES my_chat.users (id)
)ENGINE InnoDB CHARACTER SET utf8;
-- --------------------------------------------------------------------------------------

-- --------------------------------------------------------------------------------------
DROP TABLE IF EXISTS my_chat.chat_messages;
CREATE TABLE IF NOT EXISTS my_chat.chat_messages(
	chat_id BIGINT UNSIGNED NOT NULL,
    user_id BIGINT UNSIGNED NOT NULL,
    message_id BIGINT UNSIGNED NOT NULL,
	/*UNIQUE*/ INDEX(chat_id, user_id/*, message_id*/),
    FOREIGN KEY(chat_id) REFERENCES my_chat.chats (id),
    FOREIGN KEY(user_id) REFERENCES my_chat.users (id),
    FOREIGN KEY(message_id) REFERENCES my_chat.messages (id)
)ENGINE InnoDB CHARACTER SET utf8;
-- --------------------------------------------------------------------------------------

-- --------------------------------------------------------------------------------------
-- друзья, которые есть сейчас (из таблицы могут удаляться записи)
DROP TABLE IF EXISTS my_chat.friends;
CREATE TABLE IF NOT EXISTS my_chat.friends(
    user_id BIGINT UNSIGNED NOT NULL,
    friend_id BIGINT UNSIGNED NOT NULL,
    UNIQUE INDEX(user_id, friend_id),
    FOREIGN KEY(user_id) REFERENCES my_chat.users (id),
    FOREIGN KEY(friend_id) REFERENCES my_chat.users (id)
)ENGINE InnoDB CHARACTER SET utf8;
-- --------------------------------------------------------------------------------------

-- --------------------------------------------------------------------------------------
DROP TABLE IF EXISTS my_chat.requests;
CREATE TABLE IF NOT EXISTS my_chat.requests(
	user_id BIGINT UNSIGNED NOT NULL,
    requester_id BIGINT UNSIGNED NOT NULL,
    message TEXT, -- сообщение для добавления в друзья. Пример: "Здравствуте. Мы с вами всера познакомились..."
    UNIQUE INDEX(user_id, requester_id),
    UNIQUE INDEX(requester_id, user_id),
    FOREIGN KEY(user_id) REFERENCES my_chat.users (id),
    FOREIGN KEY(requester_id) REFERENCES my_chat.users (id)
)ENGINE InnoDB CHARACTER SET utf8;
-- --------------------------------------------------------------------------------------

-- --------------------------------------------------------------------------------------
-- чаты друзей - чаты всех друзей за все время (на случай, если удалить друга, чат все равно хранится в базе)
DROP TABLE IF EXISTS my_chat.friends_chats;
CREATE TABLE IF NOT EXISTS my_chat.friends_chats(
    user_id1 BIGINT UNSIGNED NOT NULL, -- smaller user_id
    user_id2 BIGINT UNSIGNED NOT NULL, -- bigger user_id
    chat_id BIGINT UNSIGNED UNIQUE NOT NULL,
    UNIQUE INDEX(user_id1, user_id2),
    FOREIGN KEY(user_id1) REFERENCES my_chat.users (id),
    FOREIGN KEY(user_id2) REFERENCES my_chat.users (id),
    FOREIGN KEY(chat_id) REFERENCES my_chat.chats (id)
)ENGINE InnoDB CHARACTER SET utf8;
-- --------------------------------------------------------------------------------------