USE my_chat;
-- comment
#comment
/*comment*/

-- --------------------------------------------------------------------------------------
DELIMITER //

DROP PROCEDURE IF EXISTS AUTHORIZATION//
CREATE PROCEDURE AUTHORIZATION(login VARCHAR(40), password BLOB)
BEGIN
	START TRANSACTION;
	SELECT id FROM my_chat.users WHERE log = login AND pass = password;
	COMMIT;
END//

DROP PROCEDURE IF EXISTS GET_MY_INFO//
CREATE PROCEDURE GET_MY_INFO(userID BIGINT UNSIGNED)
BEGIN
	START TRANSACTION;
	SELECT 
	log,
	name,
	status,
	n_unread_chats,
	n_requests
	FROM my_chat.users WHERE id = userID;
	COMMIT;
END//

DROP PROCEDURE IF EXISTS GET_USER_INFO//
CREATE PROCEDURE GET_USER_INFO(userID BIGINT UNSIGNED)
BEGIN
	START TRANSACTION;
	SELECT 
	log,
	name,
	status,
	UNIX_TIMESTAMP(last_tick)
	FROM my_chat.users WHERE id = userID;
	COMMIT;
END//

DROP PROCEDURE IF EXISTS GET_USER_LAST_TICK//
CREATE PROCEDURE GET_USER_LAST_TICK(userID BIGINT UNSIGNED)
BEGIN
	START TRANSACTION;
	SELECT
	UNIX_TIMESTAMP(last_tick)
	FROM my_chat.users WHERE id = userID;
	COMMIT;
END//

DROP PROCEDURE IF EXISTS GET_USER_AVATAR//
CREATE PROCEDURE GET_USER_AVATAR(userID BIGINT UNSIGNED)
BEGIN
	START TRANSACTION;
	SELECT 
	avatar
	FROM my_chat.users WHERE id = userID;
	COMMIT;
END//

DROP PROCEDURE IF EXISTS GET_USER_ID//
CREATE PROCEDURE GET_USER_ID(login VARCHAR(40))
BEGIN
	START TRANSACTION;
		SELECT id FROM my_chat.users WHERE log = login;
	COMMIT;
END//

DROP PROCEDURE IF EXISTS ADDUSER//
CREATE PROCEDURE ADDUSER(login VARCHAR(40), password BLOB, max_userID BIGINT UNSIGNED)
BEGIN
	START TRANSACTION;
	-- INSERT INTO my_chat.users (log, pass) values(login, password);
	
		SET @count := (SELECT id FROM my_chat.users WHERE log = login);

		IF (ISNULL(@count) OR @count = 0) THEN
			INSERT INTO my_chat.users (log, pass) values(login, password);
			SET @userID = (SELECT last_insert_id());
			IF (max_userID < @userID) THEN
				DELETE FROM my_chat.users WHERE my_chat.users.id = @userID;
			ELSE
				SELECT @userID;
			END IF;
			
		END IF;
	
	COMMIT;
END//

DROP PROCEDURE IF EXISTS SEND_CHAT_MESSAGE//
CREATE PROCEDURE SEND_CHAT_MESSAGE(userID BIGINT UNSIGNED, chatID BIGINT UNSIGNED, message_ TEXT)
BEGIN
	SET @var = NULL;
	START TRANSACTION;
		SET @var = (SELECT COUNT(*) FROM my_chat.chat_members WHERE my_chat.chat_members.chat_id = chatID AND my_chat.chat_members.user_id = userID AND NOT ISNULL(my_chat.chat_members.access));
	
		IF (!ISNULL(@var) AND @var != 0) THEN
			
			INSERT INTO my_chat.messages (message, user_id) values(message_, userID);
			SET @messageID = (SELECT last_insert_id());
			
			DROP TABLE IF EXISTS my_chat._____tmp_tab_for_insert_____;
			CREATE TEMPORARY TABLE my_chat._____tmp_tab_for_insert_____ ENGINE = MyISAM
			AS (
				SELECT my_chat.chat_members.chat_id AS chat_id, my_chat.chat_members.user_id AS user_id
				FROM my_chat.chat_members
				WHERE my_chat.chat_members.chat_id = chatID AND NOT ISNULL(my_chat.chat_members.access)
			);
			
			-- my_chat._____tmp_tab_for_insert_____ has to be because of there is oninsert triger for chat_messages, thet modifies my_chat.chat_members
			 
			INSERT INTO my_chat.chat_messages (chat_id, user_id, message_id)
				SELECT
					my_chat._____tmp_tab_for_insert_____.chat_id,
					my_chat._____tmp_tab_for_insert_____.user_id,
					@messageID
				FROM my_chat._____tmp_tab_for_insert_____;
			DROP TABLE IF EXISTS my_chat._____tmp_tab_for_insert_____;
						
			SELECT @messageID;
		END IF;
	COMMIT;
