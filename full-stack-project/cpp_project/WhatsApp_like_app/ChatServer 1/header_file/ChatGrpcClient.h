#include "const.h"
#include "Singleton.h"
#include "ConfigMgr.h"
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "message.pb.h"
#include <queue>
#include <json/json.h>
#include <json/reader.h>
#include <condition_variable>
#include <unordered_map>

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::AddFriendReq;
using message::AddFriendRsp;

using message::AuthFriendReq;
using message::AuthFriendRsp;

using message::GetChatServerRsp;
using message::LoginRsp;
using message::LoginReq;
using message::ChatService;

using message::TextChatMsgReq;
using message::TextChatMsgRsp;
using message::TextChatData;


class ChatConPool {
public:
	ChatConPool(std::size_t size, std::string host, std::string port):_host(host),_port(port),_size(size) {
		for (std::size_t i = 0;i < size;i++) {
			std::shared_ptr<Channel> channel = grpc::CreateChannel(host + ":" + port,
				grpc::InsecureChannelCredentials());

			_conns.push(ChatService::NewStub(channel));
		}
	};

	std::unique_ptr<ChatService::Stub> GetConn() {
		std::unique_lock<std::mutex> lock(_mutex);
		_cv.wait(lock, [this]() {
			if (_b_stop) {
				return true;
			}
			return !_conns.empty();
		});
		if (_b_stop) {
			return nullptr;
		}
		auto context = std::move(_conns.front());
		_conns.pop();
		return context;
	}

	void returnConn(std::unique_ptr<ChatService::Stub> con) {
		std::lock_guard<std::mutex> lock(_mutex);
		if (_b_stop) {
			return;
		}
		_conns.push(std::move(con));
		_cv.notify_one();
	}

	void Close() {
		_b_stop = true;
		_cv.notify_all();
	}

	~ChatConPool() {
		std::lock_guard<std::mutex> lock(_mutex);
		Close();
		while (!_conns.empty()) {
			_conns.pop();
		}
	}

private:
	atomic<bool> _b_stop;
	std::mutex _mutex;
	std::condition_variable _cv;
	std::queue<std::unique_ptr<ChatService::Stub>> _conns;
	std::size_t _size;
	std::string _host;
	std::string _port;
};


class ChatGrpcClient :public Singleton<ChatGrpcClient> {
	friend class Singleton<ChatGrpcClient>;
private:
	ChatGrpcClient();
	std::unordered_map <std::string, std::unique_ptr<ChatConPool>> _pools;
	std::unordered_map<std::string, std::string> _ip_to_name;

public:
	AddFriendRsp NotifyAddFriend(std::string server_ip, const AddFriendReq& req);
	AuthFriendRsp NotifyAuthFriend(std::string server_ip, const AuthFriendReq& req);
	bool GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo);
	TextChatMsgRsp NotifyTextChatMsg(std::string server_ip, const TextChatMsgReq& req, const Json::Value& rtvalue);
	//KickUserRsp NotifyKickUser(std::string server_ip, const KickUserReq& req);
};