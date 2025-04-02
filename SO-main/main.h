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
	std::string get_time();

	void test() {
		std::cout << "Shalom2";
	}

	int decode_signal(char(&in_buff)[DEFAULT_BUFLEN], int in_size, char(&out_buff)[DEFAULT_BUFLEN]) {
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
}