END//

DROP PROCEDURE IF EXISTS SEND_PRIVATE_MESSAGE//
CREATE PROCEDURE SEND_PRIVATE_MESSAGE(userID BIGINT UNSIGNED, freindID BIGINT UNSIGNED, message_ TEXT)
BEGIN
	SET @var = NULL;
	START TRANSACTION;
		SET @var = (SELECT COUNT(*) FROM my_chat.friends WHERE my_chat.friends.user_id = userID AND my_chat.friends.friend_id = friendID);
	COMMIT;
	IF (!ISNULL(@var) AND @var != 0) THEN
		SET @friends_chatsID = NULL;
		SET @minID = LEAST(userID, friendID);
		SET @maxID = GREATEST(userID, friendID);
		START TRANSACTION;
			SET @friends_chatID = (SELECT chat_id FROM my_chat.friends_chats WHERE my_chat.friends_chats.user_id1 = @minID AND my_chat.friends_chats.user_id2 = @maxID);
		COMMIT;
		CALL SEND_CHAT_MESSAGE(userID, @friends_chatID, message_);
	END IF;
END//

DROP PROCEDURE IF EXISTS CREATE_CHAT//
CREATE PROCEDURE CREATE_CHAT(userID BIGINT UNSIGNED, chat_name_ VARCHAR(255), chat_avatar_ BLOB)
BEGIN
	START TRANSACTION;
		INSERT INTO my_chat.chats (chat_name, avatar) values(chat_name_, chat_avatar_);
		SET @chatID = (SELECT last_insert_id());
		INSERT INTO my_chat.chat_members (chat_id, user_id, access) values(@chatID, userID, 'a');
		SELECT @chatID;
	COMMIT;
END//

DROP PROCEDURE IF EXISTS SEND_REQUEST//
CREATE PROCEDURE SEND_REQUEST(requesterID BIGINT UNSIGNED, userID BIGINT UNSIGNED, message_ TEXT)
BEGIN
	SET @var = NULL;
	START TRANSACTION;
		SET @var = (SELECT COUNT(*) FROM my_chat.friends WHERE my_chat.friends.user_id = requesterID AND my_chat.friends.friend_id = userID);
	
		IF (ISNULL(@var) OR @var = 0) THEN
			SET @var = (SELECT COUNT(*) FROM my_chat.requests WHERE (my_chat.requests.user_id = requesterID AND my_chat.requests.requester_id = userID) OR (my_chat.requests.user_id = userID AND my_chat.requests.requester_id = requesterID));
			
			IF (ISNULL(@var) OR @var = 0) THEN
				INSERT INTO my_chat.requests (user_id, requester_id, message) values(userID, requesterID, message_);
				SELECT last_insert_id();
			END IF;
		END IF;
	COMMIT;
END//

