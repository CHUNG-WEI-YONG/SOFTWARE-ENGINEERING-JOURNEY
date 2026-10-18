#include "const.h"

#include "LogicSystem.h"
#include "Cserver.h"
#include "MsgNode.h"
#include "StatusGrpcClient.h"
#include "MysqlMgr.h"
#include "RedisMjr.h"
#include "ConfigMgr.h"
#include "UserMgr.h"
#include "ChatGrpcClient.h"
#include "FileGrpcClient.h"

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


inline int64_t getCurrentTimestamp() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()
	).count();
}

// 若客户端需要可读时间文本 (如 "2026-10-01 20:58:00")
inline std::string getCurrentTimeString() {
	auto now = std::chrono::system_clock::now();
	std::time_t in_time_t = std::chrono::system_clock::to_time_t(now);
	std::stringstream ss;
	ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
	return ss.str();
}


LogicWorker::LogicWorker():_b_stop(false),_p_server(nullptr) {
	RegisterCallBacks();
	Start();
}

void LogicWorker::Start()
{
	if (!_worker_thread.joinable()) {
		_worker_thread = std::thread(&LogicWorker::DealMsg, this);
	}
}

void LogicWorker::End()
{
	if (_b_stop==true) {
		return;
	}
	_b_stop = true;

	// 1. 唤醒可能正在等待条件变量的工作线程
	_consume.notify_all();

	// 2. 等待工作线程安全退出
	if (_worker_thread.joinable()) {
		_worker_thread.join();
	}
}

LogicWorker::~LogicWorker()
{
	End();
}

void LogicWorker::RegisterCallBacks() {
	_fun_callbacks[MSG_CHAT_LOGIN]= [this](shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
		this->LoginHandler(session, msg_id, msg_data);
		};

	_fun_callbacks[ID_SEARCH_USER_REQ] = [this](shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
		this->SearchUser(session, msg_id, msg_data);
		};
	_fun_callbacks[ID_ADD_FRIEND_REQ] = [this](shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
		this->AddFriendApply(session, msg_id, msg_data);
		};

	_fun_callbacks[ID_AUTH_FRIEND_REQ] = [this](shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
		this->AuthFriendApply(session, msg_id, msg_data);
		};

	_fun_callbacks[ID_TEXT_CHAT_MSG_REQ] = [this](shared_ptr<CSession > session, const short& msg_id, const string& msg_data) {
		this->DealChatTextMsg(session, msg_id, msg_data);
		};

	_fun_callbacks[ID_SNED_FILE_REQ] = [this](shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
		this->UploadFile(session, msg_id, msg_data);
		};

	_fun_callbacks[ID_FILE_CHAT_MSG] = [this](shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
		this->DealChatFileMsg(session, msg_id, msg_data);
		};

	_fun_callbacks[ID_DOWNLOAD_FILE_REQ]= [this](shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
		this->DownloadFile(session, msg_id, msg_data);
		};

	_fun_callbacks[ID_HEART_BEAT_REQ] = [this](shared_ptr<CSession>session, const short& msg_id, const string& msg_data) {
		this->HeartBeatHandle(session, msg_id, msg_data);
	};

	_fun_callbacks[ID_LOAD_CHAT_THREAD_REQ] = [this](shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
		this->GetUserThreadHandler(session, msg_id, msg_data);
		};

	_fun_callbacks[ID_CREATE_PRIVATE_CHAT_REQ] = [this](shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
		this->CreateNewPrivateThread(session, msg_id, msg_data);
	};

	_fun_callbacks[ID_LOAD_CHAT_MSG_REQ] = [this](shared_ptr<CSession>session, const short& msg_id, const string& msg_data) {
		this->LoadChatMsg(session, msg_id, msg_data);
	};
}

