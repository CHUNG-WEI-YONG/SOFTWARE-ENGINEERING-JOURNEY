#include "ChatServiceImpl.h"
#include "UserMgr.h"
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include "RedisMjr.h"
#include "CSession.h"
#include "MySqlMgr.h"


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
	rt["name"] = user_info->name;
	rt["nick"] = user_info->nick;
	rt["icon"] = user_info->icon;
	rt["desc"] = user_info->desc;
	rt["sex"] = user_info->sex;

	auto rt_str = rt.toStyledString();
	session->Send(rt_str, ID_NOTIFY_AUTH_FRIEND_REQ);
	std::cout << "Successfully sent ID_NOTIFY_AUTH_FRIEND_REQ to user " << to_uid << std::endl;
	return Status::OK;

}
Status ChatServiceImpl::NotifyTextChatMsg(ServerContext* context, const TextChatMsgReq* req, TextChatMsgRsp* rsp) {
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