DROP PROCEDURE IF EXISTS ACCEPT_REQUEST//
CREATE PROCEDURE ACCEPT_REQUEST(userID BIGINT UNSIGNED, requesterID BIGINT UNSIGNED)
BEGIN
	SET @var = NULL;
	START TRANSACTION;
		SET @var = (SELECT COUNT(*) FROM my_chat.requests WHERE (my_chat.requests.user_id = userID AND my_chat.requests.requester_id = requesterID));
			
		IF (!ISNULL(@var) AND @var != 0) THEN
			DELETE FROM my_chat.requests WHERE (my_chat.requests.user_id = userID AND my_chat.requests.requester_id = requesterID);
			
			SET @minID = LEAST(userID, requesterID);
			SET @maxID = GREATEST(userID, requesterID);
			
			SET @chatID = (SELECT chat_id FROM my_chat.friends_chats WHERE user_id1 = @minID AND user_id2 = @maxID);
			
			IF (ISNULL(@chatID)) THEN
				INSERT INTO my_chat.chats () values()/*(chat_name, avatar) values(NULL, NULL)*/;
				SET @chatID = (SELECT last_insert_id());
				INSERT INTO my_chat.chat_members (chat_id, user_id, access) values(@chatID, userID, 'u');
				INSERT INTO my_chat.chat_members (chat_id, user_id, access) values(@chatID, requesterID, 'u');
			
				INSERT INTO my_chat.friends_chats (user_id1, user_id2, chat_id) values (@minID, @maxID, @chatID);
			END IF;
			
			INSERT INTO my_chat.friends (user_id, friend_id) values (userID, requesterID);
			INSERT INTO my_chat.friends (user_id, friend_id) values (requesterID, userID);
			
			SELECT @chatID;
		END IF;
	COMMIT;
END//

DROP PROCEDURE IF EXISTS ADD_MEMBER//
CREATE PROCEDURE ADD_MEMBER(adder_userID BIGINT UNSIGNED, chatID BIGINT UNSIGNED, memberID BIGINT UNSIGNED, access_ ENUM('p','u'))
BEGIN
	IF (!ISNULL(access_)) THEN
		START TRANSACTION;
			SET @var = (SELECT COUNT(*) FROM my_chat.chat_members 
						WHERE  (my_chat.chat_members.chat_id = chatID
								AND my_chat.chat_members.user_id = adder_userID
								AND (!ISNULL(my_chat.chat_members.access))
								AND my_chat.chat_members.access != 'u'
							   )
					   );
					
			IF (!ISNULL(@var) AND @var != 0) THEN
				SET @var = (SELECT COUNT(*) FROM my_chat.chat_members 
							WHERE (my_chat.chat_members.chat_id = chatID
							AND my_chat.chat_members.user_id = memberID)
							);
				IF (ISNULL(@var) OR @var = 0) THEN
					INSERT INTO my_chat.chat_members (chat_id, user_id, access) values(chatID, memberID, access_);
					SELECT 1;
				END IF;
			END IF;
		COMMIT;
	END IF;
END//

DROP PROCEDURE IF EXISTS SET_MEMBER_ACCESS//
CREATE PROCEDURE SET_MEMBER_ACCESS(setter_userID BIGINT UNSIGNED, chatID BIGINT UNSIGNED, memberID BIGINT UNSIGNED, access_ ENUM('p','u'))
BEGIN
	START TRANSACTION;
		SET @var = (SELECT COUNT(*) FROM my_chat.chat_members 
					WHERE  (my_chat.chat_members.chat_id = chatID
							AND my_chat.chat_members.user_id = setter_userID
							AND my_chat.chat_members.access = 'a'
						   )
					);
					
		IF (!ISNULL(@var) AND @var != 0) THEN
			SET @var = (SELECT COUNT(*) FROM my_chat.chat_members 
						WHERE (my_chat.chat_members.chat_id = chatID
						AND my_chat.chat_members.user_id = memberID)
						);
			IF (ISNULL(@var) OR @var = 0) THEN
				UPDATE my_chat.chat_members SET access := access_ WHERE my_chat.chat_members.chat_id = chatID AND my_chat.chat_members.user_id = memberID;
				SELECT 1;
			END IF;
		END IF;
	COMMIT;
END//

DROP PROCEDURE IF EXISTS USER_ONLINE//
CREATE PROCEDURE USER_ONLINE(userID BIGINT UNSIGNED, UNIX_TIMESTAMP_time BIGINT UNSIGNED)
BEGIN
	START TRANSACTION;
		UPDATE my_chat.users SET last_tick = FROM_UNIXTIME(UNIX_TIMESTAMP_time) WHERE id = userID;
	COMMIT;
END//

DROP PROCEDURE IF EXISTS SET_STATUS//
CREATE PROCEDURE SET_STATUS(userID BIGINT UNSIGNED, status_ TEXT)
BEGIN
	START TRANSACTION;
		UPDATE my_chat.users SET status = status_ WHERE id = userID;
	COMMIT;
END//

