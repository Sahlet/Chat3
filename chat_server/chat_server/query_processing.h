#pragma once

#include <My\WinSockBaseSerialization.h>
#include <My\thread_pool.h>
#include "mysqlWrap.h"
#include <iostream>

void query_processor(My::WinSocket sock, mysqlWrap& connection);