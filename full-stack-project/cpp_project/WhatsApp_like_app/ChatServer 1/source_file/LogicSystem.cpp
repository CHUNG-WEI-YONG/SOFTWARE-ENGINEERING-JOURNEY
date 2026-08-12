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

	_fun_callbacks[ID_SEARCH_USER_REQ] = [this](shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
		this->SearchUser(session, msg_id, msg_data);
		};
	_fun_callbacks[ID_ADD_FRIEND_REQ] = [this](shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
		this->AddFriendApply(session, msg_id, msg_data);
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
	std::shared_ptr<UserInfo> userInfo = std::make_shared<UserInfo>();
	bool bsuccess = GetBaseInfo(base_key, uid, userInfo);
	if (!bsuccess) {
		rtvalue["error"] = ErrorCodes::UidInvalid;
		return;
	}
	rtvalue["uid"] = uid;
	rtvalue["passwd"] = userInfo->passwd;
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
	bool gsuccess=GetFriendApply(uid, list);
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

bool LogicSystem::GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userInfo) {
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
	}
	return true;
}

void LogicSystem::SearchUser(shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
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

void LogicSystem::AddFriendApply(shared_ptr<CSession> session, const short& msg_id, const string & msg_data) {
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
	if (server_str == user_addr) {
		auto session = UserMgr::GetInstance()->GetSession(to_uid);
		if (session) {
			Json::Value notify;
			notify["error"] = ErrorCodes::Success;
			notify["applyuid"] = uid;
			notify["name"] = applyname;
			notify["desc"] = "";
			std::string reply_str = notify.toStyledString();
			session->Send(reply_str,ID_NOTIFY_ADD_FRIEND_REQ);
		}
		return;
	}

	std::string base_key = USER_BASE_INFO + std::to_string(to_uid);
	auto to_userInfo = std::make_shared<UserInfo>();
	bool bsuccess = GetBaseInfo(base_key, to_uid, to_userInfo);
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

bool LogicSystem::isPureDigit(const std::string& word) {
	for (char c : word) {
		if(!isdigit(c)){
			return false;
		}
	}
	return true;
}

bool LogicSystem::GetFriendApply(int to_uid, std::vector<std::shared_ptr<ApplyInfo>>& list)
{

	return MysqlMgr::GetInstance()->GetApplyList(to_uid, list, 0, 10);
}

void LogicSystem::SearchUserByUid(const std::string& uid, Json::Value& rtvalue) {
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
void LogicSystem::SearchUserByName(const std::string& name, Json::Value& rtvalue) {
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