DROP PROCEDURE IF EXISTS SET_AVATAR//
CREATE PROCEDURE SET_AVATAR(userID BIGINT UNSIGNED, avatar_ BLOB)
BEGIN
	START TRANSACTION;
		UPDATE my_chat.users SET avatar = avatar_ WHERE id = userID;
	COMMIT;
END//

DROP PROCEDURE IF EXISTS FIND_USER//
CREATE PROCEDURE FIND_USER(regular_user_name VARCHAR(40), offset_ BIGINT UNSIGNED, number BIGINT UNSIGNED)
BEGIN
	START TRANSACTION;
		SELECT
			my_chat.users.id,
			UNIX_TIMESTAMP(my_chat.users.last_tick),
			my_chat.users.log,
			my_chat.users.name
		FROM my_chat.users
		WHERE (my_chat.users.name REGEXP regular_user_name) OR (my_chat.users.log REGEXP regular_user_name)
		ORDER BY my_chat.users.id ASC
		LIMIT offset_, number;
	COMMIT;
END//

DROP PROCEDURE IF EXISTS FIND_USER_WITH_ID_GREATER_THEN_pred_userID//
CREATE PROCEDURE FIND_USER_WITH_ID_GREATER_THEN_pred_userID(regular_user_name VARCHAR(40), pred_userID BIGINT UNSIGNED, number BIGINT UNSIGNED)
BEGIN
	START TRANSACTION;
		SELECT
			my_chat.users.id,
			UNIX_TIMESTAMP(my_chat.users.last_tick),
			my_chat.users.log,
			my_chat.users.name
		FROM my_chat.users
		WHERE my_chat.users.id > pred_userID AND (my_chat.users.name REGEXP regular_user_name) OR (my_chat.users.log REGEXP regular_user_name)
		ORDER BY my_chat.users.id ASC
		LIMIT number;
	COMMIT;
END//

DROP PROCEDURE IF EXISTS FIND_CHAT//
CREATE PROCEDURE FIND_CHAT(userID BIGINT UNSIGNED, regular_chat_name VARCHAR(255), offset_ BIGINT UNSIGNED, number BIGINT UNSIGNED)
BEGIN
	START TRANSACTION;
		SELECT
			my_chat.chats.id,
			my_chat.chats.chat_name,
			my_chat.chats.avatar,
			
			my_chat.chat_members.access,
			my_chat.chat_members.n_unread_messages
		FROM my_chat.chat_members, my_chat.chats
		WHERE my_chat.chat_members.user_id = userID AND my_chat.chat_members.chat_id = my_chat.chats.id
			AND (my_chat.chats.chat_name REGEXP regular_chat_name)
		LIMIT offset_, number;
	COMMIT;
END//

-- вернет чаты, в которых есть сообщения за некоторый промежуток времени
DROP PROCEDURE IF EXISTS FIND_CHAT_BY_TIME_RANGE//
CREATE PROCEDURE FIND_CHAT_BY_TIME_RANGE(userID BIGINT UNSIGNED, UNIX_TIMESTAMP_left_end BIGINT UNSIGNED, UNIX_TIMESTAMP_right_end BIGINT UNSIGNED, offset_ BIGINT UNSIGNED, number BIGINT UNSIGNED)
BEGIN
	SET @left = LEAST(UNIX_TIMESTAMP_left_end, UNIX_TIMESTAMP_right_end);
	SET @right = GREATEST(UNIX_TIMESTAMP_left_end, UNIX_TIMESTAMP_right_end);
	START TRANSACTION;
		SELECT
			my_chat.chats.id,
			my_chat.chats.chat_name,
			my_chat.chats.avatar,
			
			my_chat.chat_members.access,
			my_chat.chat_members.n_unread_messages
		FROM my_chat.chat_members, my_chat.chats
		WHERE my_chat.chat_members.user_id = userID
			AND (
				SELECT COUNT(*) FROM my_chat.chat_messages
				WHERE
					my_chat.chat_messages.chat_id = my_chat.chat_members.chat_id
					AND
					my_chat.chat_messages.user_id = my_chat.chat_members.user_id
					AND
					(
						SELECT COUNT(my_chat.messages.id) FROM my_chat.messages
						WHERE
							my_chat.chat_messages.message_id = my_chat.messages.id
							AND
							UNIX_TIMESTAMP(my_chat.messages.last_tick) BETWEEN @left AND @right
						#LIMIT 1 -- this line is commented because of there is one record in my_chat.messages with my_chat.messages.id
					) > 0
				LIMIT 1
			) > 0
			AND my_chat.chat_members.chat_id = my_chat.chats.id
		LIMIT offset_, number;
	COMMIT;
