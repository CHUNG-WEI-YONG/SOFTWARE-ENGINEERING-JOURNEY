#include "const.h"
#pragma once
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "message.pb.h"
#include <mutex>
#include "data.h"
#include "CServer.h"
#include <memory>

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


class ChatServiceImpl final:public ChatService::Service {
public:
	ChatServiceImpl();
	Status NotifyAddFriend(ServerContext* context, const AddFriendReq* req, AddFriendRsp* rsp) override;
	Status NotifyAuthFriend(ServerContext* context, const AuthFriendReq* req, AuthFriendRsp* rsp)override;
	Status NotifyTextChatMsg(ServerContext* context, const TextChatMsgReq* req, TextChatMsgRsp* rsp)override;

	bool GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo> userinfo);



};