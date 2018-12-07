#include "storage.hpp"
#include <iostream>

void LocalStorage::add_new_data(std::string oid, std::string ans_oid, unsigned long long num)
{
	std::pair<std::string, std::string> oids(oid, ans_oid);
	std::lock_guard<std::mutex> lck(mtx);
	if (stor.find(oids) != stor.end())
		stor[oids] += num;
	else
		stor[oids] = num;
}
bool LocalStorage::update_database(const DBManager& manager)
{
	if (stor.empty())
		return true;
	std::map<std::pair<std::string, std::string>, unsigned long long> buf;
	{
		std::lock_guard<std::mutex> lck(mtx);
		buf = std::move(stor);
		stor.clear();
	}
	std::vector<voteInfo> vis;
	for (auto&& el : buf)
		vis.push_back(voteInfo{el.second, el.first.first, el.first.second});
	return manager.vote_poll_bulk(vis);
}