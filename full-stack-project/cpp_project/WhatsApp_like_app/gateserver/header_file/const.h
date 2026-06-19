#pragma once
#include <iostream>
#include <boost/beast/http.hpp>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <memory>
#include "Singleton.h"
#include <functional>
#include <map>
#include <unordered_map>
#include <json.hpp>
#include <fstream>
#include <filesystem>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>




namespace beast = boost::beast;     
namespace http  = beast::http;
namespace net   = boost::asio;
using tcp   = boost::asio::ip::tcp;
using json = nlohmann::json;

enum ErrorCodes{
    Success=0,
    Error_Json=1001,
    RPC_Failed=1002,
};

class ConfigMgr;
extern ConfigMgr gCfg;