void LogicWorker::LoginHandler(shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
	Json::Reader reader;
	Json::Value root;
	if (!reader.parse(msg_data, root)) {
		return;
	}
	Json::Value rtvalue;

	auto uid = root["uid"].asInt();
	auto token = root["token"].asString();
	

	//auto rsp = StatusGrpcClient::GetInstance()->Login(uid, token);


	Defer defer([this,&rtvalue,session]() {
		std::string return_val = rtvalue.toStyledString();
		session->Send(return_val, MSG_CHAT_LOGIN_RSP);

		});



	std::string uid_str = std::to_string(uid);
	std::string token_key = USERTOKENPREFIX + uid_str;
	std::string token_value = "";
	bool rsuccess = RedisMjr::GetInstance()->Get(token_key, token_value);
	if (!rsuccess) {
		rtvalue["error"] = ErrorCodes::UidInvalid;
		return;
	}
	if (token_value != token) {
		rtvalue["error"] = ErrorCodes::TokenInvalid;
		return;
	}
	rtvalue["error"] = ErrorCodes::Success;

	std::string base_key = USER_BASE_INFO + uid_str;
	std::shared_ptr<UserInfo> userInfo = std::make_shared<UserInfo>();
	bool bsuccess = GetBaseInfo(base_key, uid, userInfo);
	if (!bsuccess) {
		rtvalue["error"] = ErrorCodes::UidInvalid;
		return;
	}
	rtvalue["uid"] = uid;
	//rtvalue["passwd"] = userInfo->passwd;
	rtvalue["email"] = userInfo->email;
	rtvalue["desc"] = userInfo->desc;
	rtvalue["nick"] = userInfo->nick;
	rtvalue["sex"] = userInfo->sex;
	rtvalue["icon"] = userInfo->icon;
	//rtvalue["token"] = rsp.token();
	rtvalue["name"] = userInfo->name;
	//rtvalue["token"] = rsp.token();
	//do for mysql to do the search for friend list and user list

	std::vector<std::shared_ptr<ApplyInfo>> list;
	bool gsuccess = GetFriendApply(uid, list);
	if (gsuccess) {
		for (auto& apply : list) {
			Json::Value obj;
			obj["name"] = apply->_name;
			obj["uid"] = apply->_uid;
			obj["desc"] = apply->_desc;
			obj["icon"] = apply->_icon;
			obj["nick"] = apply->_nick;
			obj["sex"] = apply->_sex;
			obj["status"] = apply->_status;
			rtvalue["apply_list"].append(obj);
		}
	}


	std::vector<std::shared_ptr<UserInfo>> friend_list;
	bool fsuccess = GetFriendList(uid, friend_list);
	if (fsuccess) {
		for (auto f : friend_list) {
			Json::Value obj;
			obj["name"] = f->name;
			obj["uid"] = f->uid;
			obj["icon"] = f->icon;
			obj["desc"] = f->desc;
			obj["sex"] = f->sex;
			obj["nick"] = f->nick;
			rtvalue["friend_list"].append(obj);
		}
	}

	//
	auto lock = uid_str;
	std::string identifier = RedisMjr::GetInstance()->acquireLock(lock, ACQUIRE_TIME_OUT, LOCK_TIME_OUT);
	if (identifier.empty()) {
		rtvalue["error"] = ErrorCodes::SERVER_BUSY; // 或者特定的抢锁失败错误码
		return;
	}
	Defer defer2([this, &identifier,&lock ]{
		RedisMjr::GetInstance()->releaseLock(lock, identifier);
		});

	std::string user_server_key = USERIPPREFIX + uid_str;
	std::string ip = "";
	bool success = RedisMjr::GetInstance()->Get(user_server_key, ip);
	if (success) {
		if (!ip.empty()) {
			auto& cfg = ConfigMgr::Inst();
			std::string server_curr = cfg["SelfServer"]["Name"];
			if (server_curr == ip) {
				auto old_session=UserMgr::GetInstance()->GetSession(uid);
				if (old_session && old_session->GetSessionId() != session->GetSessionId()) {
						std::cout << "Kick old session: " << old_session->GetSessionId() << std::endl;
						old_session->NotifyOffline();
						// ClearSession 会在 NotifyOffline 内部的定时器优雅关闭后执行，这里不需要重复 Clear
				}
			}
			else {
				KickUserReq req;
				req.set_uid(uid);
				ChatGrpcClient::GetInstance()->NotifyKickUser(ip, req);
			}

		}

	}




	

	//Log in server logic
	auto config = ConfigMgr::Inst();
	auto server_name = config["SelfServer"]["Name"];
	//std::string c = "";
	//bool redissuccess = RedisMjr::GetInstance()->HGet(LOGIN_COUNT, server_name, c);
	//int count = 0;
	//if (!c.empty()) {
	//	count = std::stoi(c);
	//}
	//count++;

	//std::string count_str = std::to_string(count);
	//bool Setsuccess = RedisMjr::GetInstance()->HSet(LOGIN_COUNT, server_name, count_str);
	//if (!Setsuccess) {
	//	return;
	//}

	session->SetUserId(uid);

	std::string user_uuid = USERIPPREFIX + uid_str;
	std::string user_session = USER_SESSION_PREFIX + uid_str;
	RedisMjr::GetInstance()->Set(user_uuid, server_name);
	UserMgr::GetInstance()->SetUserSession(uid, session);
	RedisMjr::GetInstance()->Set(user_session, session->GetSessionId());

	std::cout << "User " << uid << " logged in successfully on pure memory mode." << std::endl;
	return;



}

void LogicWorker::PostMsgToQue(shared_ptr < LogicNode> msg) {
	std::unique_lock<std::mutex> lock(_mutex);
	_msg_que.push(msg);
	if (_msg_que.size() == 1) {
		lock.unlock();
		_consume.notify_one();
	}
}

void LogicWorker::SetServer(std::shared_ptr<Cserver> pserver) {
	_p_server = pserver;
}

void LogicWorker::DealMsg() {
	for (;;) {
		std::unique_lock<std::mutex> lock(_mutex);
		while (_msg_que.empty()&&!_b_stop) {
			_consume.wait(lock);
		}

		if (_b_stop) {
			while (!_msg_que.empty()) {
				auto msg = _msg_que.front();
				std::cout << "Receive msg id is " << msg->_recvnode->_msg_id;
				
				auto call_back = _fun_callbacks.find(msg->_recvnode->_msg_id);
				if (call_back == _fun_callbacks.end()) {
					_msg_que.pop();
					continue;
				}
				call_back->second(msg->_session, msg->_recvnode->_msg_id, std::string(msg->_recvnode->_data, msg->_recvnode->_total_len));
				_msg_que.pop();



			}
		break;
		}
		auto msg_node = _msg_que.front();
		cout << "recv_msg id  is " << msg_node->_recvnode->_msg_id << endl;
		auto call_back_iter = _fun_callbacks.find(msg_node->_recvnode->_msg_id);
		if (call_back_iter == _fun_callbacks.end()) {
			_msg_que.pop();
			std::cout << "msg id [" << msg_node->_recvnode->_msg_id << "] handler not found" << std::endl;
			continue;
		}
		call_back_iter->second(msg_node->_session, msg_node->_recvnode->_msg_id,
			std::string(msg_node->_recvnode->_data, msg_node->_recvnode->_total_len));
		_msg_que.pop();

	}
}

