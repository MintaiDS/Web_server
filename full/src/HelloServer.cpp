#include <fastcgi2/component.h>
#include <fastcgi2/component_factory.h>
#include <fastcgi2/handler.h>
#include <fastcgi2/request.h>
#include <fastcgi2/logger.h>
#include <fastcgi2/config.h>

#include <cstdint>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <string>
#include <sstream>

#include "lib/storage.hpp"
#include "lib/manager.hpp"
#include "lib/util.hpp"

LocalStorage loc_stor;
DBManager manager;

void processStorage()
{
    std::cout<<"I'm working!\n";
    while (true)
    {
        clock_t beg = clock();
        if (!loc_stor.update_database(manager))
        {
            std::cout<<"Bulk update failed!\n";
        }
        clock_t end = clock();
        double spent = (double)(end - beg) / CLOCKS_PER_SEC;
        if (spent >= DAEMON_PERIOD)
        {
            std::cout<<"Too long database update: "<<spent - DAEMON_PERIOD<<" extra seconds\n";
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::seconds((uint32_t)(DAEMON_PERIOD - spent)));
        }
    }
}





class HelloServer : virtual public fastcgi::Component, virtual public fastcgi::Handler
{
    public:
        HelloServer(fastcgi::ComponentContext *context) :
                fastcgi::Component(context)
        {

        }

        virtual void onLoad()
        {
            uri = mongocxx::uri("mongodb://localhost:27017/?minPoolSize=128&maxPoolSize=1024");
            std::shared_ptr<mongocxx::instance> instance = std::shared_ptr<mongocxx::instance>(new mongocxx::instance{});
            std::shared_ptr<mongocxx::pool> pool = std::shared_ptr<mongocxx::pool>(new mongocxx::pool{uri});
            manager = DBManager(instance, pool);
            thrd = std::unique_ptr<std::thread>(new std::thread(&processStorage));
        }

        virtual void onUnload()
        {

        }

        virtual void handleRequest(fastcgi::Request *request, fastcgi::HandlerContext *context)
        {
            std::vector<std::string> parsed_uri = parse_URI(request->getScriptName());
            size_t sz = parsed_uri.size();
            if (sz == 3)
            {
                if (request->getRequestMethod() == "GET")
                {
                    json j;
                    if (manager.get_poll_by_id(parsed_uri[2], &j, false))
                    {
                        json j_siren = create_siren(j, false, false);
                        std::stringbuf buffer(j_siren.dump(4));
                        request->setContentType("application/vnd.siren+json");
                        request->write(&buffer);
                    }
                    else
                    {
                        request->setStatus(404);
                        std::stringbuf buffer("No poll with this ID was found!");
                        request->setContentType("plain/text");
                        request->write(&buffer);
                    }
                }
                else
                    request->setStatus(404);
            }
            else
            {
                if (sz == 4)
                {
                    if (parsed_uri[3] == URI_RESULTS)
                    {
                        if (request->getRequestMethod() == "GET")
                        {
                            json j;
                            if (manager.get_poll_by_id(parsed_uri[2], &j, true))
                            {
                                json j_siren = create_siren(j, false, true);
                                std::stringbuf buffer(j_siren.dump(4));
                                request->setContentType("application/vnd.siren+json");
                                request->write(&buffer);
                            }
                            else
                            {
                                request->setStatus(404);
                                std::stringbuf buffer("No poll with this ID was found!");
                                request->setContentType("plain/text");
                                request->write(&buffer);
                            }
                        }
                        else
                        {
                            if (request->getRequestMethod() == "POST")
                            {
                                json j;
                                try {
                                    std::string s;
                                    request->requestBody().toString(s);
                                    json req_j = json::parse(s);
                                    json parsed_req_j = parse_siren(req_j);
                                    if (manager.get_poll_by_id(parsed_uri[2], &j, true))
                                    {
                                        
                                        size_t sz = j["answers"].size();
                                        bool found = false;
                                        for (size_t i = 0; i < sz; ++i)
                                        {
                                            if (j["answers"][i]["id"]["$oid"].get<std::string>() == parsed_req_j["answers"][0]["id"]["$oid"].get<std::string>())
                                            {
                                                if (j["answers"][i]["num"].get<unsigned long long>() < IMMEDIATE_NUM)
                                                {
                                                    if (manager.vote_poll_single(voteInfo{1, j["_id"]["$oid"].get<std::string>(), j["answers"][i]["id"]["$oid"].get<std::string>()}))
                                                    {
                                                        j["answers"][i]["num"] = j["answers"][i]["num"].get<unsigned long long>() + 1;
                                                        json j_siren = create_siren(j, false, true);
                                                        std::stringbuf buffer(j_siren.dump(4));
                                                        request->setContentType("application/vnd.siren+json");
                                                        request->write(&buffer);
                                                    }
                                                    else
                                                        request->setStatus(500);
                                                }
                                                else
                                                {
                                                    loc_stor.add_new_data(j["_id"]["$oid"].get<std::string>(), j["answers"][i]["id"]["$oid"].get<std::string>(), 1);
                                                    json j_siren = create_siren(j, false, true);
                                                    std::stringbuf buffer(j_siren.dump(4));
                                                    request->setContentType("application/vnd.siren+json");
                                                    request->write(&buffer);
                                                }
                                                found = true;
                                                break;
                                            }
                                        }
                                        if (!found)
                                        {
                                            request->setStatus(400);
                                            std::stringbuf buffer("No answer with this ID was found!");
                                            request->setContentType("plain/text");
                                            request->write(&buffer);
                                        }
                                    }
                                    else
                                    {
                                        request->setStatus(404);
                                        std::stringbuf buffer("No poll with this ID was found!");
                                        request->setContentType("plain/text");
                                        request->write(&buffer);
                                    }
                                } catch (std::exception e)
                                {
                                    request->setStatus(400);
                                }
                                
                            }
                            else
                                request->setStatus(404);
                        }
                    }
                    else
                      request->setStatus(404);  
                }
                else
                    request->setStatus(404);
            }
        }
    private:
        mongocxx::uri uri;
        std::unique_ptr<std::thread> thrd;
};

FCGIDAEMON_REGISTER_FACTORIES_BEGIN()
FCGIDAEMON_ADD_DEFAULT_FACTORY("HelloServerFactory", HelloServer)
FCGIDAEMON_REGISTER_FACTORIES_END()