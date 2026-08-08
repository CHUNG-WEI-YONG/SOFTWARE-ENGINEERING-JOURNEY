#include "const.h"

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
	MysqlPool(std::string& url, std::string& user, std::string& passwd, std::size_t size, std::string& schema);

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
	std::shared_ptr<UserInfo> GetUser(int uid);
	std::shared_ptr<UserInfo> GetUser(const std::string& name);
	std::shared_ptr<UserInfo> GetUserByEmail(const std::string& email);
};