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

struct UserInfo {
	std::string name;
	std::string passwd;
	std::string email;
	int u_id;
};


//final use class
class MysqlDao {
private:
	std::unique_ptr<MysqlPool> _pool;
public:
	MysqlDao();
	~MysqlDao();
	int RegUser(const std::string& name, const std::string& email, const std::string& paswd);
};