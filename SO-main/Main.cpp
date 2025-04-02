#include<iostream>
#include<winsock2.h>
#include<WS2tcpip.h>
#include<windows.h>
#include<stdio.h>
#include<vector>

#include<thread>
#include<mutex>
#include<atomic>

#define SOCKET_PORT "9999"
#define DEFAULT_BUFLEN 1024
#define MAX_CLIENT_SOCKETS 5

#include"../common.h"
#include"main.h"

int sockets_count = 0;
int socket_id = 0;
SOCKET sockets[MAX_CLIENT_SOCKETS]{ INVALID_SOCKET };
bool sockets_is_used[MAX_CLIENT_SOCKETS]{ false };
std::vector<so::User> user_data_vec;

std::mutex thread_mutex;
std::atomic_bool server_loop = true;

int thread_func(int socket_id, SOCKET* client_socket, int max_buffer);
void thread_listen(SOCKET* server_socket);

int main() {
	SOCKET server_socket = INVALID_SOCKET;
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
	std::cout << "Listening on " << SOCKET_PORT << " port" << std::endl;
	std::cout << "------------------------------------------------------\n";
	std::thread listen = std::thread(thread_listen, &server_socket);
	listen.detach();

	while (server_loop);

	listen.~thread();
	closesocket(server_socket);
	WSACleanup();

	return 0;
}

void thread_listen(SOCKET* server_socket) {
	while (1) {
		// Check for free sockets
		socket_id = -1;
		sockets_count = 0;
		for (int i = 0; i < MAX_CLIENT_SOCKETS; i++) {
			std::cout << sockets[i] << " ";
			if (!sockets_is_used[i]) {
				if (socket_id == -1)
					socket_id = i;
			}
			else
				sockets_count++;
		}
		std::cout << "ID: " << socket_id << " Count: " << sockets_count << std::endl;
		if (listen(*server_socket, SOMAXCONN) == SOCKET_ERROR) {
			std::cout << "Listen failed with error: " << WSAGetLastError() << std::endl;
			continue;
		}

		if (sockets_count < MAX_CLIENT_SOCKETS) {
			// Accepting requests
			sockets[socket_id] = accept(*server_socket, NULL, NULL);
			if (sockets[socket_id] == INVALID_SOCKET) {
				std::cout << "Accept failed: " << WSAGetLastError() << std::endl;
				closesocket(sockets[sockets_count]);
				continue;
			}
			std::cout << "Accept success" << std::endl;
			thread_mutex.lock();
			std::thread x = std::thread(thread_func, socket_id, &sockets[socket_id], DEFAULT_BUFLEN);
			sockets_is_used[socket_id] = true;
			thread_mutex.unlock();
			// Detach to run threads in parallel
			x.detach();
		}
	}
}

int thread_func(int socket_id, SOCKET* client_socket, int max_buffer) {
	// Let's talk
	char recvbuf[DEFAULT_BUFLEN];
	char sendbuf[DEFAULT_BUFLEN]{ ' ' };
	int rec_result, send_result;
	int recvbuflen = DEFAULT_BUFLEN;
	int err;
	do {
		// Print saved users
		for (int i = 0; i < user_data_vec.size(); i++) {
			so::User target = user_data_vec[i];
			so::print_user(&target);
		}
		for (char& ch : recvbuf)
			ch = ' ';

		rec_result = recv(*client_socket, recvbuf, recvbuflen, 0);
		if (rec_result > 0) {
			//std::cout << "\nBytes recieved from " << socket_id << ": " << rec_result << std::endl;
			std::cout << "Message: " << std::endl;
			for (int i = 0; i < rec_result; i++) {
				std::cout << recvbuf[i];
			}std::cout << std::endl;

			int sendbuflen = so::decode_signal(recvbuf, rec_result, sendbuf, &user_data_vec);
			if (sendbuflen == -1) {
				break;
			}
			if (sendbuflen == -9001) {
				server_loop = false;
			}
			send_result = send(*client_socket, sendbuf, sendbuflen, 0);
			if (send_result == SOCKET_ERROR) {
				std::cout << "Send failed: " << WSAGetLastError() << std::endl;
				closesocket(*client_socket);
				return 1;
			}
			std::cout << "Bytes sent to " << socket_id << ": " << send_result << std::endl;
		}
		else if (rec_result == 0)
			std::cout << "Connection closing" << std::endl;
		else {
			std::cout << "Connection failed: " << WSAGetLastError() << std::endl;
			closesocket(*client_socket);
			return 1;
		}

	} while (rec_result > 0);

	// DC from mc server
	err = shutdown(*client_socket, SD_SEND);
	if (err == SOCKET_ERROR) {
		std::cout << "Shutdown failed: " << WSAGetLastError() << std::endl;
		closesocket(*client_socket);
	}

	std::cout << "Thread/Socket " << socket_id << " has finished work. Terminating...\n";
	thread_mutex.lock();
	sockets_is_used[socket_id] = false;
	thread_mutex.unlock();
}