bool LogicWorker::GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userInfo) {
	std::string info_Str = "";
	bool success = RedisMjr::GetInstance()->Get(base_key, info_Str);
	if (success) {
		Json::Reader reader;
		Json::Value value;
		reader.parse(info_Str, value);
		userInfo->name = value["name"].asString();
		userInfo->email = value["email"].asString();
		userInfo->uid = value["uid"].asInt();
		userInfo->passwd = value["passwd"].asString();
		userInfo->nick = value["nick"].asString();
		userInfo->icon = value["icon"].asString();
		userInfo->desc = value["desc"].asString();
		userInfo->sex = value["sex"].asInt();
		std::cout << "GetBaseInfo get :User uid is " << userInfo->uid << " , email is " << userInfo->email << " , password is " << userInfo->passwd << " , name is " << userInfo->name;
	}
	else{
		std::shared_ptr<UserInfo> user_info=nullptr;
		user_info=MysqlMgr::GetInstance()->GetUser(uid);
		if (user_info == nullptr) {
			return false;
		}
		userInfo = user_info;

		Json::Value user_json;
		user_json["uid"] = userInfo->uid;
		user_json["name"] = userInfo->name;
		user_json["email"] = userInfo->email;
		user_json["passwd"] = userInfo->passwd;
		user_json["nick"] = userInfo->nick;
		user_json["icon"] = userInfo->icon;
		user_json["desc"] = userInfo->desc;
		user_json["sex"] = userInfo->sex;

		RedisMjr::GetInstance()->Set(base_key, user_json.toStyledString());
	}
	return true;
}

void LogicWorker::SearchUser(shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data,root);
	auto uid = root["uid"].asString();
	cout << "User search user with uid : " << uid;
	Json::Value rt;
	Defer defer([this, &rt, session]() {
		auto rt_str = rt.toStyledString();
		session->Send(rt_str, ID_SEARCH_USER_RSP);
		});

	bool isUid = isPureDigit(uid);
	if (isUid) {
		SearchUserByUid(uid,rt);
	}
	else {
		SearchUserByName(uid, rt);
	}


}

void LogicWorker::AddFriendApply(shared_ptr<CSession> session, const short& msg_id, const string & msg_data) {
	Json::Reader reader;
	Json::Value value;
	reader.parse(msg_data, value);
	auto uid = value["uid"].asInt();
	auto applyname = value["applyname"].asString();
	auto nickname=value["nickname"].asString();
	auto to_uid = value["to_uid"].asInt();
	cout << "Apply friend receive from user id " << uid << ", applyname is " << applyname << ", nickname is " << nickname << ", to uid " << to_uid;


	Json::Value rtvalue;
	rtvalue["error"] = ErrorCodes::Success;
	Defer defer([this, &rtvalue, session]() {
		std::string rt_str = rtvalue.toStyledString();
		session->Send(rt_str, ID_ADD_FRIEND_RSP);
		});
	MysqlMgr::GetInstance()->AddFriendApply(uid, to_uid);
	auto to_str = std::to_string(to_uid);
	auto key = USERIPPREFIX + to_str;
	std::string user_addr="";
	bool success = RedisMjr::GetInstance()->Get(key, user_addr);
	if (!success) {
		rtvalue["error"] = ErrorCodes::EmailNotMatch;
		return;
	}

	std::string server_str=ConfigMgr::Inst()["SelfServer"]["Name"];
	std::string base_key = USER_BASE_INFO + std::to_string(to_uid);
	auto to_userInfo = std::make_shared<UserInfo>();
	bool bsuccess = GetBaseInfo(base_key, to_uid, to_userInfo);

	if (server_str == user_addr) {
		auto session = UserMgr::GetInstance()->GetSession(to_uid);
		if (session) {
			Json::Value notify;
			notify["error"] = ErrorCodes::Success;
			notify["applyuid"] = uid;
			notify["name"] = applyname;
			if (bsuccess) {
				notify["desc"] = to_userInfo->desc;
				notify["icon"] = to_userInfo->icon;
				notify["sex"] = to_userInfo->sex;
				notify["nick"] = to_userInfo->nick;
			}
			std::string reply_str = notify.toStyledString();
			session->Send(reply_str,ID_NOTIFY_ADD_FRIEND_REQ);
		}
		return;
	}

	AddFriendReq req;
	req.set_name(applyname);
	req.set_applyuid(uid);
	req.set_touid(to_uid);
	req.set_desc("");
	if (bsuccess) {
		req.set_icon(to_userInfo->icon);
		req.set_sex(to_userInfo->sex);
		req.set_nick(to_userInfo->nick);
	}
	ChatGrpcClient::GetInstance()->NotifyAddFriend(user_addr, req);
	
}

