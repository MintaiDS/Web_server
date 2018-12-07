#include "manager.hpp"
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <mongocxx/options/update.hpp>
#include <mongocxx/options/find.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/pool.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/collection.hpp>
#include <mongocxx/result/update.hpp>
#include <bsoncxx/oid.hpp>
#include <mongocxx/model/update_one.hpp>

#include <iostream>

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

bool DBManager::get_poll_by_id(std::string oid, json *j, bool need_results) const
{
    try {
        auto client = pool->acquire();
        auto collection = (*client)[DB_NAME][COLLECTION_NAME];
        mongocxx::options::find find_options;
        if (!need_results)
            find_options.projection(bsoncxx::builder::stream::document{}
                << "answers.num" << 0 << bsoncxx::builder::stream::finalize);
        bsoncxx::stdx::optional<bsoncxx::document::value> maybe_result = 
            collection.find_one(bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(oid) << bsoncxx::builder::stream::finalize, find_options);
        if (maybe_result)
        {
            *j = json::parse(bsoncxx::to_json(*maybe_result));
            return true;
        }
        return false;
    } catch (std::exception e)
    {
        return false;
    }
}

bool DBManager::vote_poll_single(const voteInfo &vi) const
{
    try {
        auto client = pool->acquire();
        auto collection = (*client)[DB_NAME][COLLECTION_NAME];
        bsoncxx::stdx::optional<mongocxx::result::update> maybe_result = 
            collection.update_one(bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(vi.oid) << bsoncxx::builder::stream::finalize,
                bsoncxx::builder::stream::document{} << "$inc" << bsoncxx::builder::stream::open_document
                << "answers.$[elem].num" << (int64_t)(vi.num) << bsoncxx::builder::stream::close_document << bsoncxx::builder::stream::finalize,
                (new mongocxx::options::update())->array_filters(bsoncxx::builder::stream::array{}
                    << bsoncxx::builder::stream::open_document 
                    << "elem.id" << bsoncxx::oid(vi.oid_ans) 
                    << bsoncxx::builder::stream::close_document
                    << bsoncxx::builder::stream::finalize));
        if (maybe_result)
        {
            return true;
        }
        return false;
    } catch (std::exception e)
    {
        return false;
    }
}

bool DBManager::vote_poll_bulk(const std::vector<voteInfo> &vis) const
{
    if (vis.empty())
        return true;
    try {
        auto client = pool->acquire();
        auto collection = (*client)[DB_NAME][COLLECTION_NAME];

        auto bulk = collection.create_bulk_write();
        for (auto&& vi : vis)
        {
            auto doc = make_document(kvp("_id", bsoncxx::oid(vi.oid)));
            auto upd = make_document(kvp("$inc", make_document(kvp("answers.$[elem].num", (int64_t)(vi.num)))));
            mongocxx::model::update_one op{doc.view(), upd.view()};
            op.array_filters(bsoncxx::builder::stream::array{}
                    << bsoncxx::builder::stream::open_document 
                    << "elem.id" << bsoncxx::oid(vi.oid_ans) 
                    << bsoncxx::builder::stream::close_document
                    << bsoncxx::builder::stream::finalize);
            bulk.append(op);
        }
        auto maybe_result = bulk.execute();
        if (maybe_result)
        {
            return true;
        }
        return false;
    } catch (std::exception e)
    {
        return false;
    }
}