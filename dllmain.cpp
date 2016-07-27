#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

BOOL InitalizeSocket()
{
    WSADATA  data;

    if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
        return FALSE;

    return TRUE;
}

BOOL ReleaseSocket()
{
    WSACleanup();

    return TRUE;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
        return InitalizeSocket();
        break;

    case DLL_PROCESS_DETACH:
        return ReleaseSocket();
        break;

	}
	return TRUE;
}