END//

DROP PROCEDURE IF EXISTS GET_USER_FRIENDS//
CREATE PROCEDURE GET_USER_FRIENDS(userID BIGINT UNSIGNED, offset_ BIGINT UNSIGNED, number BIGINT UNSIGNED)
BEGIN
	START TRANSACTION;
		SELECT
			my_chat.users.id,
			UNIX_TIMESTAMP(my_chat.users.last_tick),
			my_chat.users.log,
			my_chat.users.name
		FROM my_chat.friends, my_chat.users
		WHERE my_chat.friends.user_id = userID AND my_chat.friends.friend_id = my_chat.users.id
		LIMIT offset_, number;
	COMMIT;
END//

DROP PROCEDURE IF EXISTS GET_MY_FRIENDS//
CREATE PROCEDURE GET_MY_FRIENDS(userID BIGINT UNSIGNED, offset_ BIGINT UNSIGNED, number BIGINT UNSIGNED)
BEGIN
	START TRANSACTION;
		SELECT
			my_chat.users.id,
			UNIX_TIMESTAMP(my_chat.users.last_tick),
			my_chat.users.log,
			my_chat.users.name,
			
			my_chat.friends_chats.chat_id
		FROM my_chat.friends, my_chat.friends_chats, my_chat.users
		WHERE my_chat.friends.user_id = userID AND my_chat.friends.user_id = my_chat.users.id AND
				((	my_chat.friends.user_id = my_chat.friends_chats.user_id1
					AND
					my_chat.friends.friend_id = my_chat.friends_chats.user_id2
				  )
				  OR
				  (
					my_chat.friends.friend_id = my_chat.friends_chats.user_id1
					AND
					my_chat.friends.user_id = my_chat.friends_chats.user_id2
				))
		LIMIT offset_, number;
	COMMIT;
END//

DROP PROCEDURE IF EXISTS FIND_USER_FRIENDS//
CREATE PROCEDURE FIND_USER_FRIENDS(userID BIGINT UNSIGNED, regular_user_name VARCHAR(40), offset_ BIGINT UNSIGNED, number BIGINT UNSIGNED)
BEGIN
	START TRANSACTION;
		SELECT
			my_chat.users.id,
			UNIX_TIMESTAMP(my_chat.users.last_tick),
			my_chat.users.log,
			my_chat.users.name
		FROM my_chat.friends, my_chat.users
		WHERE
			my_chat.friends.user_id = userID
			AND my_chat.friends.friend_id = my_chat.users.id
			AND (my_chat.users.name REGEXP regular_user_name)
		LIMIT offset_, number;
	COMMIT;
END//

DROP PROCEDURE IF EXISTS FIND_MY_FRIENDS//
CREATE PROCEDURE FIND_MY_FRIENDS(userID BIGINT UNSIGNED, regular_user_name VARCHAR(40), offset_ BIGINT UNSIGNED, number BIGINT UNSIGNED)
BEGIN
	START TRANSACTION;
		SELECT
			my_chat.users.id,
			UNIX_TIMESTAMP(my_chat.users.last_tick),
			my_chat.users.log,
			my_chat.users.name,
			
			my_chat.friends_chats.chat_id
		FROM my_chat.friends, my_chat.friends_chats, my_chat.users
		WHERE my_chat.friends.user_id = userID AND my_chat.friends.user_id = my_chat.users.id
			AND (my_chat.users.name REGEXP regular_user_name)
			AND
			((	my_chat.friends.user_id = my_chat.friends_chats.user_id1
				AND
				my_chat.friends.friend_id = my_chat.friends_chats.user_id2
			  )
			  OR
			  (
				my_chat.friends.friend_id = my_chat.friends_chats.user_id1
				AND
				my_chat.friends.user_id = my_chat.friends_chats.user_id2
			))
		LIMIT offset_, number;
	COMMIT;
