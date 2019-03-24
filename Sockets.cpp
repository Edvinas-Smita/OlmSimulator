#include "Sockets.h"

BOOL WSAstarted = FALSE;

HANDLE acceptorHand = nullptr, pollerHand = nullptr, clientPollerHand = nullptr;	//infinte loop handles

SOCKET serverSocket = INVALID_SOCKET;
BYTE isServerRunning()
{
	return serverSocket != INVALID_SOCKET;
}
SOCKET connectedSockets[MAX_CONS];
sockaddr_in connectedAddrs[MAX_CONS];
BYTE currentConSockCount = 0;	//server count of connected sockets

SOCKET thisClientSocket = INVALID_SOCKET;
BYTE isPlayerConnected()
{
	return thisClientSocket != INVALID_SOCKET;
}
BYTE playerCount = 0;			//client count of other clients in the server
BYTE getPlayerCount()
{
	return playerCount;
}
BYTE thisClientIDInServer = 0;
BYTE getThisPlayerID()
{
	return thisClientIDInServer;
}

int WSA()
{
	if (!WSAstarted)
	{
		WSADATA wsaData = {0};
		if (int errorVal = WSAStartup(MAKEWORD(2, 2), &wsaData))
		{
			dwprintf(L"WSAStartup failed: %d\n", errorVal);
			return 1;
		}
	}
	return 0;
}
int deWSA()
{
	if (WSAstarted)
	{
		if (int errorVal = WSACleanup())
		{
			dwprintf(L"WSACleanup failed(???????): %d\n", errorVal);
			return 1;
		}
	}
	return 0;
}
int checkDeWSA()
{
	if (thisClientSocket == INVALID_SOCKET && serverSocket == INVALID_SOCKET)
	{
		return deWSA();
	}
	return 0;
}
void deleteArrayElementAndCollapse(void *arr, int deleteIndex, int elementSize, int elementCount, void *filler)
{
	memmove_s(
		(BYTE*) arr + deleteIndex * elementSize,
		(elementCount - deleteIndex) * elementSize,
		(BYTE*) arr + (deleteIndex + 1) * elementSize,
		(elementCount - deleteIndex - 1) * elementSize
	);
	/*BYTE *ptr = (BYTE*) arr;
	for (int i = deleteIndex * elementSize; i < elementSize * (elementCount - 1); i++)
	{
		ptr[i] = ptr[i + elementSize];
	}
	BYTE *fill = (BYTE*) filler;*/
	for (int i = elementSize - 1; i >= 0; i--)
	{
		((BYTE*) arr)[elementSize * (elementCount - 1) + i] = ((BYTE*) filler)[i];
	}
}
void dwprintf(const WCHAR *format, ...)
{
	va_list args;
	va_start(args, format);
	WCHAR buffer[1024];
	vswprintf_s(buffer, 1024, format, args);
	OutputDebugString(buffer);
	va_end(args);
}


