#include "Sockets.h"

BOOL WSAstarted = FALSE;

SOCKET serverSocket = INVALID_SOCKET;
SOCKET connectedSockets[MAX_CONS] = {INVALID_SOCKET};
sockaddr_in connectedAddrs[MAX_CONS] = {{0}};
BYTE currentConSockCount = 0;
BYTE getPlayerCount(){return currentConSockCount;}

SOCKET thisClientSocket = INVALID_SOCKET;
BYTE thisClientIDInServer = 0;
BYTE getThisClientID(){return thisClientIDInServer;}


void deleteArrayElementAndCollapse(void *arr, int deleteIndex, int elementSize, int elementCount, void *filler)
{
	BYTE *ptr = (BYTE*) arr;
	for (int i = deleteIndex * elementSize; i < elementSize * (elementCount - 1); i++)
	{
		ptr[i] = ptr[i + elementSize];
	}
	BYTE *fill = (BYTE*) filler;
	for (int i = elementSize - 1; i >= 0; i--)
	{
		ptr[elementSize * (elementCount - 1) + i] = fill[i];
	}
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
struct acceptorArgs
{
	SOCKET sock;
	LPCWSTR pass;
};
struct acceptorAuthArgs
{
	int *nWaitingForAuth;
	SOCKET sock;
	sockaddr_in addr;
	LPCWSTR pass;
};
DWORD WINAPI acceptor(PVOID args)
{
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
		wprintf_s(L"Awaiting connections: %d/%d clients are authenticated and %d/%d in backlog\n", currentConSockCount, MAX_CONS, i, MAX_WAITING_FOR_AUTH);
		potentialSocket = accept(((acceptorArgs*) args)->sock, (sockaddr *) &potentialAddr, &sz);
		if (potentialSocket == INVALID_SOCKET)
		{
			wprintf_s(L"accept failed: %d\n", WSAGetLastError());
			Sleep(GENERIC_SLEEP_TIME);
			continue;
		}

		++i;
		acceptorAuthArgs aaargs = {&i, potentialSocket, potentialAddr, ((acceptorArgs*) args)->pass};
		DWORD acceptorThrID;
		HANDLE acceptorHand = CreateThread(NULL, 0, &acceptorAuth, &aaargs, 0, &acceptorThrID);
	}

	return 0;
}
void actualAccept(SOCKET sock, sockaddr_in addr, int *waitingCount)
{
	(*waitingCount)--;
	short response = CF_ACCEPT;
	response |= currentConSockCount << 8;
	connectedSockets[currentConSockCount++] = sock;
	connectedAddrs[currentConSockCount] = addr;

	int err = send(sock, (char *) &response, 2, 0);
	if (err == SOCKET_ERROR)
	{
		wprintf_s(L"send failed: %d\n", WSAGetLastError());
	}

	wchar_t b[16];
	wprintf_s(L"A client has connected. id in application: %d, SOCKET: %d, address: %s:%d\n",
			  currentConSockCount, sock, InetNtopW(AF_INET, &addr.sin_addr, b, 16), addr.sin_port);
}
void actualReject(SOCKET sock, int *waitingCount, unsigned char reasons)
{
	(*waitingCount)--;
	short response = CF_REJECT | reasons;
	int err = send(sock, (char *) &response, 2, 0);
	if (err == SOCKET_ERROR)
	{
		wprintf_s(L"send failed: %d\n", WSAGetLastError());
	}
	//closesocket(sock);
}
DWORD WINAPI acceptorAuth(PVOID args)
{
	acceptorAuthArgs *aargs = (acceptorAuthArgs *) args;
	SOCKET sock = aargs->sock;
	sockaddr_in addr = aargs->addr;

	int nBytes, err;
	char buffer[256] = {0};

	pollfd pfd = {sock, POLLIN, 0};
	err = WSAPoll(&pfd, 1, WAIT_FOR_AUTH_MS);
	if (err == SOCKET_ERROR)
	{
		wprintf_s(L"(AUTH) WSAPoll failed: %d\n", WSAGetLastError());
		actualReject(sock, aargs->nWaitingForAuth, REASON_SERVERERR);
		return -1;
	}
	else if (err == 0)
	{
		wprintf_s(L"(AUTH) WSAPoll timed out...\n");
		actualReject(sock, aargs->nWaitingForAuth, REASON_SERVERERR | REASON_TIMEOUT);
		return -2;
	}

	nBytes = recv(sock, buffer, sizeof(buffer) / sizeof(*buffer), MSG_PEEK);
	if (nBytes == SOCKET_ERROR)
	{
		wprintf_s(L"(AUTH) recv peek failed: %d\n", WSAGetLastError());
		actualReject(sock, aargs->nWaitingForAuth, REASON_SERVERERR);
		return -1;
	}

	nBytes = recv(sock, buffer, nBytes, 0);
	if (nBytes == SOCKET_ERROR)
	{
		wprintf_s(L"(AUTH) recv failed: %d\n", WSAGetLastError());
		actualReject(sock, aargs->nWaitingForAuth, REASON_SERVERERR);
		return -1;
	}
	wprintf_s(L"(AUTH) %d bytes recvd from port %d: %s\n", nBytes, addr.sin_port, buffer);

	if (wcscmp((wchar_t *) buffer, aargs->pass) == 0)
	{
		actualAccept(sock, addr, aargs->nWaitingForAuth);
	}
	else
	{
		actualReject(sock, aargs->nWaitingForAuth, REASON_INVALPASS);
	}

	return 0;
}

