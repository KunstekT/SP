// server winsock2

#define _WIN32_WINNT 0x501

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <cassert>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>

#define DEFAULT_PORT 4000 
#define DEFAULT_BUFLEN 512

// visual studio
#pragma comment(lib, "Ws2_32.lib")


SOCKET constructSocket() {
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET) {
		printf("socket() failed\n");
		WSACleanup();
		exit(1);
	}
	return sock;
}

void socketErrorCheck(int returnInt, SOCKET socketToClose, const char* action) {
	const char* actionAttempted = action;
	if (returnInt == SOCKET_ERROR) {
		printf("socket error. %s failed with error: %d\n", actionAttempted, WSAGetLastError());
	}
	else {

		return;
	}
	closesocket(socketToClose);
	WSACleanup();
	exit(1);
}


int main() {

	WSADATA wsaData;

	int iResult;
	int sendResult;
	char recvbuf[DEFAULT_BUFLEN] = { 0 };
	int buflen = DEFAULT_BUFLEN;
	int currentClients = 0;

	SOCKET serverSocket = INVALID_SOCKET;
	SOCKET clientSocket = INVALID_SOCKET;

	struct sockaddr_in servAddr;
	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen;

	fd_set activeFdSet;
	fd_set readFdSet;

	std::vector<SOCKET> sockArray;

	std::unordered_map<SOCKET, std::string> sockMap;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	serverSocket = constructSocket();

	ZeroMemory(&servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(DEFAULT_PORT);

	iResult = bind(serverSocket, (struct sockaddr*)&servAddr, sizeof(servAddr));
	socketErrorCheck(iResult, serverSocket, "bind");

	iResult = listen(serverSocket, SOMAXCONN);
	socketErrorCheck(iResult, serverSocket, "listen");

	FD_ZERO(&activeFdSet);
	FD_SET(serverSocket, &activeFdSet);
	sockArray.push_back(serverSocket);

	printf("cekanje na spajanje...\n");

	std::vector<SOCKET> newSocketArray(sockArray);

	while (1) {

		readFdSet = activeFdSet;
		iResult = select(FD_SETSIZE, &readFdSet, NULL, NULL, NULL);
		socketErrorCheck(iResult, serverSocket, "select");


		for (int i = 0; i < (int)sockArray.size(); i++) {

			SOCKET currSocketFd = sockArray.at(i);

			if (FD_ISSET(currSocketFd, &readFdSet)) {

				if (currSocketFd == serverSocket) {

					clientAddrLen = sizeof(clientAddr);
					clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
					if (clientSocket == INVALID_SOCKET) {
						printf("error: %d\n", WSAGetLastError());
						closesocket(clientSocket);
						WSACleanup();
						return 1;
					}
					else {}

					FD_SET(clientSocket, &activeFdSet);

					newSocketArray.push_back(clientSocket);
				}
				else {

					ZeroMemory(recvbuf, buflen);

					iResult = recv(currSocketFd, recvbuf, buflen, 0);

					if (iResult > 0) {
						currentClients++;
						printf("%s\n", recvbuf);

						std::string msg(recvbuf);

						char identifier = msg[0];
						std::string restOfMsg = msg.substr(1, strlen(msg.c_str()));

						if (identifier == 'N') {
							std::string newClientJoinedMsg = "B" + std::to_string(currentClients);

							sockMap.insert(std::pair<SOCKET, std::string>(currSocketFd, restOfMsg));

							const char* msgToSend = newClientJoinedMsg.c_str();
							SOCKET sockFd = (SOCKET)newSocketArray.at(currentClients);
							sendResult = send(sockFd, msgToSend, (int)strlen(msgToSend) + 1, 0);
							socketErrorCheck(sendResult, sockFd, "send");
							if (currentClients == 4) {
								for (int j = 1; j < (int)newSocketArray.size(); j++) {
									if (clientSocket != (SOCKET)newSocketArray.at(j)) {
										SOCKET sockFd = (SOCKET)newSocketArray.at(j);
										sendResult = send(sockFd, "P", (int)strlen("P") + 1, 0);
										socketErrorCheck(sendResult, sockFd, "send");
									}
								}
							}
						}
						else {

							for (int j = 1; j < (int)newSocketArray.size(); j++) {
								SOCKET sockFd = (SOCKET)newSocketArray.at(j);
								sendResult = send(sockFd, recvbuf, iResult + 1, 0);
								socketErrorCheck(sendResult, sockFd, "send");
							}
						}

						ZeroMemory(recvbuf, buflen);

					}
					else if (iResult == 0) {

						printf("connection with socket %d closing (user: %s)...\n", currSocketFd, (const char*)sockMap[currSocketFd].c_str());

						iResult = shutdown(currSocketFd, SD_SEND);
						socketErrorCheck(iResult, currSocketFd, "shutdown");

						closesocket(currSocketFd);
						FD_CLR(currSocketFd, &activeFdSet);

						sockMap.erase(currSocketFd);

						std::vector<SOCKET> tempArr;
						for (auto sock : newSocketArray) {
							if ((SOCKET)sock != currSocketFd) {
								tempArr.push_back((SOCKET)sock);
							}
						}
						newSocketArray.assign(tempArr.begin(), tempArr.end());
					}
					else {
						printf("recv error: %d\n", WSAGetLastError());
						closesocket(currSocketFd);
						WSACleanup();
						return 1;
					}
					printf("\n");
				}
			}
		}
		sockArray.assign(newSocketArray.begin(), newSocketArray.end());

	}
	WSACleanup();
	return 0;
}
