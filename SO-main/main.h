#include<iostream>
#include<winsock2.h>
#include<WS2tcpip.h>
#include<windows.h>
#include<chrono>
#include<vector>
#include<string>

#include<mutex>

// Update to module, someday

namespace so {

	std::vector<std::string> get_words(char in_buff[DEFAULT_BUFLEN], int size);
	std::string login(std::vector<User>* user_vec, User data);
	std::string get_time();

	void test() {
		std::cout << "Shalom2";
	}

	int decode_signal(char(&in_buff)[DEFAULT_BUFLEN], int in_size, char(&out_buff)[DEFAULT_BUFLEN], std::vector<User>* user_vec) {
		std::vector<std::string> decoded_vec = get_words(in_buff, in_size);
		std::string mess = "";

		// End trans
		if (decoded_vec[0] == "exit") {
			return -1;
		}

		if (decoded_vec[0] == "get") {
			if (decoded_vec[1] == "time") {
				if (decoded_vec.size() < 3 || decoded_vec[2] != "/n")
					mess = "Server time: ";
				mess += get_time();
			}
		}
		else if (decoded_vec[0] == "echo")
			for (int i = 1; i < decoded_vec.size(); i++) {
				mess += decoded_vec[i] + ' ';
			}
		else if (decoded_vec[0] == "login") {
			if (decoded_vec.size() < 3)
				mess = "Invalid number of arguments";
			else {
				std::string login = decoded_vec[1];
				int type_int = std::stoi(decoded_vec[2]);
				so::USER_TYPE type = static_cast<so::USER_TYPE>(type_int);
				User user = { 0, login, type };
				mess = so::login(user_vec, user);
			}
		}
		else if (decoded_vec[0] == "admin") {
			// Arguments check
			if (decoded_vec.size() < 3)
				mess = "Invalid number of arguments";
			else {
				int id = std::stoi(decoded_vec[1]);
				// Check if really admin
				bool f_chk;
				for (int i = 0; i < user_vec->size(); i++) {
					if (user_vec->at(i).id == id && user_vec->at(i).type == USER_TYPE::Admin) {
						f_chk = true;
						break;
					}
				}
				if (f_chk) {
					if (decoded_vec[2] == "kill")
						return -9001;
				}
				else {
					mess = "No permission";
				}
			}
		}
		else
			mess = "Invalid command";

		// Save message
		for (int i = 0; i < mess.size(); i++) {
			out_buff[i] = mess[i];
		}
		return mess.size();
	}

	std::vector<std::string> get_words(char in_buff[DEFAULT_BUFLEN], int size) {
		std::vector<std::string> out;
		std::string buffer = "";
		int id = 0;
		while (id < size)
			if (in_buff[id] == ' ' && buffer.size() != 0) {
				out.push_back(buffer);
				buffer.clear();
				id++;
				continue;
			}
			else {
				buffer.push_back(in_buff[id++]);
			}
		out.push_back(buffer);
		return out;
	}

	std::string get_time() {
		std::chrono::time_point now = std::chrono::system_clock::now();
		now += std::chrono::hours(2);
		return std::format("{:%Y}:{:%m}:{:%d}, {:%H}:{:%M}", now, now, now, now, now);
	}

	std::string login(std::vector<User>* user_vec, User data) {
		// Check if user exists
		bool logged_chk = false;
		for (int i = 0; i < user_vec->size(); i++) {
			if (data.name == (*user_vec)[i].name || data.type == (*user_vec)[i].type) 
				logged_chk = true;
		}

		if (logged_chk) {
			return "f User already exists";
		}
		data.id = user_vec->size();
		user_vec->push_back(data);

		return "t " + std::to_string(data.id);
	}
}