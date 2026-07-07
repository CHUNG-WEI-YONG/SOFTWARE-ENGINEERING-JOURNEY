#include "StatusServiceImpl.h"
#include "const.h"
#include "ConfigMgr.h"
#include <random>
#include <sstream>
#include <iomanip>
#include <string>

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
//
//Status StatusServiceImpl::GetChatServer(ServerContext* context, const GetChatServerReq* request, GetChatServerRsp* reply) {
//	std::string prefix("Status Server received: ");
//	const auto& server = getChatServer();
//	reply->set_host(server.host);
//	reply->set_port(server.port);
//	reply->set_error(ErrorCodes::Success);
//	reply->set_token(GenerateStringUUID);
//	return Status::OK;
//
//}
//
//ChatServer StatusServiceImpl::getChatServer() {
//	std::lock_guard<std::mutex> lock(_servers_mutex);
//
//	auto Minserver = _servers.begin()->second;
//	for (const auto& server : _servers) {
//		if (server.second.conn_count < Minserver.second.conn_count) {
//			Minserver = server.second;
//		}
//	}
//	return Minserver;
//}
//
//void StatusServiceImpl::InsertToken(int uid, std::string token) {
//	std::lock_guard<std::mutex> lock(_tokens_mutex);
//	_tokens[uid] = token;
//}
//
//StatusServiceImpl::StatusServiceImpl() :_server_index(0)
//{
//	auto& cfg = ConfigMgr::Inst();
//	ChatServer server;
//	server.port = cfg["ChatServer1"]["Port"];
//	server.host = cfg["ChatServer1"]["Host"];
//	server.conn_count = 0;
//	server.name = cfg["ChatServer1"]["Host"];
//	_servers[server.name] = server;
//	server.port = cfg["ChatServer2"]["Port"];
//	server.host = cfg["ChatServer2"]["Host"];
//	server.name = cfg["ChatServer1"]["Host"];
//	_servers[server.name] = server;
//
//}

Status StatusServiceImpl::GetChatServer(ServerContext* context, const GetChatServerReq* request, GetChatServerRsp* reply)
{
    //std::lock_guard<std::mutex> lock(_ChatServerLock);
    std::string prefix("status server has received :  ");
    _server_index = (_server_index+1) % (_servers.size());
    auto& server = _servers[_server_index];
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
    ChatServer server;
    server.port = cfg["ChatServer1"]["Port"];
    server.host = cfg["ChatServer1"]["Host"];
    _servers.push_back(server);
    std::cout << "🚀 StatusServer Loaded ChatServer1: " << server.host << ":" << server.port << std::endl;
    server.port = cfg["ChatServer2"]["Port"];
    server.host = cfg["ChatServer2"]["Host"];
    _servers.push_back(server);
    std::cout << "🚀 StatusServer Loaded ChatServer2: " << server.host << ":" << server.port << std::endl;
}

Status StatusServiceImpl::Login(ServerContext* context, const LoginReq* request, LoginRsp* reply){
    int uid=request->uid();
    std::string token=request->token();
    std::lock_guard<std::mutex> lock(_tokens_mutex);
    auto iter=_tokens.find(uid);
    if(iter==_tokens.end()){
        reply->set_error(ErrorCodes::UidInvalid);
        return Status::OK;
    }
    if(iter->second!=token){
        reply->set_error(ErrorCodes::TokenInvalid);
        return Status::OK;
    }
    reply->set_error(ErrorCodes::Success);
    reply->set_uid(uid);
    reply->set_token(token);
    return Status::OK;

}

void StatusServiceImpl::InsertToken(int uid, std::string token){
    std::lock_guard<std::mutex> lock(_tokens_mutex);
    _tokens[uid]=token;

}