END//

DROP PROCEDURE IF EXISTS GET_REQUESTS_TO_ME//
CREATE PROCEDURE GET_REQUESTS_TO_ME(userID BIGINT UNSIGNED, offset_ BIGINT UNSIGNED, number BIGINT UNSIGNED)
BEGIN
	START TRANSACTION;
		SELECT
			my_chat.users.id,
			UNIX_TIMESTAMP(my_chat.users.last_tick),
			my_chat.users.log,
			my_chat.users.name,
			
			my_chat.requests.message
		FROM my_chat.requests, my_chat.users
		WHERE my_chat.requests.user_id = userID AND my_chat.users.id = my_chat.requests.requester_id
		LIMIT offset_, number;
	COMMIT;
END//

DROP PROCEDURE IF EXISTS GET_REQUESTS_FROM_ME//
CREATE PROCEDURE GET_REQUESTS_FROM_ME(userID BIGINT UNSIGNED, offset_ BIGINT UNSIGNED, number BIGINT UNSIGNED)
BEGIN
	START TRANSACTION;
		SELECT
			my_chat.users.id,
			UNIX_TIMESTAMP(my_chat.users.last_tick),
			my_chat.users.log,
			my_chat.users.name,
			
			my_chat.requests.message
		FROM my_chat.requests, my_chat.users
		WHERE my_chat.requests.requester_id = userID AND my_chat.users.id = my_chat.requests.user_id
		LIMIT offset_, number;
	COMMIT;
END//

DROP PROCEDURE IF EXISTS GET_CHAT_MEMBERS//
CREATE PROCEDURE GET_CHAT_MEMBERS(userID BIGINT UNSIGNED, chatID BIGINT UNSIGNED, offset_ BIGINT UNSIGNED, number BIGINT UNSIGNED)
BEGIN
	START TRANSACTION;
		SET @var = (SELECT COUNT(*) FROM my_chat.chat_members 
					WHERE  (my_chat.chat_members.chat_id = chatID
							AND my_chat.chat_members.user_id = userID
							AND !ISNULL(my_chat.chat_members.access)
						   )
					);
					
		IF (!ISNULL(@var) AND @var != 0) THEN
			SELECT
				my_chat.users.id,
				UNIX_TIMESTAMP(my_chat.users.last_tick),
				my_chat.users.log,
				my_chat.users.name
			FROM my_chat.chat_members, my_chat.users
			WHERE !ISNULL(my_chat.chat_members.access) AND my_chat.chat_members.user_id = my_chat.users.id
			LIMIT offset_, number;
		END IF;
	COMMIT;
END//

DROP PROCEDURE IF EXISTS GET_MESSAGES//
CREATE PROCEDURE GET_MESSAGES(userID BIGINT UNSIGNED, chatID BIGINT UNSIGNED, offset_ BIGINT UNSIGNED, number BIGINT UNSIGNED)
BEGIN
	START TRANSACTION;
		SELECT
			my_chat.chat_messages.message_id,
			my_chat.chat_messages.unread,
			my_chat.messages.user_id,
			my_chat.messages.message,
			UNIX_TIMESTAMP(my_chat.messages.last_tick)
		FROM my_chat.chat_messages, my_chat.messages
		WHERE my_chat.chat_messages.chat_id = chatID AND my_chat.chat_messages.user_id = userID
			AND my_chat.messages.id = my_chat.chat_messages.message_id
		LIMIT offset_, number;
	COMMIT;
END//

DROP PROCEDURE IF EXISTS GET_CHATS//
CREATE PROCEDURE GET_CHATS(userID BIGINT UNSIGNED, offset_ BIGINT UNSIGNED, number BIGINT UNSIGNED)
BEGIN
	START TRANSACTION;
		SELECT
			my_chat.chats.id,
			my_chat.chats.chat_name,
			my_chat.chats.avatar,
			
			my_chat.chat_members.access,
			my_chat.chat_members.n_unread_messages
		FROM my_chat.chat_members, my_chat.chats
		WHERE my_chat.chat_messages.user_id = userID AND my_chat.chat_members.chat_id = my_chat.chats.id
		LIMIT offset_, number;
	COMMIT;
END//

DELIMITER ;
-- --------------------------------------------------------------------------------------