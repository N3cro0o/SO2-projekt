#include<iostream>
#include<winsock2.h>
#include<WS2tcpip.h>
#include<windows.h>
#include<stdio.h>

#define SOCKET_PORT "9999"
#define DEFAULT_BUFLEN 512

#include"main.h"

int main() {
	SOCKET server_socket = INVALID_SOCKET, user_accept = INVALID_SOCKET;
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
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	wsa_err = getaddrinfo(NULL, SOCKET_PORT, &hints, &result);
	if (wsa_err != 0) {
		std::cout << "Getaddrinfo failed: " << wsa_err << std::endl;
		WSACleanup();
		return 1;
	}
	std::cout << "Got address" << std::endl;

	// Sockets
	server_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (server_socket == INVALID_SOCKET) {
		std::cout << "Invalid server socket " << WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		WSACleanup();
		return -1;
	}
	std::cout << "Socket ready" << std::endl;

	// Bind socket
	wsa_err = bind(server_socket, result->ai_addr, (int)result->ai_addrlen);
	if (wsa_err == SOCKET_ERROR) {
		std::cout << "Bind failed with error: " << WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		closesocket(server_socket);
		WSACleanup();
		return 1;
	}
	std::cout << "Socket bind sucessful" << std::endl;
	freeaddrinfo(result);

	// Listen on a Server socket
	if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
		std::cout << "Listen failed with error: " << WSAGetLastError() << std::endl;
		closesocket(server_socket);
		WSACleanup();
		return 1;
	}
	std::cout << "Listening on " << SOCKET_PORT << " port" << std::endl;

	// Accepting requests - change for endless listening and for threads
	// For now, it's sigle use only
	user_accept = accept(server_socket, NULL, NULL);
	if (user_accept == INVALID_SOCKET) {
		std::cout << "Accept failed: " << WSAGetLastError() << std::endl;
		closesocket(user_accept);
		WSACleanup();
		return 1;
	}
	closesocket(server_socket);

	// Let's talk
	char recvbuf[DEFAULT_BUFLEN];
	int rec_result, send_result;
	int recvbuflen = DEFAULT_BUFLEN;
	do {

		rec_result = recv(user_accept, recvbuf, recvbuflen, 0);
		if (rec_result > 0) {
			std::cout << "Bytes recieved: " << rec_result << std::endl;
			std::cout << "Message: " << recvbuf << std::endl;
			// Echo the buffer back to the sender
			send_result = send(user_accept, recvbuf, rec_result, 0);
			if (send_result == SOCKET_ERROR) {
				std::cout << "Send failed: " << WSAGetLastError() << std::endl;
				closesocket(user_accept);
				WSACleanup();
				return 1;
			}
			std::cout << "Bytes sent: " << send_result << std::endl;
		}
		else if (rec_result == 0)
			std::cout << "Connection closing" << std::endl;
		else {
			std::cout << "Connection failed: " << WSAGetLastError() << std::endl;
			closesocket(user_accept);
			WSACleanup();
			return 1;
		}

	} while (rec_result > 0);

	// DC from mc server
	wsa_err = shutdown(user_accept, SD_SEND);
	if (wsa_err == SOCKET_ERROR) {
		std::cout << "Shutdown failed: " << WSAGetLastError() << std::endl;
		closesocket(user_accept);
		WSACleanup();
		return 1;
	}
	closesocket(user_accept);
	WSACleanup();

	return 0;
}