void actualAccept(SOCKET sock, sockaddr_in addr, int *waitingCount, COLORREF *allPlayerColors, BYTE *allPlayerPositions)
{
	(*waitingCount)--;
	BYTE response = CF_ACCEPT;
	connectedSockets[currentConSockCount] = sock;
	connectedAddrs[currentConSockCount] = addr;

	int err = send(sock, (char *) &response, 1, 0);
	if (err == SOCKET_ERROR)
	{
		dwprintf(L"send failed: %d\n", WSAGetLastError());
	}

	CatchupData data;
	data.playerCount = currentConSockCount;
	memcpy_s(&data.allPlayerColors, sizeof(data.allPlayerColors), allPlayerColors, MAX_CONS * sizeof(COLORREF));
	memcpy_s(&data.allPlayerPositions, sizeof(data.allPlayerPositions), allPlayerPositions, MAX_CONS * sizeof(BYTE));
	err = send(sock, (char *) &data, sizeof(CatchupData), 0);
	if (err == SOCKET_ERROR)
	{
		dwprintf(L"send failed: %d\n", WSAGetLastError());
	}

	sendToClients(PLAYER_JOIN, currentConSockCount, currentConSockCount);
	wchar_t b[16];
	dwprintf(L"A client has connected. id in application: %d, SOCKET: %d, address: %s:%d\n",
			 currentConSockCount, sock, InetNtopW(AF_INET, &addr.sin_addr, b, 16), addr.sin_port);
	currentConSockCount++;
}
void actualReject(SOCKET sock, int *waitingCount, BYTE reasons)
{
	(*waitingCount)--;
	BYTE response = reasons | CF_REJECT;
	int err = send(sock, (char *) &response, 1, 0);
	if (err == SOCKET_ERROR)
	{
		dwprintf(L"send failed: %d\n", WSAGetLastError());
	}
	//closesocket(sock);
}
DWORD WINAPI acceptorAuth(PVOID args)
{
	AcceptorAuth_Args *aargs = (AcceptorAuth_Args *) args;

	int nBytes, err;
	WCHAR buffer[256] = {0};

	pollfd pfd = {0};
	pfd.fd = aargs->sock;
	pfd.events = POLLIN;
	err = WSAPoll(&pfd, 1, WAIT_FOR_AUTH_MS);
	if (err == SOCKET_ERROR)
	{
		dwprintf(L"(AUTH) WSAPoll failed: %d\n", WSAGetLastError());
		actualReject(aargs->sock, aargs->nWaitingForAuth, REASON_SERVERERR);
		free(aargs);
		return -1;
	}
	else if (err == 0)
	{
		dwprintf(L"(AUTH) WSAPoll timed out...\n");
		actualReject(aargs->sock, aargs->nWaitingForAuth, REASON_SERVERERR | REASON_TIMEOUT);
		free(aargs);
		return -2;
	}

	nBytes = recv(aargs->sock, (char*) buffer, sizeof(buffer) / sizeof(*buffer), MSG_PEEK);
	if (nBytes == SOCKET_ERROR)
	{
		dwprintf(L"(AUTH) recv peek failed: %d\n", WSAGetLastError());
		actualReject(aargs->sock, aargs->nWaitingForAuth, REASON_SERVERERR);
		free(aargs);
		return -1;
	}

	nBytes = recv(aargs->sock, (char*) buffer, nBytes, 0);
	if (nBytes == SOCKET_ERROR)
	{
		dwprintf(L"(AUTH) recv failed: %d\n", WSAGetLastError());
		actualReject(aargs->sock, aargs->nWaitingForAuth, REASON_SERVERERR);
		free(aargs);
		return -1;
	}
	dwprintf(L"(AUTH) %d bytes recvd from port %d: %s\n", nBytes, aargs->addr.sin_port, buffer);

	if (wcscmp(buffer, aargs->pass) == 0)
	{
		actualAccept(aargs->sock, aargs->addr, aargs->nWaitingForAuth, aargs->allPlayerColors, aargs->allPlayerPositions);
	}
	else
	{
		actualReject(aargs->sock, aargs->nWaitingForAuth, REASON_INVALPASS);
	}

	free(aargs);
	return 0;
}
DWORD WINAPI acceptor(PVOID args)
{
	Acceptor_Args arg = *(Acceptor_Args*) args;
	free(args);
	for (int i = 0;;)
	{
		if (i == MAX_WAITING_FOR_AUTH)
		{
			Sleep(GENERIC_SLEEP_TIME);
			continue;
		}
		SOCKET potentialSocket = INVALID_SOCKET;
		sockaddr_in potentialAddr;
		int sz = sizeof(sockaddr_in);
		dwprintf(L"Awaiting connections: %d/%d clients are authenticated and %d/%d in backlog\n", currentConSockCount, MAX_CONS, i, MAX_WAITING_FOR_AUTH);
		potentialSocket = accept(arg.sock, (sockaddr *) &potentialAddr, &sz);
		if (potentialSocket == INVALID_SOCKET)
		{
			dwprintf(L"accept failed: %d\n", WSAGetLastError());
			Sleep(GENERIC_SLEEP_TIME);
			continue;
		}

		++i;
		AcceptorAuth_Args *aaargs = (AcceptorAuth_Args*) malloc(sizeof(AcceptorAuth_Args));
		aaargs->nWaitingForAuth = &i;
		aaargs->sock = potentialSocket;
		aaargs->addr = potentialAddr;
		aaargs->pass = arg.pass;
		aaargs->allPlayerColors = arg.allPlayerColors;
		aaargs->allPlayerPositions = arg.allPlayerPositions;
		DWORD acceptorThrID;
		HANDLE acceptorHand = CreateThread(NULL, 0, &acceptorAuth, aaargs, 0, &acceptorThrID);
	}

	return 0;
}