void LogicWorker::AuthFriendApply  (shared_ptr<CSession> session, const short& msg_id, const string& msg_data)
{
	Json::Reader reader;
	Json::Value value;
	reader.parse(msg_data, value);
	auto uid = value["fromuid"].asInt();
	auto to_uid = value["touid"].asInt();
	auto nick = value["nickname"].asString();
	cout << "Auth friend receive from user id " << uid << " to user id " << to_uid << " which nickname is " << nick;

	auto user_info = std::make_shared<UserInfo>();
	std::string basic_key = USER_BASE_INFO + std::to_string(to_uid);

	Json::Value rt;
	bool success = GetBaseInfo(basic_key, to_uid, user_info);
	if (!success) {
		cout << "Error , friend does not exist";
		rt["error"] = ErrorCodes::UidInvalid;
		return;
	}
	else {
		rt["name"] = user_info->name;
		//rt["email"] = user_info->email;
		rt["desc"] = user_info->desc;
		rt["sex"] = user_info->sex;
		rt["icon"] = user_info->icon;
		rt["nickname"] = user_info->nick;
		rt["uid"] = to_uid;
		
	}
	rt["error"] = ErrorCodes::Success;
	Defer defer([this, &session, &rt,msg_id] {
		std::string rt_str = rt.toStyledString();
		session->Send(rt_str, ID_AUTH_FRIEND_RSP);
	});

	std::vector<std::shared_ptr <message::AddFriendMsg>> chat_datas;
	int thread_id = 0;
	bool add_res=MysqlMgr::GetInstance()->AddFriend(uid, to_uid,nick, chat_datas,thread_id);
	if (!add_res) {
		rt["error"] = ErrorCodes::Error_Json; // 或其它错误码
		return;
	}
	rt["thread_id"] = thread_id;
	rt["chat_datas"] = Json::Value(Json::arrayValue);
	auto chat_time = getCurrentTimestamp();
	for (const auto& chat_data : chat_datas) {
		Json::Value chat;
		chat["sender"] = chat_data->sender_id();
		chat["msg_id"] = chat_data->msg_id();
		chat["thread_id"] = thread_id;
		chat["unique_id"] = chat_data->unique_id();
		chat["msg_content"] = chat_data->msgcontent();
		chat["chat_time"] = chat_time;
		chat["status"] = chat_data->status();
		rt["chat_datas"].append(chat);
	}

	auto key = USERIPPREFIX + std::to_string(to_uid);
	std::string to_server = "";
	bool rsuccess=RedisMjr::GetInstance()->Get(key, to_server);
	if (!rsuccess) {
		return;
	}

	auto& cfg = ConfigMgr::Inst();
	std::string thisserver = cfg["SelfServer"]["Name"];
	if (thisserver == to_server) {
		auto Rsession = UserMgr::GetInstance()->GetSession(to_uid);
		if (Rsession) {
			Json::Value value;
			value["error"] = ErrorCodes::Success;
			value["fromuid"] = uid;
			value["to_uid"] = to_uid;
			value["thread_id"] = thread_id;
			value["chat_datas"] = rt["chat_datas"];
			std::string base_key = USER_BASE_INFO + std::to_string(uid);
			auto sender_info = std::make_shared<UserInfo>();
			bool findsuccess = GetBaseInfo(base_key, uid, sender_info);
			if (findsuccess) {
				value["name"] = sender_info->name;
				value["nickname"] = sender_info->nick;
				value["icon"] = sender_info->icon;
				value["sex"] = sender_info->sex;
			}
			else {
				value["error"] = ErrorCodes::UidInvalid;
			}

			std::string rtvalue = value.toStyledString();
			Rsession->Send(rtvalue, ID_NOTIFY_AUTH_FRIEND_REQ);
		}
		return;

	}
	AuthFriendReq req;
	req.set_fromuid(uid);
	req.set_touid(to_uid);

	for (const auto& chat_data : chat_datas) {
		auto text_msg = req.add_textmsgs();
		text_msg->CopyFrom(*chat_data);
		text_msg->set_thread_id(thread_id);
	}

	ChatGrpcClient::GetInstance()->NotifyAuthFriend(to_server, req);

}

void LogicWorker::DealChatTextMsg(shared_ptr<CSession> session, const short& msg_id, const string& msg_data)
{
	Json::Value value;
	Json::Reader reader;
	if (!reader.parse(msg_data, value)) {
		return;
	}

	auto from_uid = value["from_uid"].asInt();
	auto to_uid = value["to_uid"].asInt();
	auto local_id = value["msg_id"].asInt(); // 客户端本地临时分配的 UUID
	auto thread_id = value["thread_id"].asInt();
	int chat_msg_id = 0;

	std::string content = value["content"].asString();

	// 🎯 修正 1：强类型枚举转换为 int 传入
	bool b_saved = MysqlMgr::GetInstance()->SaveNewMsg(
		from_uid,
		thread_id,
		content,
		static_cast<int>(ChatMsgType::TEXT),
		chat_msg_id
	);

	Json::Value rt;

	// 🎯 修正 2：入库失败先填好错误信息，确保 Defer 无论在何处 return 都能把 ACK 弹回客户端
	if (!b_saved || chat_msg_id <= 0) {
		rt["error"] = ErrorCodes::SAVE_MSG_FAILED;
		rt["local_id"] = local_id;
		rt["thread_id"] = thread_id;
		session->Send(rt.toStyledString(), ID_TEXT_CHAT_MSG_RSP);
		return;
	}

	// 🎯 修正 3：在定义 Defer 前完整装配好发送给发送方的回执包
	rt["error"] = ErrorCodes::Success;
	rt["from_uid"] = from_uid;
	rt["to_uid"] = to_uid;
	rt["thread_id"] = thread_id;
	rt["content"] = content;
	rt["msg_id"] = chat_msg_id; // 真实落库 ID
	rt["local_id"] = local_id;    // 临时 UUID

	// 保证发送端必定能收到带完整 ID 的 ACK 回应
	Defer defer([&rt, &session]() {
		std::string rt_str = rt.toStyledString();
		session->Send(rt_str, ID_TEXT_CHAT_MSG_RSP);
		});

	// 🎯 修正 4：类名拼写修正为 RedisMgr，变量名重命名为 b_get_ip
	auto key = USERIPPREFIX + std::to_string(to_uid);
	std::string server_ip = "";
	bool b_get_ip = RedisMjr::GetInstance()->Get(key, server_ip);
	if (!b_get_ip) {
		// 对端不在线，消息已在 MySQL 中持久化，直接 return 让 defer 发送 ACK
		return;
	}

	auto& cfg = ConfigMgr::Inst();
	std::string selfserver = cfg["SelfServer"]["Name"];

	// 1. 同服务器投递
	if (server_ip == selfserver) {
		auto to_session = UserMgr::GetInstance()->GetSession(to_uid);
		if (to_session) {
			Json::Value notify_json;
			notify_json["error"] = ErrorCodes::Success;
			notify_json["from_uid"] = from_uid;
			notify_json["to_uid"] = to_uid;
			notify_json["thread_id"] = thread_id;
			notify_json["msg_id"] = chat_msg_id;
			notify_json["type"] = "text";
			notify_json["content"] = content;

			to_session->Send(notify_json.toStyledString(), ID_NOTIFY_TEXT_CHAT_MSG_REQ);
		}
		return;
	}

	// 2. 跨服务器通过 gRPC 投递
	TextChatMsgReq msgReq;
	msgReq.set_fromuid(from_uid);
	msgReq.set_touid(to_uid);
	msgReq.set_thread_id(thread_id);

	TextChatData* data = msgReq.add_textmsgs();
	data->set_msg_id(chat_msg_id);
	data->set_msgcontent(content);

	std::cout << "Forward to server: " << server_ip
		<< ", real msg_id: " << chat_msg_id << std::endl;

	ChatGrpcClient::GetInstance()->NotifyTextChatMsg(server_ip, msgReq);


}

