#include "ChatServiceImpl.h"
#include "UserMgr.h"
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include "RedisMjr.h"
#include "CSession.h"
#include "MySqlMgr.h"

#include <chrono>
#include <cstdint>

inline int64_t getCurrentTimestamp() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()
	).count();
}


ChatServiceImpl::ChatServiceImpl() {

}
Status ChatServiceImpl::NotifyAddFriend(ServerContext* context, const AddFriendReq* req, AddFriendRsp* rsp) {
	std::cout << "Server correctly being notify" << std::endl;
	int uid = req->applyuid();
	int to_uid = req->touid();
	Defer defer([&rsp,uid,to_uid]() {
		rsp->set_applyuid(uid);
		rsp->set_touid(to_uid);
		rsp->set_error(ErrorCodes::Success);
		});
	auto session = UserMgr::GetInstance()->GetSession(to_uid);
	if (session == nullptr) {
		return Status::OK;
	}
	Json::Value rt;
	rt["applyuid"] = uid;
	rt["error"] = ErrorCodes::Success;
	rt["name"] = req->name();
	rt["desc"] = req->desc();
	rt["icon"] = req->icon();
	rt["sex"] = req->sex();
	rt["nick"] = req->nick();
	std::string rt_str = rt.toStyledString();
	session->Send(rt_str, ID_NOTIFY_ADD_FRIEND_REQ);

	return Status::OK;
}
Status ChatServiceImpl::NotifyAuthFriend(ServerContext* context, const AuthFriendReq* req, AuthFriendRsp* rsp) {
	auto from_uid = req->fromuid();
	auto to_uid = req->touid();
	auto session = UserMgr::GetInstance()->GetSession(to_uid);
	Defer defer([this, rsp, from_uid,to_uid] {
		rsp->set_error(ErrorCodes::Success);
		rsp->set_fromuid(from_uid);
		rsp->set_touid(to_uid);
		});
	if (session == nullptr) {
		return Status::OK;
	}

	Json::Value rt;
	auto user_info = std::make_shared<UserInfo>();
	auto key = USER_BASE_INFO + std::to_string(from_uid);
	bool bsuccess=GetBaseInfo(key,from_uid, user_info);
	if (!bsuccess) {
		rt["error"] = ErrorCodes::UidInvalid;
	}
	else {
		rt["error"] = ErrorCodes::Success;
	}
	rt["fromuid"] = from_uid;
	rt["to_uid"] = to_uid;
	rt["touid"] = to_uid;
	rt["name"] = user_info->name;
	rt["nick"] = user_info->nick;
	rt["icon"] = user_info->icon;
	rt["desc"] = user_info->desc;
	rt["sex"] = user_info->sex;
	int thread_id = 0;
	rt["chat_datas"] = Json::Value(Json::arrayValue);
	auto chat_time = getCurrentTimestamp();
	for (int i = 0;i < req->textmsgs_size();i++) {
		const auto& text_msg = req->textmsgs(i);
		if (thread_id == 0 && text_msg.thread_id() > 0) {
			thread_id = text_msg.thread_id();
		}

		Json::Value chat;
		chat["sender"] = text_msg.sender_id();
		chat["msg_id"] = text_msg.msg_id();
		chat["thread_id"] = text_msg.thread_id();
		chat["unique_id"] = text_msg.unique_id();
		chat["msg_content"] = text_msg.msgcontent();
		chat["chat_time"] = static_cast<Json::Int64>(chat_time);
		chat["status"] = text_msg.status();
		rt["chat_datas"].append(chat);
	}

	rt["thread_id"] = thread_id;



	auto rt_str = rt.toStyledString();
	session->Send(rt_str, ID_NOTIFY_AUTH_FRIEND_REQ);
	std::cout << "Successfully sent ID_NOTIFY_AUTH_FRIEND_REQ to user " << to_uid << std::endl;
	return Status::OK;

}

