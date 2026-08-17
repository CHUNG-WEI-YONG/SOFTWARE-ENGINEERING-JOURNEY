#pragma once
#include "Singleton.h"
#include <queue>
#include <thread>
#include "CSession.h"
#include <queue>
#include <map>
#include <functional>
#include "const.h"
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include <unordered_map>
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "message.pb.h"
#include "UserMgr.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using message::AddFriendReq;
using message::AddFriendRsp;

using message::AuthFriendReq;
using message::AuthFriendRsp;

using message::ChatService;
using message::TextChatMsgReq;
using message::TextChatMsgRsp;
using message::TextChatData;
using message::KickUserReq;
using message::KickUserRsp;



class Cserver;
typedef  function<void(shared_ptr<CSession>, const short& msg_id, const string& msg_data)> FunCallBack;
class LogicSystem :public Singleton<LogicSystem>
{
	friend class Singleton<LogicSystem>;
public:
	~LogicSystem();
	void PostMsgToQue(shared_ptr < LogicNode> msg);
	void SetServer(std::shared_ptr<Cserver> pserver);
	bool GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& user_info);
private:
	LogicSystem();
	void DealMsg();
	void RegisterCallBacks();
	void LoginHandler(shared_ptr<CSession> session, const short& msg_id, const string& msg_data);
	void SearchUser(shared_ptr<CSession> session, const short& msg_id, const string& msg_data);
	void AddFriendApply(shared_ptr<CSession> session, const short& msg_id, const string& msg_data);
	void AuthFriendApply(shared_ptr<CSession> session, const short &msg_id, const string& msg_data);
	void SearchUserByUid(const std::string& uid, Json::Value& rt);
	void SearchUserByName(const std::string& uid, Json::Value& rt);
	bool isPureDigit(const std::string& word);
	bool GetFriendApply(int to_uid, std::vector<std::shared_ptr<ApplyInfo>>& list);

	std::thread _worker_thread;
	std::queue<shared_ptr<LogicNode>> _msg_que;
	std::mutex _mutex;
	std::condition_variable _consume;
	bool _b_stop;
	std::map<short, FunCallBack> _fun_callbacks;
	std::shared_ptr<Cserver> _p_server;
};