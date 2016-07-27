#pragma once

#include <WinSock2.h>


void            setInitalized(bool stats);
void            setServerAddress(SOCKADDR_IN address);
bool            isInitalized();
SOCKADDR_IN     getServerAddress();