void receiver(SOCKET socket, int socketID)
{
	int err;
	CmdVal commandAndValue;
	err = recv(socket, (char*) &commandAndValue, sizeof(CmdVal), 0);
	if (err == SOCKET_ERROR)
	{
		dwprintf(L"recv failed: %d\n", WSAGetLastError());
		return;
	}

	if (commandAndValue.cmd == PLAYER_QUIT)
	{
		SOCKET inval = INVALID_SOCKET;
		deleteArrayElementAndCollapse((void *) connectedSockets, socketID, sizeof(SOCKET), currentConSockCount--, &inval);
	}
	
	sendToClients(commandAndValue.cmd, commandAndValue.val, socketID);
}
DWORD WINAPI poller(PVOID args)
{
	int errorVal, i;
	pollfd pfd[MAX_CONS];

	while (true)
	{
		if (currentConSockCount == 0)
		{
			Sleep(GENERIC_SLEEP_TIME);
			continue;
		}

		for (i = 0; i < currentConSockCount; i++)
		{
			pfd[i].fd = connectedSockets[i];
			pfd[i].events = POLLIN;
		}
		errorVal = WSAPoll(pfd, currentConSockCount, POLL_TIMEOUT);
		if (errorVal == SOCKET_ERROR)
		{
			dwprintf(L"WSAPoll failed: %d\n", WSAGetLastError());
			Sleep(GENERIC_SLEEP_TIME);
		}
		else if (errorVal)
		{
			//dwprintf(L"%d sockets are piping up...\n", errorVal);
			for (i = 0; i < currentConSockCount; i++)
			{
				if (pfd[i].revents & POLLERR)
				{//POLLERR comes with POLLHUP or POLLNVAL - either case remove it
					SOCKET inval = INVALID_SOCKET;
					deleteArrayElementAndCollapse((void *) connectedSockets, i, sizeof(SOCKET), currentConSockCount--, &inval);
					sendToClients(PLAYER_QUIT, 0, i);
				}
				else if (pfd[i].revents & POLLIN)
				{
					receiver(connectedSockets[i], i);
				}
				else if (pfd[i].revents)	//revent 0 means nothing
				{
					dwprintf(L"unrecognized event from %d: 0x%x\n", i, pfd[i].revents);
				}
			}
		}
	}
	return MAXDWORD;
}
SOCKET setupListenSocket(PCSTR ip, unsigned short port)
{
	int errorVal;

	SOCKET sock = INVALID_SOCKET;
	sockaddr_in addr = {0};
	addr.sin_family = AF_INET;
	errorVal = inet_pton(AF_INET, ip, &addr.sin_addr);
	if (errorVal != 1)
	{
		return INVALID_SOCKET;
	}
	addr.sin_port = htons(port);

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		return INVALID_SOCKET;
	}

	printf("Trying to bind...\n");
	errorVal = bind(sock, (sockaddr*) &addr, sizeof(addr));
	if (errorVal == SOCKET_ERROR)
	{
		closesocket(sock);
		return INVALID_SOCKET;
	}

	errorVal = listen(sock, LISTEN_BACKLOG);
	if (errorVal == SOCKET_ERROR)
	{
		closesocket(sock);
		return INVALID_SOCKET;
	}

	return sock;
}

void handleMessageFromServer(HWND msgback)
{
	SenderCmdVal senderCommandAndValue;
	int nBytes = recv(thisClientSocket, (char*) &senderCommandAndValue, sizeof(SenderCmdVal), 0);
	if (nBytes == SOCKET_ERROR)
	{
		dwprintf(L"recv failed: %d\n", WSAGetLastError());
		return;
	}

	if (senderCommandAndValue.cmd == PLAYER_JOIN)
	{
		playerCount++;// = senderCommandAndValue.sender;
		//return;
	}
	else if (senderCommandAndValue.cmd == PLAYER_QUIT)
	{
		playerCount--;
		if (senderCommandAndValue.sender < thisClientIDInServer)
		{
			thisClientIDInServer--;
		}
	}
	if (senderCommandAndValue.cmd == SERVER_CLOSED)
	{
		dwprintf(L"Server closed (clean)\n");
	}

	ValueAndSenderID *vsid = (ValueAndSenderID*) malloc(sizeof(ValueAndSenderID));
	*vsid = {senderCommandAndValue.val, senderCommandAndValue.sender};
	SendMessage(msgback, WM_MULTIPLAYER, senderCommandAndValue.cmd, (LPARAM) vsid);
}
DWORD WINAPI clientPoller(PVOID args)
{
	int errorVal;
	pollfd pfd = {thisClientSocket, POLLIN};

	while (true)
	{
		errorVal = WSAPoll(&pfd, 1, POLL_TIMEOUT);
		if (errorVal == SOCKET_ERROR)
		{
			dwprintf(L"(CLIENT) WSAPoll failed: %d\n", WSAGetLastError());
			Sleep(GENERIC_SLEEP_TIME);
		}
		else
		{
			if (pfd.revents & POLLERR)
			{
				dwprintf(L"Server closed (not clean)\n");
				SendMessage((HWND) args, WM_MULTIPLAYER, SERVER_CLOSED, (LPARAM) nullptr);
			}
			else if (pfd.revents & POLLIN)
			{
				handleMessageFromServer((HWND) args);
			}
			else if (pfd.revents)	//revent 0 means nothing
			{
				dwprintf(L"unrecognized event: 0x%x\n", pfd.revents);
			}
		}
	}
	return MAXDWORD;
}

