#include "const.h"
#include "LogicSystem.h"
#include "Cserver.h"
#include "MsgNode.h"
#include "StatusGrpcClient.h"
#include "MysqlMgr.h"

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

	if (token != "123456") {
		rtvalue["error"] = ErrorCodes::TokenInvalid;
		return;
	}

	//rtvalue["name"] = "TESTUSER";
	//rtvalue["uid"] = uid;
	rtvalue["error"] = rsp.error();

	if (rsp.error()!= ErrorCodes::Success) {
		return;
	}

	auto iter = _users.find(uid);
	std::shared_ptr<UserInfo> user_info = nullptr;
	if (iter == _users.end()){
		auto userinfo = MysqlMgr::GetInstance()->GetUser(uid);
		if (userinfo == nullptr) {
			rtvalue["error"] = ErrorCodes::UidInvalid;
			return;
		}

		_users[uid] = userinfo;
	}
	else {
		user_info = iter->second;
	}
	rtvalue["uid"] = uid;
	rtvalue["token"] = rsp.token();
	rtvalue["name"] = user_info->name;
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