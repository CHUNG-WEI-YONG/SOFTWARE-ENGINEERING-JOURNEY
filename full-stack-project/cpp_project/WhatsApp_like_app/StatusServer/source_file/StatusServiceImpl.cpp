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
    std::string prefix("status server has received :  ");
    _server_index = (_server_index++) % (_servers.size());
    auto& server = _servers[_server_index];
    reply->set_host(server.host);
    reply->set_port(server.port);
    reply->set_error(ErrorCodes::Success);
    reply->set_token(GenerateStringUUID());
    return Status::OK;
}
StatusServiceImpl::StatusServiceImpl() :_server_index(0)
{
    auto& cfg = ConfigMgr::Inst();
    ChatServer server;
    server.port = cfg["ChatServer1"]["Port"];
    server.host = cfg["ChatServer1"]["Host"];
    _servers.push_back(server);
    server.port = cfg["ChatServer2"]["Port"];
    server.host = cfg["ChatServer2"]["Host"];
    _servers.push_back(server);
}