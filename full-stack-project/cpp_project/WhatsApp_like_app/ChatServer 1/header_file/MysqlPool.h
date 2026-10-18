#include "const.h"
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

class SqlConnection {
public:
	SqlConnection(std::unique_ptr<mysqlx::Session> conn, int64_t last_time) :_session(std::move(conn)), _last_time(last_time) {};
	std::unique_ptr<mysqlx::Session> _session;
	int64_t _last_time;
};

class MysqlPool {
public:
	std::unique_ptr<SqlConnection> getConn();
	void returnConn(std::unique_ptr<SqlConnection>);
	void end();
	~MysqlPool();
	void checkConnection();
	void CheckConnectionPro();
	MysqlPool(std::string& url, std::string& user, std::string& passwd, std::size_t size, std::string& schema);
	bool reconnect(long long timestamp);

private:
	std::condition_variable _cv;
	std::mutex _mutex;
	std::queue<std::unique_ptr<SqlConnection>> _conns;
	std::string _url;
	std::string _user;
	std::string _passwd;
	std::string _schema;
	atomic <bool> _b_stop;
	std::size_t _Poolsize;
	std::thread _check_thread;
	std::atomic<int> _fail_conn;

};




//final use class
class MysqlDao {
private:
	std::unique_ptr<MysqlPool> _pool;
public:
	MysqlDao();
	~MysqlDao();
	int RegUser(const std::string& name, const std::string& email, const std::string& paswd);
	bool CheckEmail(const std::string& name, const std::string& email);
	bool UpdatePwd(const std::string& name, const std::string& email,const string& newpasswd);
	bool CheckPwd(const std::string& email, const std::string& passwd, UserInfo& userinfo);
	bool AddFriendApply(const int& from_uid, const int& to_uid);
	bool GetApplyList(int to_uid, std::vector<std::shared_ptr<ApplyInfo>>& list, int begin, int limit = 5);
	bool AddFriend(int uid, int to_uid, std::string nickname, std::vector < std::shared_ptr<message::AddFriendMsg>>& chat_data,int& thread_id);
	std::shared_ptr<UserInfo> GetUser(int uid);
	std::shared_ptr<UserInfo> GetUser(const std::string& name);
	std::shared_ptr<UserInfo> GetUserByEmail(const std::string& email);
	bool GetFriendList(int uid, std::vector<std::shared_ptr<UserInfo>>& friend_list);
	bool GetUserThread(int uid, int last_id, int page_size, std::vector<std::shared_ptr<ChatThreadInfo>>& threads, bool& load_more, int& next_last_id);
	bool CreatePrivateChatThread(int uid, int to_uid, int& thread_id);
	bool SaveNewMsg(int from_uid, int thread_id, std::string content, int type, int& chat_msg_id);
	std::shared_ptr<PageResult> LoadChatMsg(int thread_id, int last_msg_id, int pgsize);


};