Status ChatServiceImpl::NotifyTextChatMsg(ServerContext* context, const TextChatMsgReq* req, TextChatMsgRsp* rsp) {
	int to_uid = req->touid();
	int from_uid = req->fromuid();
	int thread_id = req->thread_id();
	auto session = UserMgr::GetInstance()->GetSession(to_uid);
	if (session == nullptr) {
		// 目标用户不在这台服务器上（可能刚下线）
		return Status::OK;
	}

	rsp->set_error(ErrorCodes::Success);
	rsp->set_fromuid(from_uid);
	rsp->set_touid(to_uid);
	rsp->set_thread_id(thread_id);

	Json::Value rt;
	rt["from_uid"] = from_uid;
	rt["to_uid"] = to_uid;
	rt["error"] = ErrorCodes::Success;
	rt["thread_id"] = thread_id;

	Json::Value text_array;
	for (auto& msg : req->textmsgs()) {
		Json::Value element;
		element["content"] = msg.msgcontent();
		element["msg_id"] = msg.msg_id();
		//element["unique_id"] = msg.unique_id();

		// 尝试判断 content 是否是包含文件信息的 JSON
		Json::Value content_json;
		Json::Reader reader;
		if (reader.parse(msg.msgcontent(), content_json) && content_json.isObject() && content_json.isMember("token")) {
			element["type"] = "file";
			element["filename"] = content_json["filename"];
			element["filesz"] = content_json["filesz"];
			element["token"] = content_json["token"];
			element["md5"] = content_json["md5"];
		}
		else {
			element["type"] = "text";
		}

		text_array.append(element);
	}

	rt["text_array"] = text_array;

	std::string rt_str = rt.toStyledString();
	// 统一用 ID_NOTIFY_TEXT_CHAT_MSG_REQ 下发，客户端解析 text 数组即可
	session->Send(rt_str, ID_NOTIFY_TEXT_CHAT_MSG_REQ);

	return Status::OK;
}

Status ChatServiceImpl::NotifyKickUser(ServerContext* context, const KickUserReq* req, KickUserRsp* rsp)
{
	auto uid = req->uid();
	rsp->set_uid(uid);

	auto session = UserMgr::GetInstance()->GetSession(uid);
	if (!session) {
		// 用户在本节点并不存在（可能已经提前掉线），对踢人操作而言直接判定为成功完成
		rsp->set_error(ErrorCodes::Success);
		return Status::OK;
	}

	std::cout << "[ChatServer] Received gRPC NotifyKickUser for UID: " << uid << std::endl;

	// 1. 发送被顶号下线通知包（内部包含 200ms 定时器延时 Close 和 ClearSession）
	session->NotifyOffline();

	// 2. 立即从本地内存管理器的在线表中解绑该用户
	UserMgr::GetInstance()->RmvUserSession(uid,session->get_uuid());

	// 3. 明确应答主叫端执行成功
	rsp->set_error(ErrorCodes::Success);
	return Status::OK;
}

bool ChatServiceImpl::GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo> userinfo) {
	std::string info_str = "";
	bool success=RedisMjr::GetInstance()->Get(base_key, info_str);
	if (success) {
		Json::Reader reader;
		Json::Value value;
		reader.parse(info_str,value);
		userinfo->name = value["name"].asString();
		userinfo->uid = value["uid"].asInt();
		userinfo->desc = value["desc"].asString();
		userinfo->icon = value["icon"].asString();
		userinfo->sex = value["sex"].asInt();
		userinfo->nick = value["nick"].asString();
		std::cout << "user login id is " << userinfo->uid << " , name is " << userinfo->name << " , user desc is " << userinfo->desc;

	}
	else {
		std::shared_ptr<UserInfo> user;
		user=MysqlMgr::GetInstance()->GetUser(uid);
		if (user == nullptr) {
			return false;
		}
		userinfo = user;
	}
	return true;
}