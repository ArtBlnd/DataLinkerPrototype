#include "SharedSection.h"

#pragma data_seg("SHARED") 

static struct
{
    bool          isInitalized;

    SOCKET        ServerSock;
    SOCKADDR_IN   ServerAddr;
} currentStats = { 0, };

#pragma data_seg()
#pragma comment(linker, "/SECTION:SHARED,RWS")

void            setInitalized(bool stats)
{
    currentStats.isInitalized = stats;
}

void            setServerAddress(SOCKADDR_IN address)
{
    currentStats.ServerAddr = address;
}

bool            isInitalized()
{
    return currentStats.isInitalized;
}

SOCKADDR_IN     getServerAddress()
{
    return currentStats.ServerAddr;
}