bool LogicWorker::isPureDigit(const std::string& word) {
	for (char c : word) {
		if(!isdigit(c)){
			return false;
		}
	}
	return true;
}

bool LogicWorker::GetFriendApply(int to_uid, std::vector<std::shared_ptr<ApplyInfo>>& list)
{

	return MysqlMgr::GetInstance()->GetApplyList(to_uid, list, 0, 10);
}

void LogicWorker::SearchUserByUid(const std::string& uid, Json::Value& rtvalue) {
	rtvalue["error"] = ErrorCodes::Success;
	auto uid_str = USER_BASE_INFO + uid;
	std::string user_Info_str="";
	bool success=RedisMjr::GetInstance()->Get(uid_str, user_Info_str);
	if (success) {
		Json::Reader reader;
		Json::Value root;
		reader.parse(user_Info_str, root);
		auto uid = root["uid"].asInt();
		auto name = root["name"].asString();
		auto pwd = root["passwd"].asString();
		auto email = root["email"].asString();
		auto nick = root["nick"].asString();
		auto desc = root["desc"].asString();
		auto sex = root["sex"].asInt();
		auto icon = root["icon"].asString();
		std::cout << "user  uid is  " << uid << " name  is "
			<< name << " pwd is " << pwd << " email is " << email << " icon is " << icon << endl;

		rtvalue["uid"] = uid;
		rtvalue["pwd"] = pwd;
		rtvalue["name"] = name;
		rtvalue["email"] = email;
		rtvalue["nick"] = nick;
		rtvalue["desc"] = desc;
		rtvalue["sex"] = sex;
		rtvalue["icon"] = icon;
		return;
	}

	std::shared_ptr<UserInfo> userInfo = nullptr;
	int uid_int = std::stoi(uid);
	userInfo = MysqlMgr::GetInstance()->GetUser(uid_int);
	if (userInfo == nullptr) {
		rtvalue["error"] = ErrorCodes::UidInvalid;
		return;
	}

	Json::Value redis_root;
	redis_root["uid"] = userInfo->uid;
	redis_root["passwd"] = userInfo->passwd;
	redis_root["name"] = userInfo->name;
	redis_root["email"] = userInfo->email;
	redis_root["nick"] = userInfo->nick;
	redis_root["desc"] = userInfo->desc;
	redis_root["sex"] = userInfo->sex;
	redis_root["icon"] = userInfo->icon;

	RedisMjr::GetInstance()->Set(uid_str, redis_root.toStyledString());

	//返回数据
	rtvalue["uid"] = userInfo->uid;
	rtvalue["passwd"] = userInfo->passwd;
	rtvalue["name"] = userInfo->name;
	rtvalue["email"] = userInfo->email;
	rtvalue["nick"] = userInfo->nick;
	rtvalue["desc"] = userInfo->desc;
	rtvalue["sex"] = userInfo->sex;
	rtvalue["icon"] = userInfo->icon;


}
void LogicWorker::SearchUserByName(const std::string& name, Json::Value& rtvalue) {
	rtvalue["error"] = ErrorCodes::Success;

	std::string base_key = NAME_INFO + name;

	//优先查redis中查询用户信息
	std::string info_str = "";
	bool b_base = RedisMjr::GetInstance()->Get(base_key, info_str);
	if (b_base) {
		Json::Reader reader;
		Json::Value root;
		reader.parse(info_str, root);
		auto uid = root["uid"].asInt();
		auto name = root["name"].asString();
		auto pwd = root["pwd"].asString();
		auto email = root["email"].asString();
		auto nick = root["nick"].asString();
		auto desc = root["desc"].asString();
		auto sex = root["sex"].asInt();
		auto icon = root["icon"].asString();
		std::cout << "user  uid is  " << uid << " name  is "
			<< name << " pwd is " << pwd << " email is " << email << endl;

		rtvalue["uid"] = uid;
		rtvalue["pwd"] = pwd;
		rtvalue["name"] = name;
		rtvalue["email"] = email;
		rtvalue["nick"] = nick;
		rtvalue["desc"] = desc;
		rtvalue["sex"] = sex;
		rtvalue["icon"] = icon;
		return;
	}

	//redis中没有则查询mysql
	//查询数据库
	std::shared_ptr<UserInfo> user_info = nullptr;
	user_info = MysqlMgr::GetInstance()->GetUser(name);
	if (user_info == nullptr) {
		rtvalue["error"] = ErrorCodes::UidInvalid;
		return;
	}

	//将数据库内容写入redis缓存
	Json::Value redis_root;
	redis_root["uid"] = user_info->uid;
	redis_root["pwd"] = user_info->passwd;
	redis_root["name"] = user_info->name;
	redis_root["email"] = user_info->email;
	redis_root["nick"] = user_info->nick;
	redis_root["desc"] = user_info->desc;
	redis_root["sex"] = user_info->sex;
	redis_root["icon"] = user_info->icon;

	RedisMjr::GetInstance()->Set(base_key, redis_root.toStyledString());

	//返回数据
	rtvalue["uid"] = user_info->uid;
	rtvalue["pwd"] = user_info->passwd;
	rtvalue["name"] = user_info->name;
	rtvalue["email"] = user_info->email;
	rtvalue["nick"] = user_info->nick;
	rtvalue["desc"] = user_info->desc;
	rtvalue["sex"] = user_info->sex;
	rtvalue["icon"] = user_info->icon;
}

