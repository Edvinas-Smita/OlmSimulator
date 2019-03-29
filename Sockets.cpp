#include "Sockets.h"

BOOL WSAstarted = FALSE;

HANDLE acceptorHand = nullptr, pollerHand = nullptr, clientPollerHand = nullptr;	//infinte loop handles

ServerData serverData;
BYTE isServerRunning()
{
	return serverData.serverSocket != INVALID_SOCKET;
}

SOCKET thisClientSocket = INVALID_SOCKET;	//could maybe store this in ClientData struct but its not useful anywhere else
BYTE isPlayerConnected()
{
	return thisClientSocket != INVALID_SOCKET;
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
	if (thisClientSocket == INVALID_SOCKET && serverData.serverSocket == INVALID_SOCKET)
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


void actualAccept(SOCKET sock, sockaddr_in addr, int *waitingCount, SharedData *sharedData)
{
	(*waitingCount)--;
	BYTE response = CF_ACCEPT;
	serverData.connectedSockets[serverData.currentConSockCount] = sock;
	serverData.connectedAddrs[serverData.currentConSockCount] = addr;

	int err = send(sock, (char *) &response, 1, 0);
	if (err == SOCKET_ERROR)
	{
		dwprintf(L"send failed: %d\n", WSAGetLastError());
	}

	CatchupData catchup;
	catchup.clientData.thisClientID = serverData.currentConSockCount;
	catchup.sharedData = *sharedData;
	err = send(sock, (char *) &catchup, sizeof(CatchupData), 0);
	if (err == SOCKET_ERROR)
	{
		dwprintf(L"send failed: %d\n", WSAGetLastError());
	}

	sendToClients(PLAYER_JOIN, 0, serverData.currentConSockCount);
	wchar_t b[16];
	dwprintf(L"A client has connected. id in application: %d, SOCKET: %d, address: %s:%d\n",
			 serverData.currentConSockCount, sock, InetNtopW(AF_INET, &addr.sin_addr, b, 16), addr.sin_port);
	serverData.currentConSockCount++;
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

	if (serverData.currentConSockCount == MAX_CONS)
	{
		actualReject(aargs->sock, aargs->nWaitingForAuth, REASON_SERVER_FULL);
	}
	else if (wcscmp(buffer, aargs->pass) == 0)
	{
		actualAccept(aargs->sock, aargs->addr, aargs->nWaitingForAuth, aargs->sharedData);
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
		dwprintf(L"Awaiting connections: %d/%d clients are authenticated and %d/%d in backlog\n", serverData.currentConSockCount, MAX_CONS, i, MAX_WAITING_FOR_AUTH);
		potentialSocket = accept(arg.sock, (sockaddr *) &potentialAddr, &sz);
		if (potentialSocket == INVALID_SOCKET)
		{
			dwprintf(L"accept failed: %d\n", WSAGetLastError());
			Sleep(GENERIC_SLEEP_TIME);
			continue;
		}
		if (serverData.currentConSockCount == MAX_CONS)
		{
			actualReject(potentialSocket, &i, REASON_SERVER_FULL);
			continue;
		}

		++i;
		AcceptorAuth_Args *aaargs = (AcceptorAuth_Args*) malloc(sizeof(AcceptorAuth_Args));
		aaargs->nWaitingForAuth = &i;
		aaargs->sock = potentialSocket;
		aaargs->addr = potentialAddr;
		aaargs->pass = arg.pass;
		aaargs->sharedData = arg.sharedData;
		DWORD acceptorThrID;
		HANDLE acceptorHand = CreateThread(NULL, 0, &acceptorAuth, aaargs, 0, &acceptorThrID);
	}

	return 0;
}

void receiver(SOCKET socket, int socketID, SharedData *sharedData)
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
		deleteArrayElementAndCollapse((void *) serverData.connectedSockets, socketID, sizeof(SOCKET), serverData.currentConSockCount--, &inval);
	}
	if (commandAndValue.cmd == CHANGE_OLM_LOC && socketID != 0)
	{
		return;
	}
	if (commandAndValue.cmd == CLICK_TILE && sharedData->playerPositions[socketID] != commandAndValue.val)	//pathfind and send the adjusted msg
	{
		BYTE targetX = (BYTE) commandAndValue.val % 11, targetY = (BYTE) commandAndValue.val / 11;
		BYTE currentX = sharedData->playerPositions[socketID] % 11, currentY = sharedData->playerPositions[socketID] / 11;
		//dwprintf(L"Started pathing from (%d:%d) to (%d:%d)\n", currentX, currentY, targetX, targetY);
		for (BYTE i = 0; i < (1 + !!serverData.runStatuses[socketID]); i++)
		{
			BYTE distXMod = targetX > currentX ? targetX - currentX : currentX - targetX;
			BYTE distYMod = targetY > currentY ? targetY - currentY : currentY - targetY;
			BYTE dist = distXMod > distYMod ? distXMod : distYMod;

			if (dist > 0)
			{
				if (distXMod > distYMod)
				{
					//move a tile along x axis
					currentX += targetX > currentX ? 1 : -1;
				}
				else if (distXMod < distYMod)
				{
					//move a tile along y axis
					currentY += targetY > currentY ? 1 : -1;
				}
				else
				{
					//move a tile diagonally
					currentX += targetX > currentX ? 1 : -1;
					currentY += targetY > currentY ? 1 : -1;
				}
			}
			commandAndValue.val = currentY * 11 + currentX;
			//dwprintf(L"	--->Pathed to (%d:%d)\n", currentX, currentY);
			HANDLE sending = sendToClients(commandAndValue.cmd, commandAndValue.val, socketID);
			if (WaitForSingleObject(sending, GENERIC_SLEEP_TIME) == WAIT_TIMEOUT)
			{
				TerminateThread(sending, WAIT_TIMEOUT);
				return;
			}
		}
		return;
	}
	
	sendToClients(commandAndValue.cmd, commandAndValue.val, socketID);
}
DWORD WINAPI poller(PVOID args)
{
	int errorVal, i;
	pollfd pfd[MAX_CONS];

	while (true)
	{
		if (serverData.currentConSockCount == 0)
		{
			Sleep(GENERIC_SLEEP_TIME);
			continue;
		}

		for (i = 0; i < serverData.currentConSockCount; i++)
		{
			pfd[i].fd = serverData.connectedSockets[i];
			pfd[i].events = POLLIN;
		}
		errorVal = WSAPoll(pfd, serverData.currentConSockCount, POLL_TIMEOUT);
		if (errorVal == SOCKET_ERROR)
		{
			dwprintf(L"WSAPoll failed: %d\n", WSAGetLastError());
			Sleep(GENERIC_SLEEP_TIME);
		}
		else if (errorVal)
		{
			for (i = 0; i < serverData.currentConSockCount; i++)
			{
				if (pfd[i].revents & POLLERR)
				{
					//POLLERR comes with POLLHUP or POLLNVAL - either case remove it
					SOCKET inval = INVALID_SOCKET;
					deleteArrayElementAndCollapse((void *) serverData.connectedSockets, i, sizeof(SOCKET), serverData.currentConSockCount--, &inval);
					sendToClients(PLAYER_QUIT, 0, i);
				}
				else if (pfd[i].revents & POLLIN)
				{
					receiver(serverData.connectedSockets[i], i, (SharedData *) args);
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

	if (senderCommandAndValue.cmd == SERVER_CLOSED)
	{
		dwprintf(L"Server closed (clean)\n");
	}

	ValueAndSenderID *vsid = (ValueAndSenderID*) malloc(sizeof(ValueAndSenderID));
	*vsid = {senderCommandAndValue.val, senderCommandAndValue.sender};
	PostMessage(msgback, WM_MULTIPLAYER, senderCommandAndValue.cmd, (LPARAM) vsid);
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
				PostMessage((HWND) args, WM_MULTIPLAYER, SERVER_CLOSED, (LPARAM) nullptr);
				return 1;
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

int startServer(USHORT port, LPCWSTR pass, SharedData *sharedData)
{
	SOCKET sock = setupListenSocket(SOCK_ADDR, port);
	if (sock == INVALID_SOCKET)
	{
		dwprintf(L"listen socket setup failed: %d\n", WSAGetLastError());
		return 1;
	}
	serverData.serverSocket = sock;

	for (BYTE i = 0; i < MAX_CONS; i++)
	{
		serverData.connectedSockets[i] = INVALID_SOCKET;
		serverData.connectedAddrs[i] = {};

		serverData.runStatuses[i] = 1;
		serverData.weaponRanges[i] = 0;
		serverData.weaponSpeeds[i] = 4;
	}

	Acceptor_Args *aargs = (Acceptor_Args*) malloc(sizeof(Acceptor_Args));
	LPWSTR passCopy = (LPWSTR) malloc(256 * sizeof(WCHAR));
	lstrcpyW(passCopy, pass);
	aargs->sock = sock;
	aargs->pass = passCopy;
	aargs->sharedData = sharedData;
	DWORD acceptorThreadId;
	acceptorHand = CreateThread(NULL, 0, &acceptor, aargs, 0, &acceptorThreadId);

	DWORD pollerThreadId;
	pollerHand = CreateThread(NULL, 0, &poller, aargs->sharedData, 0, &pollerThreadId);

	return 0;
}
int startClient(HWND msgback, LPCWSTR ip, USHORT port, LPCWSTR pass)
{
#pragma region initialisation
	//Defining socket structures
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

	thisClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (thisClientSocket == INVALID_SOCKET)
	{
		errorVal = WSAGetLastError();
		dwprintf(L"socket failed: %d\n", errorVal);
		return errorVal;
	}

	dwprintf(L"Trying to connect...\n");
	errorVal = connect(thisClientSocket, (sockaddr*) &addr, sizeof(addr));
	if (errorVal == SOCKET_ERROR)
	{
		errorVal = WSAGetLastError();
		dwprintf(L"connect failed: %d\n", errorVal);
		thisClientSocket = INVALID_SOCKET;
		return errorVal;
	}
#pragma endregion
#pragma region authentication
	errorVal = send(thisClientSocket, (char*) pass, wcslen((wchar_t*) pass) * 2 + 1, 0);
	if (errorVal == SOCKET_ERROR)
	{
		errorVal = WSAGetLastError();
		dwprintf(L"send failed: %d\n", errorVal);
		thisClientSocket = INVALID_SOCKET;
		return errorVal;
	}
	BYTE auth;
	errorVal = recv(thisClientSocket, (char*) &auth, 1, 0);
	//expecting to receive 1 byte: success flags (CF_ACCEPT / CF_REJECT + reason)
	if (errorVal == SOCKET_ERROR)
	{
		errorVal = WSAGetLastError();
		dwprintf(L"recv failed: %d\n", errorVal);
		thisClientSocket = INVALID_SOCKET;
		return errorVal;
	}
	else if (errorVal == 0)
	{
		errorVal = WSAGetLastError();
		dwprintf(L"Server closed connection\n");
		thisClientSocket = INVALID_SOCKET;
		return -1;
	}
	else if (errorVal != 1)
	{
		errorVal = WSAGetLastError();
		dwprintf(L"(Client auth) Wrong response from server\n");
		thisClientSocket = INVALID_SOCKET;
		return errorVal;
	}
	else if ((auth & CF_REJECT) == CF_REJECT)
	{
		errorVal = WSAGetLastError();
		dwprintf(L"Server declined connection: 0x%x\n", auth);
		thisClientSocket = INVALID_SOCKET;
		return errorVal;
	}
#pragma endregion
#pragma region catchup
	CatchupData catchup;
	errorVal = recv(thisClientSocket, (char*) &catchup, sizeof(CatchupData), 0);
	if (errorVal == SOCKET_ERROR)
	{
		errorVal = WSAGetLastError();
		dwprintf(L"recv failed: %d\n", errorVal);
		thisClientSocket = INVALID_SOCKET;
		return errorVal;
	}
	else if (errorVal == 0)
	{
		errorVal = WSAGetLastError();
		dwprintf(L"Server closed connection\n");
		thisClientSocket = INVALID_SOCKET;
		return -1;
	}
	else if (errorVal != sizeof(CatchupData))
	{
		errorVal = WSAGetLastError();
		dwprintf(L"(Catchup) Wrong response from server\n");
		thisClientSocket = INVALID_SOCKET;
		return errorVal;
	}

	SendMessage(msgback, WM_MULTIPLAYER, DATA_CATCHUP, (LPARAM) &catchup);	//SendMessage blocks so no need to worry about memory
#pragma endregion


	DWORD pollerThreadID;
	clientPollerHand = CreateThread(NULL, 0, &clientPoller, msgback, 0, &pollerThreadID);
	return 0;
}

void stopServer()
{
	TerminateThread(acceptorHand, 0);
	TerminateThread(pollerHand, 0);
	WaitForSingleObject(sendToClients(SERVER_CLOSED, 0, 0), 30000);
	for (BYTE i = 0; i < serverData.currentConSockCount; i++)
	{
		closesocket(serverData.connectedSockets[i]);
	}
	closesocket(serverData.serverSocket);
	serverData = {};

	checkDeWSA();
}
void stopClient(BOOL manual)
{
	TerminateThread(clientPollerHand, 0);
	if (manual)
	{
		WaitForSingleObject(sendToServer(PLAYER_QUIT, 1), 60000);
	}
	closesocket(thisClientSocket);
	thisClientSocket = INVALID_SOCKET;

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
	for (BYTE i = 0; i < serverData.currentConSockCount; i++)
	{
		int err = send(serverData.connectedSockets[i], (char*) args, sizeof(SenderCmdVal), 0);
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

