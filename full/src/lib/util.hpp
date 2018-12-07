#pragma once

#include <vector>
#include <string>

#include "json.hpp"

#define URI_START std::string("helloserver")
#define URI_RESULTS std::string("results")
#define IMMEDIATE_NUM 5
#define DAEMON_PERIOD 120

using json = nlohmann::json;

std::vector<std::string> parse_URI(std::string uri);
json create_siren(json &src, bool need_action, bool show_results);
json parse_siren(json& src);