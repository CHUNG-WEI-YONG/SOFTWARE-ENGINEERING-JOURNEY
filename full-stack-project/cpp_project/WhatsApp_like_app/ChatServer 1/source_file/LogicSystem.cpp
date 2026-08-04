#include "const.h"
#include "LogicSystem.h"
#include "Cserver.h"
#include "MsgNode.h"
#include "StatusGrpcClient.h"
#include "MysqlMgr.h"
#include "RedisMjr.h"
#include "ConfigMgr.h"
#include "UserMgr.h"

LogicSystem::LogicSystem():_b_stop(false),_p_server(nullptr) {
	RegisterCallBacks();
	_worker_thread = std::thread(&LogicSystem::DealMsg, this);
}

LogicSystem::~LogicSystem() {
	_b_stop = true;
	_consume.notify_all();
	_worker_thread.join();
}

void LogicSystem::RegisterCallBacks() {
	_fun_callbacks[MSG_CHAT_LOGIN]= [this](shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
		this->LoginHandler(session, msg_id, msg_data);
		};
}

void LogicSystem::LoginHandler(shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
	Json::Reader reader;
	Json::Value root;
	if (!reader.parse(msg_data, root)) {
		return;
	}
	Json::Value rtvalue;

	auto uid = root["uid"].asInt();
	auto token = root["token"].asString();

	auto rsp = StatusGrpcClient::GetInstance()->Login(uid, token);



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
		rtvalue["value"] = ErrorCodes::TokenInvalid;
		return;
	}
	rtvalue["error"] = ErrorCodes::Success;

	std::string base_key = USER_BASE_INFO + uid_str;
	std::shared_ptr<UserInfo> user_info = std::make_shared<UserInfo>();
	bool bsuccess = GetBaseInfo(base_key, uid, user_info);
	if (!bsuccess) {
		rtvalue["error"] = ErrorCodes::UidInvalid;
		return;
	}
	rtvalue["uid"] = uid;
	rtvalue["passwd"] = user_info->passwd;
	rtvalue["email"] = user_info->email;
	rtvalue["desc"] = user_info->desc;
	rtvalue["nick"] = user_info->nick;
	rtvalue["sex"] = user_info->sex;
	rtvalue["icon"] = user_info->icon;
	//rtvalue["token"] = rsp.token();
	rtvalue["name"] = user_info->name;
	//rtvalue["token"] = rsp.token();
	//do for mysql to do the search for friend list and user list

	auto config = ConfigMgr::Inst();
	auto server_name = config["SelfServer"]["Name"];
	std::string c = "";
	bool redissuccess = RedisMjr::GetInstance()->HGet(LOGIN_COUNT, server_name, c);
	int count = 0;
	if (!c.empty()) {
		count = std::stoi(c);
	}
	count++;

	std::string count_str = std::to_string(count);
	bool Setsuccess = RedisMjr::GetInstance()->HSet(LOGIN_COUNT, server_name, count_str);
	if (!Setsuccess) {
		return;
	}

	session->SetUserId(uid);

	std::string user_uuid = USERIPPREFIX + uid_str;
	RedisMjr::GetInstance()->Set(user_uuid, server_name);
	UserMgr::GetInstance()->SetUserSession(uid, session);

	if (_p_server) {
	//	auto old_session = _p_server->getsess(uuid);

	//	if (old_session && old_session->GetSessionId() != session->GetSessionId()) {
	//		// 2. 如果找到了旧连接，且旧连接不是当前这个新连接
	//		old_session->NotifyOffline(uuid); // 给旧连接发一个“你被顶号了”的通知包
	//		_p_server->ClearSession(old_session->GetSessionId()); // 🎯 物理清除【旧连接】，而不是清除当前新连接！
	//	}
	//}
	//	_p_server->ClearSession(session->GetSessionId());
	}
	std::cout << "User " << uid << " logged in successfully on pure memory mode." << std::endl;
	return;



}

void LogicSystem::PostMsgToQue(shared_ptr < LogicNode> msg) {
	std::unique_lock<std::mutex> lock(_mutex);
	_msg_que.push(msg);
	if (_msg_que.size() == 1) {
		lock.unlock();
		_consume.notify_one();
	}
}

void LogicSystem::SetServer(std::shared_ptr<Cserver> pserver) {
	_p_server = pserver;
}

void LogicSystem::DealMsg() {
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

bool LogicSystem::GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& user_info) {
	std::string info_Str = "";
	bool success = RedisMjr::GetInstance()->Get(base_key, info_Str);
	if (success) {
		Json::Reader reader;
		Json::Value value;
		reader.parse(info_Str, value);
		user_info->name = value["name"].asString();
		user_info->email = value["email"].asString();
		user_info->uid = value["uid"].asInt();
		user_info->passwd = value["passwd"].asString();
		user_info->nick = value["nick"].asString();
		user_info->icon = value["icon"].asString();
		user_info->nick = value["desc"].asString();
		user_info->sex = value["sex"].asInt();
		std::cout << "User login id is " << user_info->uid << " , email is " << user_info->email << " , password is " << user_info->passwd << " , name is " << user_info->name;
	}
	else{
		std::shared_ptr<UserInfo> userinfo=nullptr;
		userinfo=MysqlMgr::GetInstance()->GetUser(uid);
		if (user_info == nullptr) {
			return false;
		}
		user_info = userinfo;
	}
	return true;
}