#include "ChatGrpcClient.h"
#include "ConfigMgr.h"

ChatGrpcClient::ChatGrpcClient() {
    auto& config = ConfigMgr::Inst();
    auto serverlist = config["PeerServer"]["Servers"];

    std::vector<std::string> words;
    std::string word;

    std::stringstream ss(serverlist);
    while (std::getline(ss, word, ',')) {
        words.push_back(word);
    }
    for (auto& w : words) {
        SectionInfo section = config[w];
        std::string name = section["Name"];
        std::string host = section["Host"];
        std::string port = section["RPCPort"];

        if (name.empty()) {
            continue;
        }

        std::cout << "Connect correctly to " << w << " host :" << host << " rpc port: " << port << "\n";

        // ✅ 同时存入 "Host:Port" 和 "Host"，提高查找容错率
        std::string ip_port = host + ":" + port;
        _ip_to_name[ip_port] = name;

        // 初始化连接池
        _pools[name] = std::make_unique<ChatConPool>(5, host, port);
    }
}

AddFriendRsp ChatGrpcClient::NotifyAddFriend(std::string server_name, const AddFriendReq& req) {
    AddFriendRsp rsp;
    rsp.set_applyuid(req.applyuid());
    rsp.set_touid(req.touid());
    std::cout << "Notify success run"<< std::endl;
    // 3. 获取对应的连接池
    auto iter = _pools.find(server_name);
    if (iter == _pools.end()) {
        std::cout << "Cannot find connection pool for server_name: " << server_name << std::endl;
        rsp.set_error(ErrorCodes::RPCFailed);
        return rsp;
    }

    auto& pool = iter->second;
    auto conn = pool->GetConn();
    if (!conn) {
        rsp.set_error(ErrorCodes::RPCFailed);
        return rsp;
    }

    // 4. 析构时归还连接
    Defer rtConn([&conn, &pool]() {
        pool->returnConn(std::move(conn));
        });

    // 5. 发送 RPC 请求
    ClientContext context;
    Status status = conn->NotifyAddFriend(&context, req, &rsp);

    if (!status.ok()) {
        rsp.set_error(ErrorCodes::RPCFailed);
        return rsp;
    }

    rsp.set_error(ErrorCodes::Success);
    return rsp;
}
AuthFriendRsp ChatGrpcClient::NotifyAuthFriend(std::string server_ip, const AuthFriendReq& req) {
	AuthFriendRsp rsp;
	return rsp;
}
bool ChatGrpcClient::GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo) {
	return true;
}
TextChatMsgRsp ChatGrpcClient::NotifyTextChatMsg(std::string server_ip, const TextChatMsgReq& req, const Json::Value& rtvalue) {
	TextChatMsgRsp rsp;
	return rsp;
}