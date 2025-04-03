#include<iostream>
#include<string>

namespace so {
	namespace menu {
		std::string user_input() {
			std::string in;
			std::cout << "\nInput message below.\nMessage: ";
			while (in.size() == 0) {
				std::getline(std::cin, in);
			}
			return in;
		}

		std::string login(User* data);

		std::string menu_loop(User *data, int *word) {
			while (1) {
				int option;
				std::string x;
				std::cout << "Warehouse/Chat server. Select one of the options below:\n";
				std::cout << "1. Get server time\n2. Echo message\n3. User input\n";
				if (data->logged)
					std::cout << "\n10. Logoff\n";
				else
					std::cout << "\n10. Login\n";
				if (data->type == USER_TYPE::Quartermaster || data->type == USER_TYPE::Admin) {
					std::cout << "11. Read storage data\n12. Return item\n";
				}
				if (data->type == USER_TYPE::Admin) {
					std::cout << "\n100. Kill Server\n";
				}
				std::cout << "0. Terminate client\n";
				std::cin >> option;

				switch (option) {
				case 1:
					return "get time";
				case 2:
					x = user_input();
					return "echo " + x;
				case 3:
					return user_input();
				case 10:
					if (data->logged) {
						int id = data->id;
						*data = User();
						return "logoff " + std::to_string(id);
					}
					else {
						x = login(data);
						*word += 1;
						return "login " + x;
					}
				case 11:
					if (data->type == USER_TYPE::Quartermaster || data->type == USER_TYPE::Admin) {
						*word += 2;
						return "get storage all";
					}
				case 0:
					return "exit " + std::to_string(data->id);

				// Admin
				case 100:
					return "admin " + std::to_string(data->id) + " kill";
				}
			}
		}

		std::string login(User* data) {
			std::string in, out;
			std::cout << "\nInput Login and Type below.\nLogin: ";
			while (in.size() == 0) {
				std::getline(std::cin, in);
			}
			data->name = in;
			out = in + ' ';
			std::cin.clear();
			in.clear();
			std::cout << "Type (0 - Delivery, 1 - Quartermaster, 2 - Admin): ";
			while (in.size() == 0) {
				std::getline(std::cin, in);
			}
			data->type = static_cast<USER_TYPE>(std::stoi(in));
			return out + in;
		}
	}
}