#include "ChatServiceImpl.h"

ChatServiceImpl::ChatServiceImpl() {

}
Status ChatServiceImpl::NotifyAddFriend(ServerContext* context, const AddFriendReq* req, AddFriendRsp* rsp) {
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