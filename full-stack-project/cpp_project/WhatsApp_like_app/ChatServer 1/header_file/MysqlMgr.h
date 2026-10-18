#include "const.h"
#include "MysqlPool.h"
#include "UserMgr.h"
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "message.pb.h"
#include "data.h"

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
class MysqlMgr :public Singleton<MysqlMgr> {
	friend class Singleton<MysqlMgr>;

public:
	~MysqlMgr();
	int RegUser(const std::string& name, const std::string& email, const std::string& passwd);
	bool CheckEmail(const std::string& name, const std::string& email);
	bool UpdatePwd(const std::string& name, const std::string& email, const std::string& passwd);
	bool CheckPwd(const std::string& email, const std::string& passwd, UserInfo& userinfo);
	bool AddFriendApply(const int& from_uid, const int& to_uid);
	bool AddFriend(int uid, int to_uid, std::string nickname,std::vector<std::shared_ptr<message::AddFriendMsg>>& chat_data,int &thread_id);
	std::shared_ptr<UserInfo> GetUser(int uid);
	std::shared_ptr<UserInfo> GetUserByEmail(const std::string& email);
	std::shared_ptr<UserInfo> GetUser(const std::string& name);
	bool GetApplyList(int  to_uid, std::vector < std::shared_ptr<ApplyInfo>>& list, int begin, int limit = 10);
	bool GetFriendList(int uid, std::vector<std::shared_ptr<UserInfo>>& friend_list);
	bool GetUserThread(int uid, int last_id, int page_size, std::vector<std::shared_ptr<ChatThreadInfo>>& threads, bool& load_more, int& next_last_id);
	bool CreatePrivateChatThread(int uid_1, int uid_2, int& thread_id);
	bool SaveNewMsg(int from_uid, int thread_id, std::string content, int type, int& chat_msg_id);
	std::shared_ptr<PageResult> LoadChatMsg(int thread_id, int last_msg_id, int pgsize);

private:
	MysqlMgr()=default;
	MysqlDao _dao;
};