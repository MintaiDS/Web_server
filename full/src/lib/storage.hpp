#pragma once

#include <string>
#include <map>
#include <algorithm>
#include <mutex>
#include "manager.hpp"

class LocalStorage
{
	private:
		std::map<std::pair<std::string, std::string>, unsigned long long> stor;
		std::mutex mtx;
	public:
		void add_new_data(std::string oid, std::string ans_oid, unsigned long long num);
		bool update_database(const DBManager& manager);
};