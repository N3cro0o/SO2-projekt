#include<iostream>
#include<string>

namespace so {
	enum USER_TYPE {
		Admin = 2,
		Quartermaster = 1,
		Delivery = 0
	};

	struct User {
		// Add serialization to file, sometime in the future kek
		int id = 0;
		std::string name = "User-client";
		USER_TYPE type = USER_TYPE::Delivery;
	};

	void print_user(User* data) {
		std::cout << "ID " << data->id << " Username " << data->name << " Type " << data->type << std::endl;
	}
}