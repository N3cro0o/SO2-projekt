#include<iostream>
#include<winsock2.h>
#include<WS2tcpip.h>
#include<windows.h>
#include<stdio.h>
#include<string>

#include"../common.h"
#include "menu.h"

#define SOCKET_PORT "9999"
#define DEFAULT_BUFLEN 1024

int controll_word = 0;
so::User user_data;

int main() {
	SOCKET connect_socket = INVALID_SOCKET, user_accept = INVALID_SOCKET;
	WSADATA wsaData;
	int wsa_err;
	WORD ver_req = MAKEWORD(2, 2);
	wsa_err = WSAStartup(ver_req, &wsaData);
	if (wsa_err != 0) {
		std::cout << "Dll not found";
		return 1;
	}
	std::cout << "Dll was found!" << std::endl;
	std::cout << "Status: " << wsaData.szSystemStatus << std::endl;

	// Chceck Addr
	struct addrinfo* result = NULL, * ptr = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the local address and port to be used by the client
	wsa_err = getaddrinfo("127.0.0.1", SOCKET_PORT, &hints, &result);
	if (wsa_err != 0) {
		std::cout << "Getaddrinfo failed: " << wsa_err << std::endl;
		WSACleanup();
		return 1;
	}
	std::cout << "Got address" << std::endl;

	// Socket
	ptr = result;
	connect_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	if (connect_socket == INVALID_SOCKET) {
		std::cout << "Invalid server socket " << WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		WSACleanup();
		return -1;
	}
	std::cout << "Socket ready" << std::endl;

	// Connect to server. Try the next address returned by getaddrinfo if failed
	wsa_err = connect(connect_socket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (wsa_err == SOCKET_ERROR) {
		closesocket(connect_socket);
		connect_socket = INVALID_SOCKET;
	}
	freeaddrinfo(result);
	if (connect_socket == INVALID_SOCKET) {
		std::cout << "Unable to connect to server!" << std::endl;
		WSACleanup();
		return 1;
	}
	std::cout << "------------------------------------------------------\n";

	// Communication
	std::string in;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	int iResult;
	do {
		// Get user input
		controll_word = 0;
		int sendbuflen = 0;
		in = so::menu::menu_loop(&user_data, &controll_word);
		so::print_user(&user_data);
		const char* sendbuf = in.c_str();

		// Send buffer
		iResult = send(connect_socket, sendbuf, (int)strlen(sendbuf), 0);
		if (iResult == SOCKET_ERROR) {
			std::cout << "Send failed: " << WSAGetLastError() << std::endl;
			closesocket(connect_socket);
			WSACleanup();
			return 1;
		}
		std::cout << "Bytes sent: " << iResult << std::endl;

		// Wait for echo
		iResult = recv(connect_socket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			std::cout << "Bytes recieved: " << iResult << std::endl;
			std::cout << "Message: " << std::endl;

			// Check controll word
			// Login check
			if ((controll_word & (1 << 0) >> 0) == 1) {
				char success = recvbuf[0];
				if (success == 't') {
					std::string id_s;
					for (int i = 2; i < iResult; i++) {
						if (recvbuf[i] == ' ')
							break;
						id_s.push_back(recvbuf[i]);
					}
					int id = std::stoi(id_s);
					user_data.id = id;
					std::cout << "Logged!" << std::endl;
					continue;
				}
				else {
					for (int i = 2; i < iResult; i++) {
						recvbuf[i - 2] = recvbuf[i];
					}
					iResult -= 2;
					// Reset user data
					user_data = {};
				}
			}

			for (int i = 0; i < iResult; i++) {
				std::cout << recvbuf[i];
			}
			std::cout << std::endl;

		}
		else if (iResult == 0)
			std::cout << "Connection closed" << std::endl;
		else
			std::cout << "Connection failed: " << WSAGetLastError() << std::endl;
	} while (in != "exit");

	// shutdown the connection for sending since no more data will be sent
	// the client can still use the ConnectSocket for receiving data
	iResult = shutdown(connect_socket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		std::cout << "Shutdown failed: " << WSAGetLastError() << std::endl;
		closesocket(connect_socket);
		WSACleanup();
		return 1;
	}

	// Receive data until the server closes the connection
	do {
		iResult = recv(connect_socket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			std::cout << "Bytes recieved: " << iResult << std::endl;
			std::cout << "Message: " << recvbuf << std::endl;
		}
		else if (iResult == 0)
			std::cout << "Connection closed" << std::endl;
		else
			std::cout << "Connection failed: " << WSAGetLastError() << std::endl;
	} while (iResult > 0);

	// DC from client
	closesocket(connect_socket);
	WSACleanup();
	return 0;
}