DWORD WINAPI poller(PVOID args)
{
	int errorVal, i;
	pollfd pfd[MAX_CONS];

	while (true)
	{
		if (currentConSockCount == 0)
		{
			Sleep(500);
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
			wprintf_s(L"WSAPoll failed: %d\n", WSAGetLastError());
			Sleep(1000);
		}
		else
		{
			wprintf_s(L"%d sockets are piping up...\n", errorVal);
			for (i = 0; i < currentConSockCount; i++)
			{
				if (pfd[i].revents & POLLERR)
				{//POLLERR comes with POLLHUP or POLLNVAL - either case remove it
					SOCKET inval = INVALID_SOCKET;
					deleteArrayElementAndCollapse((void *) connectedSockets, i, sizeof(SOCKET), currentConSockCount--, &inval);
//TODO: send clients the updated id's
				}
				else if (pfd[i].revents & POLLIN)
				{
					receiver(connectedSockets[i], i);
				}
				else if (pfd[i].revents)	//revent 0 means nothing
				{
					wprintf_s(L"unrecognized event from %d: 0x%x\n", i, pfd[i].revents);
				}
			}
		}
	}
	return MAXDWORD;
}

void receiver(SOCKET socket, int socketID)
{
	int nBytes, err;
	wchar_t buffer[1024] = {0};
	nBytes = recv(socket, (char*) buffer, sizeof(buffer) / sizeof(*buffer), MSG_PEEK);
	if (nBytes == SOCKET_ERROR)
	{
		wprintf_s(L"recv peek failed: %d\n", WSAGetLastError());
		return;
	}

	nBytes = recv(socket, (char*) buffer, nBytes, 0);
	if (nBytes == SOCKET_ERROR)
	{
		wprintf_s(L"recv failed: %d\n", WSAGetLastError());
		return;
	}
	wprintf_s(L"%d bytes recvd from [%d]: %s\n", nBytes, socketID, buffer);

	err = send(socket, (char*) buffer, nBytes, 0);
	if (err == SOCKET_ERROR)
	{
		wprintf_s(L"send failed: %d\n", WSAGetLastError());
		return;
	}
}

