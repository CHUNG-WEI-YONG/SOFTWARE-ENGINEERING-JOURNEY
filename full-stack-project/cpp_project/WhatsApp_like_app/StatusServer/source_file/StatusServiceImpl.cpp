#include "StatusServiceImpl.h"
#include "const.h"
#include "ConfigMgr.h"
#include <random>
#include <sstream>
#include <iomanip>
#include <string>
#include "RedisMjr.h"

std::string GenerateStringUUID() {
    static std::random_device rd;
    static std::mt19937 gen(rd());

    // 👈 修正为标准库的均匀整数分布
    std::uniform_int_distribution<> dis_hex(0, 15);
    std::uniform_int_distribution<> dis_v4(8, 11);

    std::stringstream ss;
    ss << std::hex << std::setfill('0');

    // 拼装标准 UUID 格式: 8-4-4-4-12
    for (int i = 0; i < 8; ++i) ss << std::setw(1) << dis_hex(gen); ss << "-";
    for (int i = 0; i < 4; ++i) ss << std::setw(1) << dis_hex(gen); ss << "-";

    ss << "4"; // UUID v4 规范固定位
    for (int i = 0; i < 3; ++i) ss << std::setw(1) << dis_hex(gen); ss << "-";

    ss << std::setw(1) << dis_v4(gen); // v4 规范的特定前缀
    for (int i = 0; i < 3; ++i) ss << std::setw(1) << dis_hex(gen); ss << "-";
    for (int i = 0; i < 12; ++i) ss << std::setw(1) << dis_hex(gen);

    return ss.str();
}

ChatServer StatusServiceImpl::getChatServer() {
    std::lock_guard<std::mutex> lock(_ChatServerLock);

    auto Minserver = _servers.begin()->second;
    std::string count_str;
   bool found = RedisMjr::GetInstance()->HGet(LOGIN_COUNT, Minserver.name,count_str);
   if (!found) {
       Minserver.conn_count = INT_MAX;
   }
   else {
       Minserver.conn_count = std::stoi(count_str);
   }


    for (const auto& server : _servers) {
        if (server.second.name == Minserver.name) {
            continue;
        }
        std::string count_str;
        bool found = RedisMjr::GetInstance()->HGet(LOGIN_COUNT, Minserver.name, count_str);
        if (!found) {
            Minserver.conn_count = INT_MAX;
        }
        else {
            Minserver.conn_count = std::stoi(count_str);
        }
        if (server.second.conn_count < Minserver.conn_count) {
            Minserver = server.second;
        }
    }
    return Minserver;

}

 Status StatusServiceImpl::GetChatServer(ServerContext * context, const GetChatServerReq * request, GetChatServerRsp * reply)
{
    //std::lock_guard<std::mutex> lock(_ChatServerLock);
    std::string prefix("status server has received :  ");
    const auto& server = getChatServer();
    reply->set_host(server.host);
    reply->set_port(server.port);
    reply->set_error(ErrorCodes::Success);
    reply->set_token(GenerateStringUUID());
    InsertToken(request->uid(), reply->token());
    return Status::OK;
}

StatusServiceImpl::StatusServiceImpl() :_server_index(0)
{
    auto& cfg = ConfigMgr::Inst();
    auto server_list = cfg["ChatServers"]["Name"];
    std::stringstream ss(server_list);
    std::vector<std::string> words;
    std::string word;

    while (std::getline(ss, word, ',')) {
        words.push_back(word);
    }

    for (auto word : words) {
        if (cfg[word]["Name"].empty()) {
            continue;
        }
        ChatServer s;
        s.name = cfg[word]["Name"];
        s.host = cfg[word]["Host"];
        s.port = cfg[word]["Port"];
        _servers[s.name] = s;
    }
}

Status StatusServiceImpl::Login(ServerContext* context, const LoginReq* request, LoginRsp* reply){
    int uid=request->uid();
    std::string token=request->token();
    std::string uid_str = std::to_string(uid);
    std::string userSearch = USERTOKENPREFIX + uid_str;
    std::string token_val = "";
    bool success=RedisMjr::GetInstance()->Get(userSearch, token_val);
    if (!success) {
        reply->set_error(ErrorCodes::UidInvalid);
        return Status::OK;
    }
    if (token_val!=token) {
        reply->set_error(ErrorCodes::TokenInvalid);
        return Status::OK;
    }    
    reply->set_error(ErrorCodes::Success);
    reply->set_uid(uid);
    reply->set_token(token);
    return Status::OK;

}

void StatusServiceImpl::InsertToken(int uid, std::string token){
    std::string uid_str = std::to_string(uid);
    std::string userStr = USERTOKENPREFIX + uid_str;
    RedisMjr::GetInstance()->Set(userStr, token);

}