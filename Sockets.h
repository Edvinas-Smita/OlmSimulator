#pragma once
#ifndef SOCKET_HEADER
#define SOCKET_HEADER
#include <stdio.h>
#include <stdlib.h>

#include <WinSock2.h>	//ADD TO LINKER: ws2_32.lib
#include <WS2tcpip.h>

#include "Olm parts.h"

#pragma region ServerClientDefines
#define SOCK_ADDR "127.0.0.1"

#define MAX_CONS 7
#define GENERIC_SLEEP_TIME 1000
#define WAIT_FOR_AUTH_MS 1000

#define LISTEN_BACKLOG 1
#define POLL_TIMEOUT 1000

#define MAX_WAITING_FOR_AUTH 50
#define REASON_INVALPASS 128>>0
#define REASON_TIMEOUT 128>>1
#define REASON_SERVER_FULL 128>>2
#define REASON_SERVERERR 128>>4

#define BUFFER_SIZE 1024
#pragma endregion

#define WM_MULTIPLAYER 0x5555
enum Command
{
	PLAYER_JOIN,
	DATA_CATCHUP,
	PLAYER_QUIT,
	SERVER_CLOSED,
	CHANGE_COLOR,
	CLICK_TILE,
	CLICK_PART,
	CHANGE_WEAPON,
	CHANGE_OLM_LOC,
	TOGGLE_RUN,
	UPDATE_STATS,
};

BYTE			isServerRunning();
BYTE			isPlayerConnected();
int				WSA();
int				deWSA();
int				checkDeWSA();
void			deleteArrayElementAndCollapse(void *arr, int deleteIndex, int elementSize, int elementCount, void *filler);
void			dwprintf(const WCHAR *format, ...);	//unrelated to sockets

struct ClientData	//aka data of one player
{
	BYTE thisClientID;
};
struct ServerData
{
	SOCKET serverSocket = INVALID_SOCKET;
	SOCKET connectedSockets[MAX_CONS];
	sockaddr_in connectedAddrs[MAX_CONS];
	BYTE currentConSockCount = 0;
	BYTE runStatuses[MAX_CONS];
	BYTE weaponRanges[MAX_CONS];
	BYTE weaponSpeeds[MAX_CONS];
	BYTE attackCooldown[MAX_CONS];
};
struct SharedData
{
	BYTE playerCount = 0;
	BYTE playerPositions[MAX_CONS];
	COLORREF playerColors[MAX_CONS];

	OlmLocation olmLocation = West;
	int statsGrid[MAX_CONS];	//stats for player clicks on grid
	int statsParts[MAX_CONS];	//stats for player clicks on parts
};
#define notin sizeof(SharedData);
struct CatchupData
{
	ClientData clientData;
	SharedData sharedData;
};

struct CmdVal
{
	Command cmd;
	UINT64 val;
};
struct SenderCmdVal
{
	BYTE sender;
	Command cmd;
	UINT64 val;
};
struct ValueAndSenderID
{
	UINT64 value;
	BYTE sender;
};

struct AcceptorAuth_Args
{
	int *nWaitingForAuth;
	SOCKET sock;
	sockaddr_in addr;
	LPWSTR pass;
	SharedData *sharedData;
};
struct Acceptor_Args
{
	SOCKET sock;
	LPWSTR pass;
	SharedData *sharedData;
};


void			actualAccept(SOCKET sock, sockaddr_in addr, int *waitingCount, SharedData *sharedData);
void			actualReject(SOCKET sock, int *waitingCount, BYTE reasons);
DWORD WINAPI	acceptorAuth(PVOID args);
DWORD WINAPI	acceptor(PVOID args);

void			receiver(SOCKET socket, int socketID, SharedData *sharedData);
DWORD WINAPI	poller(PVOID args);
SOCKET			setupListenSocket(PCSTR ip, unsigned short port);

void			handleMessageFromServer(HWND msgback);
DWORD WINAPI	clientPoller(PVOID args);

int				startServer(USHORT port, LPCWSTR pass, SharedData *sharedData);
int				startClient(HWND msgback, LPCWSTR ip, USHORT port, LPCWSTR pass);

void			stopServer();
void			stopClient(BOOL manual = TRUE);

DWORD WINAPI	sendToServerThreadStart(PVOID args);
HANDLE			sendToServer(Command command, UINT64 value);

DWORD WINAPI	sendToClientsThreadStart(PVOID args);
HANDLE			sendToClients(Command command, UINT64 value, BYTE senderID);

#endif // !SOCKET_HEADER