int startServer(LPCWSTR pass)
{
	SOCKET sock = setupListenSocket(SOCK_ADDR, SOCK_PORT);
	if (sock == INVALID_SOCKET)
	{
		wprintf_s(L"listen socket setup failed: %d\n", WSAGetLastError());
		return 1;
	}

	DWORD pollerThreadId;
	HANDLE pollerHand = CreateThread(NULL, 0, &poller, nullptr, 0, &pollerThreadId);

	acceptorArgs aargs = {sock, pass};
	DWORD acceptorThreadId;
	HANDLE acceptorHand = CreateThread(NULL, 0, &acceptor, &aargs, 0, &acceptorThreadId);

	serverSocket = sock;

	return 0;
}

int startClient(LPCWSTR pass)
{
	//Defining socket structures
	SOCKET sock = INVALID_SOCKET;
	sockaddr_in addr = {0};
	addr.sin_family = AF_INET;
	int errorVal = inet_pton(AF_INET, SOCK_ADDR, &addr.sin_addr);
	if (errorVal != 1)
	{
		wprintf_s(L"inet_pton failed: %d\n", errorVal);
		return errorVal;
	}
	addr.sin_port = htons(SOCK_PORT);

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		errorVal = WSAGetLastError();
		wprintf_s(L"socket failed: %d\n", errorVal);
		return errorVal;
	}

	wprintf_s(L"Trying to connect...\n");
	errorVal = connect(sock, (sockaddr*) &addr, sizeof(addr));
	if (errorVal == SOCKET_ERROR)
	{
		wprintf_s(L"connect failed: %d\n", errorVal);
		return errorVal;
	}
	errorVal = send(sock, (char*) pass, wcslen((wchar_t*) pass) * 2, 0);
	if (errorVal == SOCKET_ERROR)
	{
		wprintf_s(L"send failed: %d\n", errorVal);
		return errorVal;
	}
	unsigned short auth;
	errorVal = recv(sock, (char*) &auth, 2, 0);
	//expecting to receive 2 bytes (short): id on server (if success) and success flags (CF_ACCEPT / CF_REJECT + reason)
	if (errorVal == SOCKET_ERROR)
	{
		wprintf_s(L"recv failed: %d\n", errorVal);
		return errorVal;
	}
	else if (errorVal == 0)
	{
		wprintf_s(L"Server closed connection\n");
		return -1;
	}
	else if (errorVal != 2)
	{
		wprintf_s(L"Wrong response from server\n");
		return errorVal;
	}
	else if ((auth & CF_REJECT) == CF_REJECT)
	{
		wprintf_s(L"Server declined connection: 0x%x\n", auth);
		return errorVal;
	}
	printf("Connected. ID: %d\n", auth >> 8);

	/*while (printf("> "), fgetws(buffer, BUFFER_SIZE, stdin), !feof(stdin))
	{
		errorVal = send(sock, (char*) buffer, wcslen(buffer) * 2, 0);
		if (errorVal == SOCKET_ERROR)
		{
			wprintf_s(L"send failed: %d\n", WSAGetLastError());
			exit(1);
		}

		errorVal = recv(sock, (char*) buffer, BUFFER_SIZE, 0);
		if (errorVal == SOCKET_ERROR)
		{
			wprintf_s(L"recv failed: %d\n", WSAGetLastError());
			exit(1);
		}
		else if (errorVal == 0)
		{
			wprintf_s(L"Server closed connection\n");
			exit(1);
		}
		else
		{
			wprintf_s(L"echo> %s", buffer);
		}
	}
	closesocket(sock);*/

	thisClientSocket = sock;
	thisClientIDInServer = auth >> 8;
	return 0;
}

int WSA()
{
	if (!WSAstarted)
	{
		WSADATA wsaData = {0};
		if (int errorVal = WSAStartup(MAKEWORD(2, 2), &wsaData))
		{
			wprintf_s(L"WSAStartup failed: %d\n", errorVal);
			return 1;
		}
	}
	return 0;
}

int sendToServer(int option, INT64 value)
{
	return 0;
}

int sendToClients(int option, INT64 value)
{
	return 0;
}