bool LogicWorker::GetFriendList(int uid, std::vector<std::shared_ptr<UserInfo>>& friend_list) {
	return MysqlMgr::GetInstance()->GetFriendList(uid, friend_list);
}

void LogicWorker::UploadFile(shared_ptr<CSession> session, const short& msg_id, const string& msg_data)
{
	std::cout << "Upload file in logic system succesfully done";
	Json::Value root;
	Json::Reader reader;
	reader.parse(msg_data, root);

	int from_uid = root["from_uid"].asInt();
	int to_uid = root["to_uid"].asInt();
	auto filename = root["filename"].asString();
	auto filesz = root["filesz"].asInt64();
	auto md5 = root["md5"].asString();

	Json::Value rt;
	rt["error"] = ErrorCodes::Success;
	rt["from_uid"] = from_uid;
	rt["to_uid"] = to_uid;
	rt["filename"] = filename;

	Defer defer([&rt, session]() {
		auto rtstr = rt.toStyledString();
		session->Send(rtstr, ID_SEND_FILE_RSP);
	});

	auto rsp=FileGrpcClient::GetInstance()->ApplyUploadTicket(from_uid, to_uid, filename, filesz,md5);
	if (rsp.error() != 0) {
		rt["error"] = rsp.error();
		return;
	}


	rt["fileserver_ip"] = rsp.ip();
	rt["fileserver_port"] = rsp.port();
	rt["token"] = rsp.token();


}

void LogicWorker::DealChatFileMsg(shared_ptr<CSession> session, const short& msg_id, const string& msg_data)
{
	Json::Value value;
	Json::Reader reader;
	if (!reader.parse(msg_data, value)) {
		std::cerr << "Failed to parse DealChatFileMsg json!" << std::endl;
		return;
	}

	auto from_uid = value["from_uid"].asInt();
	auto to_uid = value["to_uid"].asInt();
	auto chat_msg_id = value["msg_id"].asInt();
	std::string content = value["content"].asString(); // 如果客户端直接传了详细字段，这里取详细字段

	// 1. 回包给发送者 ACK
	Json::Value rt;
	rt["error"] = ErrorCodes::Success;
	rt["from_uid"] = from_uid;
	rt["to_uid"] = to_uid;
	rt["msg_id"] = chat_msg_id;
	rt["type"] = "file";

	Defer defer([&rt, &session]() {
		std::string rt_str = rt.toStyledString();
		session->Send(rt_str, ID_TEXT_CHAT_MSG_RSP);
		});

	// 2. 检查接收方所在服务器
	auto key = USERIPPREFIX + std::to_string(to_uid);
	std::string server_ip = "";
	bool success = RedisMjr::GetInstance()->Get(key, server_ip);
	if (!success) {
		// 对方离线，可落库存储为离线消息
		return;
	}

	// 3. 构造用于发给接收方的统一消息格式
	Json::Value notify_json;
	notify_json["from_uid"] = from_uid;
	notify_json["to_uid"] = to_uid;
	notify_json["error"] = ErrorCodes::Success;

	Json::Value msg_item;
	msg_item["msg_id"] = chat_msg_id;
	msg_item["type"] = "file";
	msg_item["filename"] = value["filename"];
	msg_item["filesz"] = value["filesz"];
	msg_item["token"] = value["token"];
	msg_item["md5"] = value["md5"];
	msg_item["content"] = content.empty() ? value["filename"].asString() : content;

	Json::Value text_array;
	text_array.append(msg_item);
	notify_json["text"] = text_array;

	auto& cfg = ConfigMgr::Inst();
	std::string selfserver = cfg["SelfServer"]["Name"];

	// ──► 情况 A：对端在同一台服务器上 ◄──
	if (server_ip == selfserver) {
		auto to_session = UserMgr::GetInstance()->GetSession(to_uid);
		if (to_session) {
			std::string notify_str = notify_json.toStyledString();
			to_session->Send(notify_str, ID_NOTIFY_TEXT_CHAT_MSG_REQ);
		}
		return;
	}

	// ──► 情况 B：对端在另一台服务器上，走 gRPC 跨服调用 ◄──
	TextChatMsgReq msgReq;
	msgReq.set_fromuid(from_uid);
	msgReq.set_touid(to_uid);

	TextChatData* data = msgReq.add_textmsgs();
	data->set_msg_id(chat_msg_id);

	// 如果 content 本身不是 json，将详细属性打包成 json 字符串塞入 msgcontent
	if (content.empty() || content[0] != '{') {
		Json::Value pack;
		pack["filename"] = value["filename"];
		pack["filesz"] = value["filesz"];
		pack["token"] = value["token"];
		pack["md5"] = value["md5"];
		data->set_msgcontent(pack.toStyledString());
	}
	else {
		data->set_msgcontent(content);
	}

	ChatGrpcClient::GetInstance()->NotifyTextChatMsg(server_ip, msgReq);
}

