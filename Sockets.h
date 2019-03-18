#pragma once
#ifndef SOCKET_HEADER
#define SOCKET_HEADER
#include <stdio.h>
#include <stdlib.h>

#include <WinSock2.h>	//ADD TO LINKER: ws2_32.lib
#include <WS2tcpip.h>

#pragma region ServerClientDefines
#define SOCK_ADDR "127.0.0.1"
#define SOCK_PORT 27015

#define MAX_CONS 7
#define GENERIC_SLEEP_TIME 1000
#define WAIT_FOR_AUTH_MS 60000

#define LISTEN_BACKLOG 1
#define POLL_TIMEOUT 1000

#define MAX_WAITING_FOR_AUTH 50
#define REASON_INVALPASS 128>>0
#define REASON_TIMEOUT 128>>1
#define REASON_SERVERERR 128>>7

#define BUFFER_SIZE 1024
#pragma endregion

#pragma region CommandDefines
#define OPT_CHANGE_ID 0
#define OPT_CHANGE_COLOR 1 << 0
#define OPT_CLICK_TILE 1 << 1
#define OPT_CLICK_PART 1 << 2
#define OPT_CHANGE_WEAPON 1 << 3
//host only
#define OPT_CHANGE_OLM_LOC 1 << 4
#define OPT_TOGGLE_RUN 1 << 5
#pragma endregion

BYTE getThisClientID();
BYTE getPlayerCount();

void deleteArrayElementAndCollapse(void *arr, int deleteIndex, int elementSize, int elementCount, void *filler);
SOCKET setupListenSocket(PCSTR ip, unsigned short port);
void actualAccept(SOCKET sock, sockaddr_in addr, int *waitingCount);
void actualReject(SOCKET sock, int *waitingCount, unsigned char reasons);
DWORD WINAPI acceptorAuth(PVOID args);
DWORD WINAPI acceptor(PVOID args);
DWORD WINAPI poller(PVOID args);
void receiver(SOCKET socket, int socketID);
int startServer(LPCWSTR pass);
int sendToClients(int option, INT64 value);


int startClient(LPCWSTR pass);
int sendToServer(int option, INT64 value);

int WSA();

#endif // !SOCKET_HEADER
