#include "ChatServiceImpl.h"
#include "UserMgr.h"
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include "CSession.h"


ChatServiceImpl::ChatServiceImpl() {

}
Status ChatServiceImpl::NotifyAddFriend(ServerContext* context, const AddFriendReq* req, AddFriendRsp* rsp) {
	int uid = req->applyuid();
	int to_uid = req->touid();
	Defer defer([&rsp,uid,to_uid]() {
		rsp->set_applyuid(uid);
		rsp->set_touid(to_uid);
		rsp->set_error(ErrorCodes::Success);
		});
	auto session = UserMgr::GetInstance()->GetSession(uid);
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
	return Status::OK;
}
Status ChatServiceImpl::NotifyTextChatMsg(ServerContext* context, const TextChatMsgReq* req, TextChatMsgRsp* rsp) {
	return Status::OK;
}

bool ChatServiceImpl::GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo> userinfo) {
	return true;
}