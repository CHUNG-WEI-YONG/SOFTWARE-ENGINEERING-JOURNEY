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
		std::string port = section["Port"];

		if (name.empty()) {
			continue;
		}

		// ✅ 避免连续临时对象的 [] 链式调用，用局部变量接住更安全且易调试
		_pools[name] = std::make_unique<ChatConPool>(5, host, port);
	}

}

AddFriendRsp ChatGrpcClient::NotifyAddFriend(std::string server_ip, const AddFriendReq& req) {
	AddFriendRsp rsp;
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