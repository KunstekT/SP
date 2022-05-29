// client using winsock2 

#if defined(_WIN32)
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#define _WIN32_WINNT 0x501
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#include <windows.h>
	#include <string>
#else
	#include <sys/socket.h>
	#include <sys/types.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <pthread.h>
	#include <unistd.h>
#endif

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <limits>

#define DEFAULT_PORT 4000
#define DEFAULT_BUFLEN 512

// visual studio
#pragma comment(lib, "Ws2_32.lib")

#if defined(_WIN32)
	HANDLE receiveThread;
#else
	pthread_t receiveThread;
#endif

#if defined(_WIN32)
	DWORD WINAPI receiveMessage(LPVOID lpParam) {

		SOCKET clientSock = *(SOCKET*)lpParam;
		char recvbuf[DEFAULT_BUFLEN] = { 0 };
		int buflen = DEFAULT_BUFLEN;
		int returnInt;

		while (1) {

			ZeroMemory(recvbuf, buflen);
			returnInt = recv(clientSock, recvbuf, buflen, 0);
			if (returnInt > 0) {
				if (recvbuf == "P") {
					send(clientSock, "OK", (int)strlen("OK"), 0);
					shutdown(clientSock, SD_SEND);
					exit(1);
				}
				printf("%s\n", recvbuf);
			}
			else if (returnInt == 0) {
				printf("zatvorena veza.\n");
				exit(1);
			}
			else {
				printf("recv error: %d\n", WSAGetLastError());
				exit(1);
			}
		}

		return 0;
	}
#else
	void * receiveMessage(void* lpParam) {
		int clientSock = *(int*)lpParam;

		if (clientSock == -1) {
			printf("clientSock failed");
		}
		else {
			char recvbuf[DEFAULT_BUFLEN] = { 0 };
			int recvbuflen = DEFAULT_BUFLEN;
			int rtnVal;

			while (1) {
				memset(&recvbuf, 0, recvbuflen);
				rtnVal = recv(clientSock, recvbuf, recvbuflen, 0);

				if (rtnVal > 0) {
					printf("%s\n", recvbuf);
				}
				else if (rtnVal == 'P') {
					send(clientSock, "OK", (int)strlen("OK"), 0);
					shutdown(clientSock, SHUT_WR);

				}
				else if (rtnVal == 0) {
					printf("connection closed.\n");
					exit(1);
				}
				else {
					exit(1);
				}
			}
		}

		return 0;
	}
#endif

int main(int argc, char** argv) {

	if (argc != 3) {
		printf("Bad args\n");
		return 1;
	}

	// argv[1] = ip adresa 
	// argv[2] = user name 

	#if defined(_WIN32)
		WSADATA wsaData;
	#endif
	int iResult;
	const char* username = argv[2];

	// Winsock inicijalizacija 
	#if defined(_WIN32)
		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			printf("WSAStartup failed: %d\n", iResult);
			return 1;
		}

		// Startanje socketa
		SOCKET connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (connectSocket == INVALID_SOCKET) {
			printf("socket() failed\n");
			WSACleanup();
			return 1;
		}
	#else
		// Startanje socketa
		int connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (connectSocket == -1) {
			printf("socket() failed\n");
			return 1;
		}
	#endif

	struct sockaddr_in servAddr;
	#if defined(_WIN32)
		ZeroMemory(&servAddr, sizeof(servAddr));
	#else
		memset(&servAddr, 0, sizeof(servAddr));
	#endif

	servAddr.sin_family = AF_INET;

	int size = sizeof(servAddr);
	#if defined(_WIN32)
		iResult = WSAStringToAddressA(argv[1], AF_INET, NULL, (struct sockaddr*)&servAddr, &size);
		if (iResult != 0) {
			printf("getaddrinfo failed: %d\n", iResult);
			WSACleanup();
			return 1;
		}
	#endif

	servAddr.sin_port = htons(DEFAULT_PORT);

	#if defined(__linux__)
		inet_aton(argv[1], &servAddr.sin_addr);
	#endif

	// Spajanje socketa sa serverom
	iResult = connect(connectSocket, (struct sockaddr*)&servAddr, sizeof(servAddr));
	#if defined(_WIN32)
		if (iResult == SOCKET_ERROR) {
			closesocket(connectSocket);
			connectSocket = INVALID_SOCKET;
			return 1;
		}
	#else
		if (iResult == -1) {
			printf("Could not connect!\n");
			close(connectSocket);
			connectSocket = -1;
			return 1;
		}
	#endif
		else {
			printf("uspjesno spojeno na server\n");
			printf("------------------------------\n");
		}

	#if defined(_WIN32)
		receiveThread = CreateThread(NULL, 0, receiveMessage, &connectSocket, 0, 0);
	#else
		int* x;
		x = (int*)calloc(sizeof(int), 1);
		*x = connectSocket;
		int errorcheck;
		errorcheck = pthread_create(&receiveThread, NULL, receiveMessage, (void*)x);
	#endif


	std::string Msg = "N" + std::string(username);
	const char* MsgBuf = (const char*)Msg.c_str();
	send(connectSocket, MsgBuf, (int)strlen(MsgBuf), 0);

	do {

		std::string newInputHead = username + std::string(": ");
		std::string newInput;
		std::getline(std::cin, newInput);
		std::cin.clear();

		if (newInput == "quit") {

			#if defined(_WIN32)

				DWORD exitCode;
				if (GetExitCodeThread(receiveThread, &exitCode) != 0) {
					TerminateThread(receiveThread, exitCode);
				}
				shutdown(connectSocket, SD_SEND);
				closesocket(connectSocket);
				WSACleanup();
			#else
				unsigned int exitCode;
				if (pthread_tryjoin_np(receiveThread, NULL) != 0) {
					// TerminateThread(receiveThread, exitCode);
					pthread_join(receiveThread, NULL);
				}
				shutdown(connectSocket, SHUT_WR);
				close(connectSocket);
			#endif

			return 0;
		}

		newInput = newInputHead + newInput;
		const char* msg = (const char*)newInput.c_str();
		iResult = send(connectSocket, msg, (int)strlen(msg), 0);

	} while (iResult > 0);

	// Prekid konekcije i gašenje socketa
	#if defined(_WIN32)
		iResult = shutdown(connectSocket, SD_SEND);
		if (iResult == SOCKET_ERROR) {
			printf("shutdown failed: %d\n", WSAGetLastError());
			closesocket(connectSocket);
			WSACleanup();
			return 1;
		}
		closesocket(connectSocket);
		WSACleanup();
	#else
		iResult = shutdown(connectSocket, SHUT_WR);
		if (iResult == -1) {
				printf("shutdown failed");
				close(connectSocket);
				return 1;
		}
		close(connectSocket);
	#endif

	return 0;
}