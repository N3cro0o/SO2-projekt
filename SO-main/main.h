#include<iostream>
#include<winsock2.h>
#include<WS2tcpip.h>
#include<windows.h>
#include<chrono>
#include<vector>
#include<map>
#include<string>

#include<mutex>

// Update to module, someday

namespace so {

	std::vector<std::string> get_words(char in_buff[DEFAULT_BUFLEN], int size);
	std::string login(std::vector<User>* user_vec, User data);
	std::string get_time();

	int decode_signal(char(&in_buff)[DEFAULT_BUFLEN], int in_size, char(&out_buff)[DEFAULT_BUFLEN], std::vector<User>* user_vec, std::mutex &user_m,
		std::map<std::string, int> * storage, std::mutex & storage_mutex) 
	{
		std::vector<std::string> decoded_vec = get_words(in_buff, in_size);
		std::string mess = "";

		// End trans
		if (decoded_vec[0] == "exit") {
			if (decoded_vec.size() >= 2) {
				int id = std::stoi(decoded_vec[1]);
				if (id != -1) {
					user_m.lock();
					user_vec->at(id).logged = false;
					user_m.unlock();
				}
			}
			return -1;
		}

		if (decoded_vec[0] == "get") {
			if (decoded_vec[1] == "time") {
				if (decoded_vec.size() < 3 || decoded_vec[2] != "/n")
					mess = "Server time: ";
				mess += get_time();
			}
			if (decoded_vec[1] == "storage") {
				if (decoded_vec[2] == "all") {
					return -10;
				}
			}
		}
		else if (decoded_vec[0] == "storage") {
			if (decoded_vec.size() < 4)
				mess = "Invalid number of arguments";
			else {
				storage_mutex.lock();
				if (decoded_vec[1] == "add") {
					std::string item = decoded_vec[2];
					int num = std::stoi(decoded_vec[3]);
					(*storage)[item] += num;
					mess = "Added " + item + " to storage";
				}
				else if (decoded_vec[1] == "remove") {
					std::string item = decoded_vec[2];
					int num = std::stoi(decoded_vec[3]);
					int storage_num = (*storage)[item];
					if (storage_num < num)
						mess = "Storage does not have enough " + item;
					else {
						(*storage)[item] -= num;
						mess = "Removed " + item + " from storage";
					}
				}
				storage_mutex.unlock();
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
				user_m.lock();
				mess = so::login(user_vec, user);
				user_m.unlock();
			}
		}
		else if (decoded_vec[0] == "logoff") {
			if (decoded_vec.size() < 2)
				mess = "Invalid number of arguments";
			else {
				int id = std::stoi(decoded_vec[1]);
				user_m.lock();
				for (int i = 0; i < user_vec->size(); i++) {
					if (user_vec->at(i).id == id) {
						(*user_vec)[i].logged = false;
						mess = "Logged off!";
					}
				}
				user_m.unlock();
			}
		}
		else if (decoded_vec[0] == "admin") {
			// Arguments check
			if (decoded_vec.size() < 3)
				mess = "Invalid number of arguments";
			else {
				int id = std::stoi(decoded_vec[1]);
				// Check if really admin
				bool f_chk = false;
				user_m.lock();
				for (int i = 0; i < user_vec->size(); i++) {
					if (user_vec->at(i).id == id && user_vec->at(i).type == USER_TYPE::Admin) {
						f_chk = true;
						break;
					}
				}
				user_m.unlock();
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
		bool quote_check = false;
		while (id < size) {
			if (in_buff[id] == '\"') {
				quote_check = !quote_check;
				id++;
				continue;
			}
			if ((in_buff[id] == ' ' && !quote_check) && buffer.size() != 0) {
				out.push_back(buffer);
				buffer.clear();
				id++;
				continue;
			}
			else {
				buffer.push_back(in_buff[id++]);
			}
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
		User active_user = {};
		// Maybe binsearch?
		for (int i = 0; i < user_vec->size(); i++) {
			if (data.name == (*user_vec)[i].name && data.type == (*user_vec)[i].type) {
				active_user = (*user_vec)[i];
			}
		}
		if (active_user.logged)
			return "f User already logged in";

		if (active_user.id != -1) { // Old user
			data.id = active_user.id;
			user_vec->at(active_user.id).logged = true;
		}
		else { // New user
			if (data.type == USER_TYPE::Admin)
				return "f Admin already exists";
			data.id = user_vec->size();
			data.logged = true;
			user_vec->push_back(data);
		}

		return "t " + std::to_string(data.id);
	}
}