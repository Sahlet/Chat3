USE my_chat;
-- comment
#comment
/*comment*/

-- --------------------------------------------------------------------------------------
DELIMITER //

DROP PROCEDURE IF EXISTS AUTORIZATION//
CREATE PROCEDURE AUTORIZATION(login VARCHAR(40), password BLOB)
BEGIN
	START TRANSACTION;
		SELECT COUNT(*) FROM my_chat.users WHERE log = login AND pass = password;
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
CREATE PROCEDURE ADDUSER(login VARCHAR(40), password BLOB)
BEGIN
	START TRANSACTION;
		INSERT INTO my_chat.users (log, pass) values(login, password);
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
             
			INSERT INTO my_chat.chat_messages (chat_id, user_id, message_id)
						SELECT (my_chat.chat_members.chat_id, my_chat.chat_members.user_id, @messageID)
						FROM my_chat.chat_members
                        WHERE my_chat.chat_members.chat_id = chatID AND NOT ISNULL(my_chat.chat_members.access);
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
CREATE PROCEDURE CREATE_CHAT(userID BIGINT UNSIGNED)
BEGIN
	START TRANSACTION;
		INSERT INTO my_chat.chats (/*chat_name, avatar*/) values(/*chat_name_, avatar_*/);
        SET @chatID = (SELECT last_insert_id());
        INSERT INTO my_chat.chat_members (chat_id, user_id, access) values(@chatID, userID, 'a');
	COMMIT;
END//

DROP PROCEDURE IF EXISTS SEND_REQUEST//
CREATE PROCEDURE SEND_REQUEST(requesterID BIGINT UNSIGNED, userID BIGINT UNSIGNED, message_ TEXT)
BEGIN
	SET @var = NULL;
	START TRANSACTION;
		SET @var = (SELECT COUNT(*) FROM my_chat.friends WHERE my_chat.friends.user_id = requesterID AND my_chat.friends.friend_id = userID);
	
		IF (ISNULL(@var) OR @var = 0) THEN
			SET @var = (SELECT COUNT(*) FROM my_chat.requests WHERE (my_chat.friends.user_id = requesterID AND my_chat.friends.requester_id = userID) OR (my_chat.friends.user_id = userID AND my_chat.friends.requester_id = requesterID));
            
            IF (ISNULL(@var) OR @var = 0) THEN
				INSERT INTO my_chat.requests (user_id, requester_id, message) values(userID, requesterID, message_);
            END IF;
		END IF;
    COMMIT;
END//

DROP PROCEDURE IF EXISTS ACCEPT_REQUEST//
CREATE PROCEDURE ACCEPT_REQUEST(userID BIGINT UNSIGNED, requesterID BIGINT UNSIGNED)
BEGIN
	SET @var = NULL;
	START TRANSACTION;
		SET @var = (SELECT COUNT(*) FROM my_chat.requests WHERE (my_chat.friends.user_id = userID AND my_chat.friends.requester_id = requesterID));
            
        IF (!ISNULL(@var) AND @var != 0) THEN
			DELETE FROM my_chat.requests WHERE (my_chat.friends.user_id = userID AND my_chat.friends.requester_id = requesterID);
            
            SET @minID = LEAST(userID, requesterID);
			SET @maxID = GREATEST(userID, requesterID);
            
            SET @var = (SELECT COUNT(*) FROM my_chat.friends_chats WHERE user_id1 = @minID AND user_id2 = @maxID);
            
            IF (ISNULL(@var) OR @var = 0) THEN
				INSERT INTO my_chat.chats () values()/*(chat_name, avatar) values(NULL, NULL)*/;
				SET @chatID = (SELECT last_insert_id());
				INSERT INTO my_chat.chat_members (chat_id, user_id, access) values(@chatID, userID, 'u');
                INSERT INTO my_chat.chat_members (chat_id, user_id, access) values(@chatID, requesterID, 'u');
            
				INSERT INTO my_chat.friends_chats (user_id1, user_id2, chat_id) values (LEAST(userID, requesterID), GREATEST(userID, requesterID), @chatID);
            END IF;
            
            INSERT INTO my_chat.friends (user_id, friend_id) values (userID, requesterID);
            INSERT INTO my_chat.friends (user_id, friend_id) values (requesterID, userID);
		END IF;
    COMMIT;
END//

DROP PROCEDURE IF EXISTS ADD_MEMBER//
CREATE PROCEDURE ADD_MEMBER(chatID BIGINT UNSIGNED, memberID BIGINT UNSIGNED, access_ ENUM('p','u'))
BEGIN
	IF (!ISNULL(access_)) THEN
		START TRANSACTION;
			/*SET @var = (SELECT COUNT(*) FROM my_chat.chat_members 
					WHERE (my_chat.chat_members.chat_id = chatID
						AND my_chat.chat_members.user_id = memberID));
					
			IF (ISNULL(@var) OR @var = 0) BEGIN*/
				INSERT INTO my_chat.chat_members (chat_id, user_id, access) values(chatID, memberID, access_);
			/*END IF;*/
		COMMIT;
	END IF;
END//

DROP PROCEDURE IF EXISTS SET_ACCESS//
CREATE PROCEDURE SET_ACCESS(chatID BIGINT UNSIGNED, memberID BIGINT UNSIGNED, access_ ENUM('p','u'))
BEGIN
	SET @var = NULL;
	START TRANSACTION;
		UPDATE my_chat.chat_members SET access = access_
			WHERE (my_chat.chat_members.chat_id = chatID
			AND my_chat.chat_members.user_id = memberID);
	COMMIT;
END//

DROP PROCEDURE IF EXISTS USER_ONLINE//
CREATE PROCEDURE USER_ONLINE(userID BIGINT UNSIGNED)
BEGIN
	START TRANSACTION;
		UPDATE my_chat.users SET last_tick = NOW() WHERE id = userID;
	COMMIT;
END//

DELIMITER ;
-- --------------------------------------------------------------------------------------