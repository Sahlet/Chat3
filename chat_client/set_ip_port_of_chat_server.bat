rem REG ADD "HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v IP_of_chat_server /d 127.0.0.1 /f
rem REG ADD "HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v Port_of_chat_server /d 30000 /f
rem REG ADD "HKEY_CURRENT_USER\Environment" /v IP_of_chat_server /d 127.0.0.1 /f
rem REG ADD "HKEY_CURRENT_USER\Environment" /v Port_of_chat_server /d 30000 /f
REG ADD "HKEY_CURRENT_USER\Environment" /v IP_of_chat_server /d 127.0.0.1 /f
REG ADD "HKEY_CURRENT_USER\Environment" /v Port_of_chat_server /d 30000 /f