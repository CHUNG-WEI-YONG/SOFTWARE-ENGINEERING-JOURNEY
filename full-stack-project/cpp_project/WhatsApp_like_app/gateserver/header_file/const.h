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
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <sw/redis++/redis++.h>
#include <cassert>
#include <mysqlx/xdevapi.h>
#define CODEPREFIX "code_"




namespace beast = boost::beast;     
namespace http  = beast::http;
namespace net   = boost::asio;
using tcp   = boost::asio::ip::tcp;
using json = nlohmann::json;

enum ErrorCodes {
    Success = 0,
    Error_Json = 1001,      // Json解析错误
    RPC_Failed = 1002,       // RPC请求错误
    VarifyExpired = 1003,   // 验证码过期
    VarifyCodeErr = 1004,   // 验证码错误
    UserExist = 1005,       // 用户已经存在
    PasswdErr = 1006,       // 密码错误
    EmailNotMatch = 1007,   // 邮箱不匹配
    PasswdUpFailed = 1008,  // 更新密码失败
    PasswdInvalid = 1009,   // 密码更新失败 (注: 图片中注释为密码更新失败/无效)
};

class ConfigMgr;
extern ConfigMgr gCfg;

class Defer {
public:
    Defer(std::function<void()> func) :_function(func) {};
    ~Defer() {
        _function();
    };

private:
    std::function<void()> _function;
};