int startServer(USHORT port, LPCWSTR pass, COLORREF *allPlayerColors, BYTE *allPlayerPositions)
{
	SOCKET sock = setupListenSocket(SOCK_ADDR, port);
	if (sock == INVALID_SOCKET)
	{
		dwprintf(L"listen socket setup failed: %d\n", WSAGetLastError());
		return 1;
	}
	serverSocket = sock;

	for (BYTE i = 0; i < MAX_CONS; i++)
	{
		connectedSockets[i] = INVALID_SOCKET;
	}

	Acceptor_Args *aargs = (Acceptor_Args*) malloc(sizeof(Acceptor_Args));
	LPWSTR passCopy = (LPWSTR) malloc(256 * sizeof(WCHAR));
	lstrcpyW(passCopy, pass);
	aargs->sock = sock;
	aargs->pass = passCopy;
	aargs->allPlayerColors = allPlayerColors;
	aargs->allPlayerPositions = allPlayerPositions;
	DWORD acceptorThreadId;
	acceptorHand = CreateThread(NULL, 0, &acceptor, aargs, 0, &acceptorThreadId);

	DWORD pollerThreadId;
	pollerHand = CreateThread(NULL, 0, &poller, nullptr, 0, &pollerThreadId);

	return 0;
}
int startClient(HWND msgback, LPCWSTR ip, USHORT port, LPCWSTR pass, COLORREF *allPlayerColors, BYTE *allPlayerPositions)
{
#pragma region initialisation
	//Defining socket structures
	SOCKET sock = INVALID_SOCKET;
	sockaddr_in addr = {0};
	char unretardedIP[16];
	sprintf_s(unretardedIP, "%ws", ip);
	addr.sin_family = AF_INET;
	int errorVal = inet_pton(AF_INET, unretardedIP, &addr.sin_addr);
	if (errorVal != 1)
	{
		dwprintf(L"inet_pton failed: %d\n", errorVal);
		return errorVal;
	}
	addr.sin_port = htons(port);

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	thisClientSocket = sock;
	if (sock == INVALID_SOCKET)
	{
		errorVal = WSAGetLastError();
		dwprintf(L"socket failed: %d\n", errorVal);
		return errorVal;
	}

	dwprintf(L"Trying to connect...\n");
	errorVal = connect(sock, (sockaddr*) &addr, sizeof(addr));
	if (errorVal == SOCKET_ERROR)
	{
		errorVal = WSAGetLastError();
		dwprintf(L"connect failed: %d\n", errorVal);
		return errorVal;
	}
#pragma endregion
#pragma region authentication
	errorVal = send(sock, (char*) pass, wcslen((wchar_t*) pass) * 2 + 1, 0);
	if (errorVal == SOCKET_ERROR)
	{
		errorVal = WSAGetLastError();
		dwprintf(L"send failed: %d\n", errorVal);
		return errorVal;
	}
	BYTE auth;
	errorVal = recv(sock, (char*) &auth, 1, 0);
	//expecting to receive 1 byte: success flags (CF_ACCEPT / CF_REJECT + reason)
	if (errorVal == SOCKET_ERROR)
	{
		errorVal = WSAGetLastError();
		dwprintf(L"recv failed: %d\n", errorVal);
		return errorVal;
	}
	else if (errorVal == 0)
	{
		errorVal = WSAGetLastError();
		dwprintf(L"Server closed connection\n");
		return -1;
	}
	else if (errorVal != 1)
	{
		errorVal = WSAGetLastError();
		dwprintf(L"(Client auth) Wrong response from server\n");
		return errorVal;
	}
	else if ((auth & CF_REJECT) == CF_REJECT)
	{
		errorVal = WSAGetLastError();
		dwprintf(L"Server declined connection: 0x%x\n", auth);
		return errorVal;
	}
#pragma endregion
#pragma region catchup
	CatchupData data;
	errorVal = recv(sock, (char*) &data, sizeof(CatchupData), 0);
	if (errorVal == SOCKET_ERROR)
	{
		errorVal = WSAGetLastError();
		dwprintf(L"recv failed: %d\n", errorVal);
		return errorVal;
	}
	else if (errorVal == 0)
	{
		errorVal = WSAGetLastError();
		dwprintf(L"Server closed connection\n");
		return -1;
	}
	else if (errorVal != sizeof(data))
	{
		errorVal = WSAGetLastError();
		dwprintf(L"(Catchup) Wrong response from server\n");
		return errorVal;
	}
	thisClientIDInServer = playerCount = data.playerCount;
	memcpy_s(allPlayerColors, MAX_CONS * sizeof(COLORREF), &data.allPlayerColors, MAX_CONS * sizeof(COLORREF));
	memcpy_s(allPlayerPositions, MAX_CONS * sizeof(BYTE), &data.allPlayerPositions, MAX_CONS * sizeof(BYTE));

	SendMessage(msgback, WM_MULTIPLAYER, DATA_CATCHUP, (LPARAM) nullptr);
#pragma endregion


	DWORD pollerThreadID;
	clientPollerHand = CreateThread(NULL, 0, &clientPoller, msgback, 0, &pollerThreadID);
	return 0;
}