void LogicWorker::DownloadFile(shared_ptr<CSession> session, const short& msg_id, const string& msg_data)
{
	Json::Value root;
	Json::Reader reader;
	if (!reader.parse(msg_data, root)) {
		std::cerr << "[LogicWorker] Failed to parse DownloadFile JSON!" << std::endl;
		return;
	}

	int uid = root["uid"].asInt();
	std::string token = root["token"].asString();
	std::string filename = root["filename"].asString();

	std::string key = FILE_UPLOAD + token;
	std::string file_info;

	Json::Value rt;

	// 1. 查询 Redis 任务是否存在
	bool success = RedisMjr::GetInstance()->Get(key, file_info);
	if (!success) {
		std::cout << "[LogicWorker] Download task not found for token: " << token << std::endl;
		rt["error"] = ErrorCodes::FILE_NOT_EXIST;
		session->Send(rt.toStyledString(), ID_DOWNLOAD_FILE_RSP);
		return; // ✅ 必须加 return，拦截后续逻辑
	}

	// 2. 解析存储在 Redis 里的文件元数据
	Json::Value file;
	if (!reader.parse(file_info, file)) {
		std::cerr << "[LogicWorker] Failed to parse file_info JSON from Redis!" << std::endl;
		rt["error"] = ErrorCodes::Error_Json;
		session->Send(rt.toStyledString(), ID_DOWNLOAD_FILE_RSP);
		return; // ✅ 必须加 return
	}

	// 3. 读取 FileServer 配置
	auto& cfg = ConfigMgr::Inst();
	std::string host = cfg["FileServer"]["Host"];
	int port = std::stoi(cfg["FileServer"]["Port"]);

	// 4. 组装响应（注意统一小写键名，并补充 filesz）
	rt["error"] = ErrorCodes::Success;
	rt["host"] = host;
	rt["port"] = port;
	rt["token"] = token;
	rt["filename"] = file.isMember("filename") ? file["filename"].asString() : filename;
	rt["filesz"] = file["filesz"].asInt64(); // ✅ 关键：提供真实文件大小供客户端计算进度和终止边界

	std::string rt_str = rt.toStyledString();
	session->Send(rt_str, ID_DOWNLOAD_FILE_RSP);

	std::cout << "[LogicWorker] Issued download ticket for UID: " << uid
		<< " Token: " << token << " File: " << rt["filename"].asString()
		<< " Size: " << rt["filesz"].asInt64() << " Bytes"
		<< " Target FileServer: " << host << ":" << port << std::endl;
}

void LogicWorker::HeartBeatHandle(shared_ptr<CSession> session, const short& msg_id, const string& msg_data)
{
	Json::Value value;
	Json::Reader reader;
	reader.parse(msg_data, value);
	int uid = value["from_uid"].asInt();

	session->UpdateHeartBeat();
	Json::Value rt;
	rt["error"] = ErrorCodes::Success;
	session->Send(rt.toStyledString(), ID_HEARTBEAT_RSP);
}

void LogicWorker::GetUserThreadHandler(shared_ptr<CSession> session, const short& msg_id, const string& msg_data)
{
	Json::Value value;
	Json::Reader reader;
	reader.parse(msg_data, value);

	int uid = value["uid"].asInt();
	std::cout << "Get thread id for the user: " << uid << endl;

	Json::Value rt;
	rt["error"] = ErrorCodes::Success;
	rt["uid"] = uid;
	auto last_id = value["last_id"].asInt();
	Defer defer([this, &session, &rt]() {
		auto rt_str = rt.toStyledString();
		session->Send(rt_str, ID_LOAD_CHAT_THREAD_RSP);
		});

	std::vector<std::shared_ptr<ChatThreadInfo>> threads;
	int page_size = 10;
	bool load_more = false;
	int next_last_id = 0;
	bool res = GetUserThread(uid,last_id ,page_size,threads,load_more,next_last_id);
	if (!res) {
		rt["error"] = ErrorCodes::UidInvalid;
		return;
	}

	for (auto chat : threads) {
		Json::Value thread;
		thread["uid_1"] = chat->_user1_id;
		thread["uid_2"] = chat->_user2_id;
		thread["thread_id"] = chat->_thread_id;
		thread["type"] = chat->_type;
		rt["threads"].append(thread);
	}

}

bool LogicWorker::GetUserThread(int uid, int last_id, int page_size,std::vector<std::shared_ptr<ChatThreadInfo>>& threads,bool& load_more,int& next_last_id)
{
	return MysqlMgr::GetInstance()->GetUserThread(uid, last_id,page_size,threads,load_more,next_last_id);
}

