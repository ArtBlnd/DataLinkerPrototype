#include "DataLinker.h"
#include "SharedSection.h"

#include <mutex>
#include <list>
#include <queue>

#pragma warning(disable:4996)

/* =========================================================================================================== */
/* ============================================= SHARED SECTIONS ============================================= */
/* =========================================================================================================== */

volatile bool   isStopped = false;

inline void     DistroyData(infoData& data)
{
    delete[] data.pData;
}

inline infoData CreateData(size_t size)
{
    infoData data;

    data.szData = size;
    data.pData = new uint8_t[size];

    return data;
}

/* =========================================================================================================== */
/* ============================================= SERVER SECTIONS ============================================= */
/* =========================================================================================================== */

#define DATA_LINKER_DEFUALT_BUFFER_SIZE 256
#define DATA_LINKER_DEFAULT_PORT 8016
#define DATA_LINKER_DEFAULT_ADDRESS "127.0.0.2"

SOCKET          sockServer;
SOCKADDR_IN     addrServer;
std::thread*    ServerTickThread;

struct infoClient
{
    SOCKET      sockClient;
    std::thread *thread;
};

inline infoClient MakeClient(SOCKET sockClient,  std::thread* thread)
{
    infoClient client;

    client.sockClient = sockClient;

    return client;
}

inline void       DistroyClient(infoClient client)
{
    client.thread->join();
    closesocket(client.sockClient);
    delete client.thread;
    
    return;
}

std::mutex              *globalConnectionMutex;
std::list<infoClient>   *globalConnections;

void ServerSendTick(SOCKET socket)
{
    char Buffer[DATA_LINKER_DEFUALT_BUFFER_SIZE];

    int  szRetBuffer;

    while (!isStopped)
    {
        szRetBuffer = recv(socket, Buffer, DATA_LINKER_DEFUALT_BUFFER_SIZE, NULL);

        if (szRetBuffer < 0 && szRetBuffer > DATA_LINKER_DEFUALT_BUFFER_SIZE)
        {
            break;
        }

        globalConnectionMutex->lock();
        for (std::list<infoClient>::iterator it = globalConnections->begin(); it != globalConnections->end(); ++it)
        {
            if (it->sockClient == socket) continue;

            send(it->sockClient, Buffer, sizeof(szRetBuffer), 0);
        }
        globalConnectionMutex->unlock();

    }
}

void ServerAcceptTick()
{
    SOCKET       sockClient;

    if(listen(sockServer, SOMAXCONN) == SOCKET_ERROR)
        return;

    while (!isStopped)
    {
        sockClient = accept(sockServer, NULL, NULL);
        if (sockClient == INVALID_SOCKET)
        {
            continue;
        }

        globalConnectionMutex->lock();
        globalConnections->push_back(
            MakeClient(
                sockClient, 
                new std::thread(ServerSendTick, sockClient)
            )
        );
        globalConnectionMutex->unlock();
    }
}

BOOL StartServer(unsigned int index)
{
    ZeroMemory(&sockServer, sizeof(SOCKET));
    ZeroMemory(&addrServer, sizeof(SOCKADDR_IN));

    globalConnectionMutex = new std::mutex;
    globalConnections = new std::list<infoClient>;

    sockServer = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    addrServer.sin_addr.s_addr = inet_addr(DATA_LINKER_DEFAULT_ADDRESS);
    addrServer.sin_port        = htons(DATA_LINKER_DEFAULT_PORT + index);
    addrServer.sin_family      = AF_INET;


    if (bind(sockServer, (SOCKADDR*)&addrServer, sizeof(SOCKADDR_IN)) != NULL)
        return FALSE;

    setInitalized(true);
    setServerAddress(addrServer);

    ServerTickThread = new std::thread(ServerAcceptTick);

    return TRUE;
}

void StopServer()
{
    isStopped = true;

    ServerTickThread->join();
    delete ServerTickThread;

    globalConnectionMutex->lock();
    for (std::list<infoClient>::iterator it = globalConnections->begin(); it != globalConnections->end(); ++it)
    {
        DistroyClient((*it));
    }
    globalConnectionMutex->unlock();

    delete globalConnectionMutex;
    delete globalConnections;


    return;
}


/* =========================================================================================================== */
/* ============================================= CLIENT SECTIONS ============================================= */
/* =========================================================================================================== */

SOCKET            sockClient;
SOCKADDR_IN       addrTargetServer;

tProcEventHandler eHandler;
std::thread*      eThread;

void ClientEventTick()
{
    char Buffer[DATA_LINKER_DEFUALT_BUFFER_SIZE];

    while (!isStopped)
    {

        int szRecv = recv(sockClient, Buffer, DATA_LINKER_DEFUALT_BUFFER_SIZE, NULL);
        infoData data = CreateData(szRecv);

        memcpy(data.pData, Buffer, szRecv);
        eHandler(data);
        DistroyData(data);
    }

    return;
}

BOOL StartClient()
{

    if (!isInitalized()) return FALSE;

    addrTargetServer = getServerAddress();
    sockClient = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (connect(sockClient, (SOCKADDR*)&addrTargetServer, sizeof(SOCKADDR_IN)))
        return FALSE;

    eThread = new std::thread(&ClientEventTick);

    return TRUE;
}

void StopClient()
{
    isStopped = true;

    eThread->join();
    delete eThread;

    closesocket(sockClient);
}

void SetEventHandler(tProcEventHandler handler)
{
    eHandler = handler;
}

void SendData(infoData data)
{
    send(sockClient, (char*)data.pData, data.szData, NULL);
}
