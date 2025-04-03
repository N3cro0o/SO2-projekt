#include<iostream>
#include<winsock2.h>
#include<WS2tcpip.h>
#include<windows.h>
#include<stdio.h>
#include<string>
#include<vector>
#include<map>

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
std::map<std::string, int> storage;

std::mutex thread_mutex;
std::mutex user_mutex;
std::mutex storage_mutex;
std::atomic_bool server_loop = true;

int thread_func(int socket_id, SOCKET* client_socket, int max_buffer);
void thread_listen(SOCKET* server_socket);

int main() {
	// Create admin
	so::User admin = { 0, "Admin", so::USER_TYPE::Admin, false };
	user_data_vec.push_back(admin);
	// Storage dummy text
	storage.insert({ "cos", 5 });
	storage.insert({ "lorem ipsum", 12 });
	storage.insert({ "dolor sit amet", 2137 });

	SOCKET server_socket = INVALID_SOCKET;
	WSADATA wsaData;
	int wsa_err;
	WORD ver_req = MAKEWORD(2, 2);
	wsa_err = WSAStartup(ver_req, &wsaData);
	if (wsa_err != 0) {
		std::cout << "[INFO] Dll not found";
		return 1;
	}
	std::cout << "[INFO] Dll was found!" << std::endl;
	std::cout << "[INFO] Status: " << wsaData.szSystemStatus << std::endl;

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
		std::cout << "[INFO] Getaddrinfo failed: " << wsa_err << std::endl;
		WSACleanup();
		return 1;
	}
	std::cout << "[INFO] Got address" << std::endl;

	// Sockets
	server_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (server_socket == INVALID_SOCKET) {
		std::cout << "[INFO] Invalid server socket " << WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		WSACleanup();
		return -1;
	}
	std::cout << "[INFO] Socket ready" << std::endl;

	// Bind socket
	wsa_err = bind(server_socket, result->ai_addr, (int)result->ai_addrlen);
	if (wsa_err == SOCKET_ERROR) {
		std::cout << "[INFO] Bind failed with error: " << WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		closesocket(server_socket);
		WSACleanup();
		return 1;
	}
	std::cout << "[INFO] Socket bind sucessful" << std::endl;
	freeaddrinfo(result);

	// Listen on a Server socket
	std::cout << "[INFO] Listening on " << SOCKET_PORT << " port" << std::endl;
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
			std::cout << "[INFO] Listen failed with error: " << WSAGetLastError() << std::endl;
			continue;
		}

		if (sockets_count < MAX_CLIENT_SOCKETS) {
			// Accepting requests
			sockets[socket_id] = accept(*server_socket, NULL, NULL);
			if (sockets[socket_id] == INVALID_SOCKET) {
				std::cout << "[INFO] Accept failed: " << WSAGetLastError() << std::endl;
				closesocket(sockets[sockets_count]);
				continue;
			}
			std::cout << "[INFO] Accept success" << std::endl;
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

			int sendbuflen = so::decode_signal(recvbuf, rec_result, sendbuf, &user_data_vec, user_mutex);
			if (sendbuflen == -1) {
				break;
			}
			if (sendbuflen == -10) {
				// Flash buffers
				for (auto const& data : storage) {
					for (char& ch : recvbuf)
						ch = ' ';
					for (char& ch : sendbuf)
						ch = ' ';
					std::cout << "[INFO] Key = " << data.first << " Data = " << data.second << std::endl;
					std::string key = data.first, body = std::to_string(data.second);
					for (int i = 0; i < key.size(); i++) {
						sendbuf[i] = key[i];
					}
					sendbuf[key.size()] = ' ';
					for (int i = key.size() + 1; i < key.size() + body.size() + 1; i++) {
						sendbuf[i] = body[i - key.size() - 1];
					}
					sendbuflen = key.size() + body.size() + 1;
					send_result = send(*client_socket, sendbuf, sendbuflen, 0);
					if (send_result == SOCKET_ERROR) {
						std::cout << "[INFO] Send failed: " << WSAGetLastError() << std::endl;
						closesocket(*client_socket);
						return 1;
					}
					std::cout << "[INFO] Bytes sent to " << socket_id << ": " << send_result << std::endl;
					rec_result = recv(*client_socket, recvbuf, recvbuflen, 0);
				}
				// Stop sending data
				std::string end = "stop";
				for (int i = 0; i < end.size(); i++) {
					sendbuf[i] = end[i];
				}
				sendbuflen = 4;
			}
			if (sendbuflen == -9001) {
				server_loop = false;
			}
			send_result = send(*client_socket, sendbuf, sendbuflen, 0);
			if (send_result == SOCKET_ERROR) {
				std::cout << "[INFO] Send failed: " << WSAGetLastError() << std::endl;
				closesocket(*client_socket);
				return 1;
			}
			std::cout << "[INFO] Bytes sent to " << socket_id << ": " << send_result << std::endl;
		}
		else if (rec_result == 0)
			std::cout << "[INFO] Connection closing" << std::endl;
		else {
			std::cout << "[INFO] Connection failed: " << WSAGetLastError() << std::endl;
			closesocket(*client_socket);
			return 1;
		}

	} while (rec_result > 0);

	// DC from mc server
	err = shutdown(*client_socket, SD_SEND);
	if (err == SOCKET_ERROR) {
		std::cout << "[INFO] Shutdown failed: " << WSAGetLastError() << std::endl;
		closesocket(*client_socket);
	}

	std::cout << "[INFO] Thread/Socket " << socket_id << " has finished work. Terminating...\n";
	thread_mutex.lock();
	sockets_is_used[socket_id] = false;
	thread_mutex.unlock();
}