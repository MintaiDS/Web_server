#pragma once

#include <mongocxx/pool.hpp>
#include <mongocxx/instance.hpp>

#include <string>

#include "util.hpp"

#define DB_NAME "polls"
#define COLLECTION_NAME "polls"

struct voteInfo {
	unsigned long long num;
	std::string oid;
	std::string oid_ans;
};

class DBManager
{
private:
	std::shared_ptr<mongocxx::instance> instance;
	std::shared_ptr<mongocxx::pool> pool;
public:
	DBManager() {}
	DBManager(std::shared_ptr<mongocxx::instance> instance, std::shared_ptr<mongocxx::pool> pool) : instance(instance), pool(pool) {}
	bool get_poll_by_id(std::string oid, json *j, bool need_results) const;
	bool vote_poll_single(const voteInfo &vi) const;
	bool vote_poll_bulk(const std::vector<voteInfo> &vis) const;
};