void LogicWorker::CreateNewPrivateThread(shared_ptr<CSession> session, const short& msg_id, const string& msg_data)
{
	Json::Value value;
	Json::Reader reader;
	reader.parse(msg_data, value);
	int uid = value["uid"].asInt();
	int to_uid = value["other_uid"].asInt();
	int thread_id = 0;

	Json::Value rt;
	rt["uid"] = uid;
	rt["other_uid"] = to_uid;
	Defer defer([this, &session, &rt]() {
		session->Send(rt.toStyledString(), ID_CREATE_PRIVATE_CHAT_RSP);
	});

	auto success=MysqlMgr::GetInstance()->CreatePrivateChatThread(uid, to_uid, thread_id);
	if (!success) {
		rt["error"] = ErrorCodes::UidInvalid;
		return ;
	}
	rt["error"] = ErrorCodes::Success;
	rt["thread_id"] = thread_id;

	return;
}

void LogicWorker::LoadChatMsg(shared_ptr<CSession> session, const short& msg_id, const string& msg_data)
{
	Json::Value val;
	Json::Reader reader;
	reader.parse(msg_data, val);
	int from_uid = val["uid"].asInt();
	int to_uid = val["to_uid"].asInt();
	int thread_id = val["thread_id"].asInt();
	int last_msg_id = val["last_msg_id"].asInt();
	int pgsize = val["pgsize"].asInt();

	Json::Value rt;
	rt["error"] = ErrorCodes::Success;
	rt["from_uid"] = from_uid;
	rt["to_uid"] = to_uid;
	rt["chat_datas"] = Json::arrayValue;
	rt["thread_id"] = thread_id;

	Defer defer([session, &rt]() {
		session->Send(rt.toStyledString(), ID_LOAD_CHAT_MSG_RSP);
		});

	std::shared_ptr<PageResult> pages = MysqlMgr::GetInstance()->LoadChatMsg(thread_id,last_msg_id, pgsize);
	if (!pages) {
		rt["error"] = ErrorCodes::LOAD_CHAT_FAILED;
		return;
	}

	rt["last_msg_id"] = pages->next_cursor;
	rt["load_more"] = pages->load_more;
	for (auto& chat : pages->messages) {
		Json::Value  chat_data;
		chat_data["sender"] = chat.sender_id;
		chat_data["msg_id"] = chat.message_id;
		chat_data["thread_id"] = chat.thread_id;
		chat_data["unique_id"] = 0;
		chat_data["msg_content"] = chat.content;
		chat_data["created_at"] = chat.created_at;
		chat_data["updated_at"] = chat.updated_at;
		chat_data["status"] = chat.status;
		chat_data["type"] = chat.type;

		if(chat.type == 1 || !chat.filename.empty()) {
			chat_data["filename"] = chat.filename;
			chat_data["filesz"] = static_cast<Json::Int64>(chat.filesz);
			chat_data["file_url"] = chat.file_url;
			chat_data["token"] = chat.token;
			chat_data["md5"] = chat.md5;
		}

		rt["chat_datas"].append(chat_data);
	}

}

void LogicSystem::PostMsgtoQue(shared_ptr<LogicNode> msg)
{
	if (!msg)return;

	std::size_t hash_val= std::hash<std::string>{}(msg->_session->GetSessionId());
	int index = hash_val % MAX_LOGICWORKER;
	_pool[index]->PostMsgToQue(msg);
}

void LogicSystem::SetServer(std::shared_ptr<Cserver> pserver)
{
	for (auto& work : _pool) {
		work->SetServer(pserver);
	}
}

bool LogicSystem::GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userInfo)
{
	std::string info_Str = "";
	bool success = RedisMjr::GetInstance()->Get(base_key, info_Str);
	if (success) {
		Json::Reader reader;
		Json::Value value;
		reader.parse(info_Str, value);
		userInfo->name = value["name"].asString();
		userInfo->email = value["email"].asString();
		userInfo->uid = value["uid"].asInt();
		userInfo->passwd = value["passwd"].asString();
		userInfo->nick = value["nick"].asString();
		userInfo->icon = value["icon"].asString();
		userInfo->desc = value["desc"].asString();
		userInfo->sex = value["sex"].asInt();
		std::cout << "GetBaseInfo get :User uid is " << userInfo->uid << " , email is " << userInfo->email << " , password is " << userInfo->passwd << " , name is " << userInfo->name;
	}
	else {
		std::shared_ptr<UserInfo> user_info = nullptr;
		user_info = MysqlMgr::GetInstance()->GetUser(uid);
		if (user_info == nullptr) {
			return false;
		}
		userInfo = user_info;

		Json::Value user_json;
		user_json["uid"] = userInfo->uid;
		user_json["name"] = userInfo->name;
		user_json["email"] = userInfo->email;
		user_json["passwd"] = userInfo->passwd;
		user_json["nick"] = userInfo->nick;
		user_json["icon"] = userInfo->icon;
		user_json["desc"] = userInfo->desc;
		user_json["sex"] = userInfo->sex;

		RedisMjr::GetInstance()->Set(base_key, user_json.toStyledString());
	}
	return true;
}


LogicSystem::LogicSystem():sz(MAX_LOGICWORKER)
{
	for (int i = 0;i < sz;i++) {
		auto workptr=std::make_shared<LogicWorker>();
		//workptr->Start();
		_pool.push_back(workptr);
	}
}


LogicSystem::~LogicSystem() {
	for (auto& work : _pool) {
		work->End();
	}
	_pool.clear();
}