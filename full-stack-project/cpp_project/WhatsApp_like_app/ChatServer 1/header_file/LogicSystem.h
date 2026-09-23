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
class LogicSystem;
typedef  function<void(shared_ptr<CSession>, const short& msg_id, const string& msg_data)> FunCallBack;
class LogicWorker
{
	friend class LogicSystem;
public:
	void Start();
	void End();
	~LogicWorker();
	void PostMsgToQue(shared_ptr < LogicNode> msg);
	void SetServer(std::shared_ptr<Cserver> pserver);
	bool GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& user_info);
	LogicWorker();
private:
	void DealMsg();
	void RegisterCallBacks();
	void LoginHandler(shared_ptr<CSession> session, const short& msg_id, const string& msg_data);
	void SearchUser(shared_ptr<CSession> session, const short& msg_id, const string& msg_data);
	void AddFriendApply(shared_ptr<CSession> session, const short& msg_id, const string& msg_data);
	void AuthFriendApply(shared_ptr<CSession> session, const short &msg_id, const string& msg_data);
	void DealChatTextMsg(shared_ptr<CSession> session, const short& msg_id, const string& msg_data);
	void SearchUserByUid(const std::string& uid, Json::Value& rt);
	void SearchUserByName(const std::string& uid, Json::Value& rt);
	bool isPureDigit(const std::string& word);
	bool GetFriendApply(int to_uid, std::vector<std::shared_ptr<ApplyInfo>>& list);
	bool GetFriendList(int uid, std::vector<std::shared_ptr<UserInfo>>& friend_list);
	void UploadFile(shared_ptr<CSession> session, const short& msg_id, const string& msg_data);
	void DealChatFileMsg(shared_ptr<CSession> session, const short& msg_id, const string& msg_data);
	void DownloadFile(shared_ptr<CSession> session, const short& msg_id, const string& msg_data);
	void HeartBeatHandle(shared_ptr<CSession> session, const short& msg_id, const string& msg_data);
	std::thread _worker_thread;
	std::queue<shared_ptr<LogicNode>> _msg_que;
	std::mutex _mutex;
	std::condition_variable _consume;
	bool _b_stop;
	std::map<short, FunCallBack> _fun_callbacks;
	std::shared_ptr<Cserver> _p_server;
};


class LogicSystem :public Singleton<LogicSystem> {
	friend class Singleton<LogicSystem>;
public:
	void PostMsgtoQue(shared_ptr<LogicNode> msg);
	void SetServer(std::shared_ptr<Cserver> pserver);
	bool GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& user_info);
	~LogicSystem();

private:
	LogicSystem();
	std::vector<std::shared_ptr<LogicWorker>> _pool;
	std::size_t sz;
	std::shared_ptr<Cserver> _p_server;

};