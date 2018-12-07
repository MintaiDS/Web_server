#include "util.hpp"

#include <sstream>

std::vector<std::string> parse_URI(std::string uri)
{
    std::vector<std::string> parts;
    std::stringstream ss(uri);
    std::string s;
    while (std::getline(ss, s, '/'))
        parts.push_back(s);
    return parts;
}


json create_siren(json &src, bool need_action, bool show_results)
{
    json j;
    j["class"] = "poll";
    j["properties"] = { {"text", src["text"]}, {"answers", src["answers"]}};
    if (need_action)
    {

    }
    j["links"] = json::array();
    if (!show_results)
    {
        j["links"].push_back({{"rel", json::array({"self"})}, {"href", "/" + URI_START + "/" + std::string(src["_id"]["$oid"])}});
        j["links"].push_back({{"rel", json::array({"related"})}, {"href", "/" + URI_START + "/" + std::string(src["_id"]["$oid"]) + "/" + URI_RESULTS}});
    }
    else
    {
        j["links"].push_back({{"rel", json::array({"related"})}, {"href", "/" + URI_START + "/" + std::string(src["_id"]["$oid"])}});
        j["links"].push_back({{"rel", json::array({"self"})}, {"href", "/" + URI_START + "/" + std::string(src["_id"]["$oid"]) + "/" + URI_RESULTS}});
    }
    return j;
}
json parse_siren(json& src)
{
    json j;
    j["text"] = src["properties"]["text"];
    j["answers"] = src["properties"]["answers"];
    return j;
}