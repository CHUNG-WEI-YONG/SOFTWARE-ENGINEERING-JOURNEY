#include "ChatGrpcClient.h"
#include "ConfigMgr.h"

ChatGrpcClient::ChatGrpcClient() {
	auto& config = ConfigMgr::Inst();
	auto serverlist = config["PeerServer"]["Servers"];

	std::vector<std::string>words;
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

		// ✅ 避免连续临时对象的 [] 链式调用，用局部变量接住更安全且易调试
		_pools[name] = std::make_unique<ChatConPool>(5, host, port);
	}

}

AddFriendRsp ChatGrpcClient::NotifyAddFriend(std::string server_ip, const AddFriendReq& req) {
    AddFriendRsp rsp;
    // Set default response payload upfront
    rsp.set_applyuid(req.applyuid());
    rsp.set_touid(req.touid());

    // 1. Find the target pool (use lock if _pools can be modified at runtime)
    auto iter = _pools.find(server_ip);
    if (iter == _pools.end()) {
        rsp.set_error(ErrorCodes::RPCFailed);
        return rsp;
    }

    auto& pool = iter->second;
    auto conn = pool->GetConn();
    if (!conn) {
        rsp.set_error(ErrorCodes::RPCFailed);
        return rsp;
    }

    // 2. Return connection to pool on function exit (Valid use of Defer)
    Defer rtConn([&conn, &pool]() {
        pool->returnConn(std::move(conn));
        });

    // 3. Execute synchronous RPC call
    ClientContext context;
    Status status = conn->NotifyAddFriend(&context, req, &rsp);

    if (!status.ok()) {
        rsp.set_error(ErrorCodes::RPCFailed);
        return rsp;
    }

    // 4. Mark success only after status is confirmed OK
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