#pragma once

#include <WinSock2.h>

#ifdef DATALINKER_EXPORTS
#define DATALINKER_API __declspec(dllexport)
#else
#define DATALINKER_API __declspec(dllimport)
#endif

struct infoData
{
    LPVOID      pData;
    size_t      szData;
};

typedef void(*tProcEventHandler)(infoData);


/* ONLY FOR SERVER */
DATALINKER_API BOOL StartServer(unsigned int index);
DATALINKER_API void StopServer();

/* ONLY FOR CLIENT */
DATALINKER_API BOOL StartClient();
DATALINKER_API void StopClient();
DATALINKER_API void SetEventHandler(tProcEventHandler handler);
DATALINKER_API void SendData(infoData data);