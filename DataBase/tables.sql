USE my_chat;
-- comment
#comment
/*comment*/

-- --------------------------------------------------------------------------------------
DROP TABLE IF EXISTS my_chat.users;
CREATE TABLE IF NOT EXISTS my_chat.users (
	id SERIAL PRIMARY KEY,				-- SERIAL = BIGINT UNSIGNED NOT NULL AUTO_INCREMENT UNIQUE
	log VARCHAR(40) NOT NULL UNIQUE,	-- UNIQUE - не повторяестя значение этого поля в каждой строке
	pass BLOB NOT NULL,
    n_unread_chats BIGINT UNSIGNED NOT NULL DEFAULT 0,
	n_requests BIGINT UNSIGNED NOT NULL DEFAULT 0,
    -- public info
    name VARCHAR(40) DEFAULT NULL,
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
    n_unread_messages BIGINT UNSIGNED NOT NULL DEFAULT 0,
    UNIQUE INDEX(chat_id, user_id), -- для быстрого поиска членов чата
    UNIQUE INDEX(user_id, chat_id), -- для быстрого поиска чатов пользователя
    FOREIGN KEY(chat_id) REFERENCES my_chat.chats (id),
    FOREIGN KEY(user_id) REFERENCES my_chat.users (id)
)ENGINE InnoDB CHARACTER SET utf8;

DELIMITER //
CREATE TRIGGER chat_members_on_update AFTER UPDATE ON my_chat.chat_members FOR EACH ROW
BEGIN
/*
	IF (NEW.chat_id != OLD.chat_id) THEN
		SET NEW.chat_id = OLD.chat_id;
	END IF;
    IF (NEW.user_id != OLD.user_id) THEN
		SET NEW.user_id = OLD.user_id;
	END IF;
*/
    
	IF (OLD.n_unread_messages != NEW.n_unread_messages) THEN
		IF ((OLD.n_unread_messages < NEW.n_unread_messages) AND (OLD.n_unread_messages = 0)) THEN
			UPDATE my_chat.users SET my_chat.users.n_unread_chats = my_chat.users.n_unread_chats + 1
				WHERE my_chat.users.id = NEW.user_id;
		ELSE
			IF ((OLD.n_unread_messages > NEW.n_unread_messages) AND (NEW.n_unread_messages = 0)) THEN
				UPDATE my_chat.users SET my_chat.users.n_unread_chats = my_chat.users.n_unread_chats - 1
					WHERE my_chat.users.id = NEW.user_id;
			END IF;
		END IF;
    END IF;
END//
DELIMITER ;
-- --------------------------------------------------------------------------------------

-- --------------------------------------------------------------------------------------
-- Пусть имеем некоторый чат chat_id и в нем n пользователей с правом на чтение сообщений,
-- на каждое новое сообщение в этом чате будет появляться n записей.
-- так сделано, чтоб понимать, кто какое сообщение прочитал и чтоб пользователь,
-- которые не имели права на чтение в тот момент, когда пришло сообщение, не могли его прочесть.
DROP TABLE IF EXISTS my_chat.chat_messages;
CREATE TABLE IF NOT EXISTS my_chat.chat_messages(
	chat_id BIGINT UNSIGNED NOT NULL,
    user_id BIGINT UNSIGNED NOT NULL,
    message_id BIGINT UNSIGNED NOT NULL,
    unread BOOL NOT NULL DEFAULT TRUE,
	-- /*UNIQUE*/ INDEX(chat_id, user_id/*, message_id*/),
    PRIMARY KEY (chat_id, user_id, message_id),
    FOREIGN KEY(chat_id) REFERENCES my_chat.chats (id),
    FOREIGN KEY(user_id) REFERENCES my_chat.users (id),
    FOREIGN KEY(message_id) REFERENCES my_chat.messages (id)
)ENGINE InnoDB CHARACTER SET utf8;

DELIMITER //
CREATE TRIGGER chat_messages_on_insert AFTER INSERT ON my_chat.chat_messages FOR EACH ROW
BEGIN
	UPDATE my_chat.chat_members SET my_chat.chat_members.n_unread_messages = my_chat.chat_members.n_unread_messages + 1
		WHERE my_chat.chat_members.chat_id = NEW.chat_id AND my_chat.chat_members.user_id = NEW.user_id;
END//

CREATE TRIGGER chat_messages_on_update AFTER UPDATE ON my_chat.chat_messages FOR EACH ROW
BEGIN
/*
	IF (NEW.chat_id != OLD.chat_id) THEN
		SET NEW.chat_id = OLD.chat_id;
	END IF;
    IF (NEW.user_id != OLD.user_id) THEN
		SET NEW.user_id = OLD.user_id;
	END IF;
    IF (NEW.message_id != OLD.message_id) THEN
		SET NEW.message_id = OLD.message_id;
	END IF;
*/
    
	IF (OLD.unread AND NOT NEW.unread) THEN
		UPDATE my_chat.chat_members SET my_chat.chat_members.n_unread_messages = my_chat.chat_members.n_unread_messages - 1
			WHERE my_chat.chat_members.chat_id = NEW.chat_id AND my_chat.chat_members.user_id = NEW.user_id;
    END IF;
END//
DELIMITER ;
-- --------------------------------------------------------------------------------------

-- --------------------------------------------------------------------------------------
-- друзья, которые есть сейчас. (из таблицы могут удаляться записи)
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
    message TEXT, -- сообщение для добавления в друзья. Пример: "Здравствуте. Мы с вами вчера познакомились. Я Петя..."
    UNIQUE INDEX(user_id, requester_id),
    UNIQUE INDEX(requester_id, user_id),
    FOREIGN KEY(user_id) REFERENCES my_chat.users (id),
    FOREIGN KEY(requester_id) REFERENCES my_chat.users (id)
)ENGINE InnoDB CHARACTER SET utf8;

DELIMITER //
CREATE TRIGGER requests_on_insert AFTER INSERT ON my_chat.requests FOR EACH ROW
BEGIN
	UPDATE my_chat.users SET my_chat.users.n_requests = my_chat.users.n_requests + 1
		WHERE my_chat.users.id = NEW.user_id;
END//

CREATE TRIGGER requests_on_delete AFTER DELETE ON my_chat.requests FOR EACH ROW
BEGIN
	UPDATE my_chat.users SET my_chat.users.n_requests = my_chat.users.n_requests - 1
		WHERE my_chat.users.id = OLD.user_id;
END//
DELIMITER ;
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