void stopServer()
{
	TerminateThread(acceptorHand, 0);
	TerminateThread(pollerHand, 0);
	WaitForSingleObject(sendToClients(SERVER_CLOSED, 0, 0), INFINITE);
	for (BYTE i = 0; i < currentConSockCount; i++)
	{
		closesocket(connectedSockets[i]);
		connectedSockets[i] = INVALID_SOCKET;
		connectedAddrs[i] = {0};
	}
	closesocket(serverSocket);
	serverSocket = INVALID_SOCKET;
	currentConSockCount = 0;

	checkDeWSA();
}
void stopClient(BOOL manual)
{
	TerminateThread(clientPollerHand, 0);
	if (manual)
	{
		WaitForSingleObject(sendToServer(PLAYER_QUIT, 1), INFINITE);
	}
	closesocket(thisClientSocket);
	thisClientSocket = INVALID_SOCKET;
	playerCount = 0;
	thisClientIDInServer = 0;

	checkDeWSA();
}

DWORD WINAPI sendToServerThreadStart(PVOID args)
{
	int errorVal = send(thisClientSocket, (char*) args, sizeof(CmdVal), 0);
	free(args);
	if (errorVal == SOCKET_ERROR)
	{
		errorVal = WSAGetLastError();
		dwprintf(L"send failed: %d\n", errorVal);
		return errorVal;
	}
	return 0;
}
HANDLE sendToServer(Command command, UINT64 value)
{
	CmdVal *args = (CmdVal*) malloc(sizeof(CmdVal));
	*args = {command, value};
	DWORD threadID;
	return CreateThread(NULL, 0, &sendToServerThreadStart, args, 0, &threadID);
}

DWORD WINAPI sendToClientsThreadStart(PVOID args)
{
	for (BYTE i = 0; i < currentConSockCount; i++)
	{
		int err = send(connectedSockets[i], (char*) args, sizeof(SenderCmdVal), 0);
		if (err == SOCKET_ERROR)
		{
			err = WSAGetLastError();
			dwprintf(L"send failed: %d\n", err);
			free(args);
			return err;
		}
	}
	free(args);
	return 0;
}
HANDLE sendToClients(Command command, UINT64 value, BYTE senderID)
{
	SenderCmdVal *args = (SenderCmdVal*) malloc(sizeof(SenderCmdVal));
	*args = {senderID, command, value};
	DWORD threadID;
	return CreateThread(NULL, 0, &sendToClientsThreadStart, args, 0, &threadID);
}

