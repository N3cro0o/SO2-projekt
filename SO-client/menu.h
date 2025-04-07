#include<iostream>
#include<string>

#define MEMO_VEC std::vector<std::pair<std::string, int>>

namespace so {
	namespace menu {
		std::string user_input(std::string text_to_show = "\nInput message below.\nMessage:") {
			std::string in;
			std::cout << text_to_show;
			while (in.size() == 0) {
				std::getline(std::cin, in);
			}
			return in;
		}

		std::string login(User* data);
		void print_memo_vec(MEMO_VEC* memo_vec);
		bool check_memo_vec(MEMO_VEC* memo_vec, std::pair<std::string, int>& pair_to_find);
		void remove_from_memo_vec(MEMO_VEC* memo_vec, std::pair<std::string, int>& pair_to_find);

		std::string menu_loop(User* data, int* word, MEMO_VEC* memo_vec) {
			while (1) {
				int option;
				std::string x;
				std::cout << "\n------------------------------------------------------\n";
				std::cout << "Warehouse communication server. Select one of the options below:\n";
				std::cout << "1. Get server time\n2. Echo message\n3. User input\n";
				if (data->logged) {
					std::cout << "\n10. Logoff\n11. Read storage data\n";
				}
				else
					std::cout << "\n10. Login\n";
				if (data->type == USER_TYPE::Quartermaster || data->type == USER_TYPE::Admin) {
					std::cout << "12. Remove item\n13. Undo remove\n";
				}
				if ((data->type == USER_TYPE::Delivery && data->logged) || data->type == USER_TYPE::Admin) {
					std::cout << "14. Add new items to storage\n";
				}

				if (data->type == USER_TYPE::Admin) {
					std::cout << "\n100. Kill Server\n";
				}
				std::cout << "\n0. Terminate client\n";
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
						memo_vec->clear();
						return "logoff " + std::to_string(id);
					}
					else {
						x = login(data);
						*word += 1;
						return "login " + x;
					}
					break;
				case 11:
					if (data->logged) {
						*word += 2;
						return "get storage all";
					}
					break;
				case 12:
					if (data->type == USER_TYPE::Quartermaster || data->type == USER_TYPE::Admin) {
						x = user_input("\nInput item below\nItem's name: ");
						int count;
						std::cout << "Now their number: \n";
						std::cin >> count;
						std::pair<std::string, int> input = { x, count };
						memo_vec->push_back(input);
						return "storage remove \"" + x + "\" " + std::to_string(count);
					}
					break;
				case 13:
					if (data->type == USER_TYPE::Quartermaster || data->type == USER_TYPE::Admin) {
						// Check vec
						if (memo_vec->size() == 0)
							break;
						// Print memo
						std::cout << "Possible actions:\n";
						print_memo_vec(memo_vec);
						std::cout << std::endl;
						// Get user input
						x = user_input("\nInput item below\nItem's name: ");
						int count;
						std::cout << "Now their number: \n";
						std::cin >> count;
						// Check input
						std::pair<std::string, int> input = { x, count };
						if (check_memo_vec(memo_vec, input)) {
							remove_from_memo_vec(memo_vec, input);
							return "storage add \"" + input.first + "\" " + std::to_string(input.second);
						}
						std::cout << "Wrong input\n";
					}
					break;
				case 14:
					if (data->type == USER_TYPE::Admin || data->type == USER_TYPE::Delivery) {
						x = user_input("\nInput item below\nItem's name: ");
						int count;
						std::cout << "Now their number: \n";
						std::cin >> count;
						return "storage add \"" + x + "\" " + std::to_string(count);
					}
					break;
				case 0:
					return "exit " + std::to_string(data->id);

					// Admin
					// Server side verification
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

		void print_memo_vec(MEMO_VEC* memo_vec) {
			for (auto& pair : *memo_vec) {
				std::cout << pair.first << ", " << pair.second << std::endl;
			}
		}

		bool check_memo_vec(MEMO_VEC* memo_vec, std::pair<std::string, int>& pair_to_find) {
			for (auto& pair : *memo_vec) {
				if (pair.first == pair_to_find.first && pair.second == pair_to_find.second)
					return true;
			}
			return false;
		}

		void remove_from_memo_vec(MEMO_VEC* memo_vec, std::pair<std::string, int>& pair_to_find) {
			int i = 0;
			for (auto& pair : *memo_vec) {
				if (pair.first == pair_to_find.first && pair.second == pair_to_find.second) {
					std::swap((*memo_vec)[i], (*memo_vec)[memo_vec->size() - 1]);
					memo_vec->pop_back();
					break;
				}
				i++;
			}
		}
	}
}