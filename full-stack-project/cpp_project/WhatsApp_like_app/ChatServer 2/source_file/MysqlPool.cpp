#include "MysqlPool.h"
#include "ConfigMgr.h"

MysqlPool::MysqlPool(std::string& url,std::string& user, std::string& passwd,std::size_t size,std::string& schema):
_url(url),_user(user),_passwd(passwd),_Poolsize(size),_schema(schema),_b_stop(false)
{
	try {
		for (int i = 0;i < size;i++) {
			auto session = std::make_unique<mysqlx::Session>(_url);
			session->sql("USE " + _schema).execute();
			auto now = std::chrono::duration_cast<std::chrono::seconds>(
				std::chrono::system_clock::now().time_since_epoch()).count();
			_conns.push(std::make_unique<SqlConnection>(std::move(session), now));

		}
		_check_thread = std::thread([this]() {
			while (!_b_stop) {
				checkConnection();
				std::this_thread::sleep_for(std::chrono::seconds(60));
			}
			});
		_check_thread.detach();
	}
	catch (mysqlx::Error& e) {
		std::cout << "Mysqlpool init failed. Error is " << e.what() << endl;
	}

}

void MysqlPool :: checkConnection() {
	std::lock_guard<mutex> lock(_mutex);
	int poolsize = _conns.size();
	auto currentTime = std::chrono::system_clock::now().time_since_epoch();
	long long time_stamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
	for (int i = 0;i < poolsize;i++) {
		auto con = std::move(_conns.front());
		_conns.pop();

		//Defer defer([this, &con]() {
		//	_conns.push(std::move(con));
		//	});
		bool b_conn_ok = false;
		if (time_stamp - con->_last_time >= 60) {
			try {
				con->_session->sql("SELECT 1").execute();
				con->_last_time = time_stamp;
				//std::cout << "Executor alive sql . Timestamp is " << time_stamp << std::endl;
				b_conn_ok = true;
			}
			catch (mysqlx::Error& e) {
				std::cout << "Error in keeping sql conneciton alive.Error is " << e.what() << std::endl;
				auto session = std::make_unique<mysqlx::Session>(_url);
				session->sql("USE " + _schema).execute();
				_conns.push(std::make_unique<SqlConnection>(std::move(session), time_stamp));
			}
		}
		else {
			b_conn_ok = true;
		}
		if (b_conn_ok) {
			_conns.push(std::move(con));
		}

	}
}

std::unique_ptr<SqlConnection> MysqlPool::getConn() {
	std::unique_lock<std::mutex> lock(_mutex);
	if (_b_stop)return nullptr;
	_cv.wait(lock, [this]() {
		if (_b_stop) {
			return true;
		}
		return !_conns.empty();
		});
	std::unique_ptr<SqlConnection> connection=std::move(_conns.front());
	_conns.pop();
	return connection;
}

void MysqlPool::returnConn(std::unique_ptr<SqlConnection> session) {
	std::lock_guard<mutex> lock(_mutex);
	if (_b_stop)return;
	_conns.push(std::move(session));
	_cv.notify_one();
	return;
}

void MysqlPool::end() {
	_b_stop = true;
	_cv.notify_all();
	return;
}

MysqlPool::~MysqlPool() {
	end();
	std::unique_lock<std::mutex> lock(_mutex);
	while (!_conns.empty()) {
		_conns.pop();
	}
	return;
};

MysqlDao::MysqlDao() {
	ConfigMgr config=ConfigMgr::Inst();
	std::string host = config["MySql"]["Host"];
	std::string port = config["MySql"]["Port"];
	std::string passwd = config["MySql"]["Passwd"];
	std::string schema = config["MySql"]["Schema"];
	std::string user = config["MySql"]["User"];

	std::string mysqlx_url = "mysqlx://" + user + ":" + passwd + "@" + host + ":" + port;
	_pool.reset(new MysqlPool(mysqlx_url,user, passwd, 5, schema));
}

MysqlDao::~MysqlDao() {
	_pool->end();
}

int MysqlDao::RegUser(const std::string& name, const std::string& email, const std::string& passwd) {
	auto conn = _pool->getConn();
	if (conn == nullptr) {
		return false;
	}
	try {
		auto result = conn->_session->sql("CALL reg_user(?,?,?,@result)").bind(name).bind(email).bind(passwd).execute();
		auto res_set = conn->_session->sql("SELECT @result").execute();
		mysqlx::Row row = res_set.fetchOne();

		if (row) {
			int result = row[0].get<int>();
			std::cout << "🎯 [MySQLX] 成功提取注册状态码. Result: " << result << std::endl;
			_pool->returnConn(std::move(conn));
			return result;
		}
		return false;
	}
	catch (mysqlx::Error &e) {
		_pool->returnConn(std::move(conn));
		std::cerr << "Sql error: " << e.what();
		return -1;

	}
}

bool MysqlDao::CheckEmail(const std::string& name, const std::string& email) {
	auto conn = _pool->getConn();
	if (conn == nullptr) {
		return false;
	}
	try {
		auto result = conn->_session->sql("SELECT email from user_table where name= ?").bind(name).execute();
		auto row = result.fetchOne();
		if (!row) {
			std::cout << "[MySQL] 查无此人，用户名对撞脱靶: " << name << std::endl;
			_pool->returnConn(std::move(conn)); 
			return false;
		}
		std::string real_email = row[0].get<std::string>();
		if (real_email!=email) {
			std::cout << "No user exists";
			_pool->returnConn(std::move(conn));
			return false;
		}
		_pool->returnConn(std::move(conn));
		return true;

	}
	catch (mysqlx::Error& e) {
		_pool->returnConn(std::move(conn));
		std::cerr << "Sql error: " << e.what();
		return false;
	}
}


bool MysqlDao::UpdatePwd(const std::string& name, const std::string& email, const string& newpasswd) {
	auto conn = _pool->getConn();
	if (conn == nullptr) {
		return false;
	}
	bool r;
	try {
		auto result = conn->_session->sql("UPDATE user_table set pwd=? where email= ?").bind(newpasswd).bind(email).execute();
		if (result.getAffectedItemsCount() == 0) {
			std::cout << "FAILED TO RESET PASSWORD" << std::endl;
			r = false;
		}
		else {
			std::cout << "Affected sql row:" << result.getAffectedItemsCount() << std::endl;
			r= true;

		}
		_pool->returnConn(std::move(conn));
		return r;
	}
	catch (mysqlx::Error& e) {
		_pool->returnConn(std::move(conn));
		std::cerr << "Sql error: " << e.what();
		return false;
	}
}

bool MysqlDao::CheckPwd(const std::string& email, const std::string& passwd, UserInfo& userinfo) {
	auto conn = _pool->getConn();
	if (conn == nullptr) {
		return false;
	}
	try {
		auto result = conn->_session->sql("SELECT uid , name, pwd from user_table where email=?").bind(email).execute();
		auto row = result.fetchOne();
		if (!row) {
			std::cout << "No this user of this email"<<std::endl;
			_pool->returnConn(std::move(conn));
			return false;
		}
		auto pass = row[2].get<std::string>();
		if (passwd != pass) {
			std::cout << "User enter wrong password"<<std::endl;
			_pool->returnConn(std::move(conn));
			return false;
		}
		userinfo.email = email;
		userinfo.name = row[1].get<std::string>();
		userinfo.uid = row[0].get<int>();
		std::cout << "User info get for u_id: " << userinfo.uid << std::endl;;
		_pool->returnConn(std::move(conn));
		return true;

	}
	catch (mysqlx::Error& e) {
		_pool->returnConn(std::move(conn));
		std::cerr << "Sql error: " << e.what();
		return false;
	}
}

bool MysqlDao::AddFriendApply(const int& from_uid, const int& to_uid)
{
	auto conn = _pool->getConn();
	if (conn == nullptr) {
		return false;
	}
	Defer defer([this, &conn]() {
		_pool->returnConn(std::move(conn));
		});
	try {
		auto result = conn->_session->sql("INSERT INTO friend_apply(from_uid,to_uid) values (?,?)"
			"ON DUPLICATE KEY update from_uid=from_uid,to_uid=to_uid").bind(from_uid).bind(to_uid).execute();
		return result.getAffectedItemsCount() >= 0;
	}catch(mysqlx::Error& e) {
		std::cerr << "Sql error: " << e.what();
		return false;
	}
	return true;
}

std::shared_ptr<UserInfo> MysqlDao::GetUser(int uid) {
	auto conn = _pool->getConn();
	if (conn == nullptr) {
		return nullptr;
	}
	Defer defer([this, &conn]() {
		_pool->returnConn(std::move(conn));
		});
	try {
		auto result = conn->_session->sql("SELECT name,pwd,email ,nick,desc,sex,icon from user_table where uid=?").bind(uid).execute();
		auto row = result.fetchOne();
		if (!row) {
			return nullptr;
		}
		std::shared_ptr<UserInfo> user_ptr = std::make_shared<UserInfo>();
		user_ptr->name = row[0].get<std::string>();
		user_ptr->passwd = row[1].get<std::string>();
		user_ptr->email = row[2].get<std::string>();
		user_ptr->nick = row[3].get<std::string>();
		user_ptr->nick = row[3].isNull() ? "" : row[3].get<std::string>();
		user_ptr->desc = row[4].isNull() ? "" : row[4].get<std::string>();
		user_ptr->icon = row[6].isNull() ? "" : row[6].get<std::string>();
		user_ptr->uid = uid;
		return user_ptr;
	}
	catch (mysqlx::Error& e) {
		//_pool->returnConn(std::move(conn));
		std::cerr << "Sql error: " << e.what();
		return nullptr;
	}
}

std::shared_ptr<UserInfo> MysqlDao::GetUser(const std::string& name) {
	auto conn = _pool->getConn();
	if (conn == nullptr) {
		return nullptr;
	}
	Defer defer([this, &conn]() {
		_pool->returnConn(std::move(conn));
		});
	try {
		auto result = conn->_session->sql("SELECT uid,pwd,email from user_table where name=?").bind(name).execute();
		auto row = result.fetchOne();
		if (!row) {
			return nullptr;
		}
		std::shared_ptr<UserInfo> user_ptr = std::make_shared<UserInfo>();
		user_ptr->uid = row[0].get<int>();
		user_ptr->passwd = row[1].get<std::string>();
		user_ptr->email = row[2].get<std::string>();
		user_ptr->name = name;
		return user_ptr;
	}
	catch (mysqlx::Error& e) {
		//_pool->returnConn(std::move(conn));
		std::cerr << "Sql error: " << e.what();
		return nullptr;
	}
}


bool MysqlDao::GetApplyList(int touid, std::vector<std::shared_ptr<ApplyInfo>>& applyList, int begin, int limit) {
	auto con = _pool->getConn();
	if (con == nullptr) {
		return false;
	}

	Defer defer([this, &con]() {
		_pool->returnConn(std::move(con));
		});


	try {
		// 准备SQL语句, 根据起始id和限制条数返回列表
		

		auto result = con->_session->sql("select apply.from_uid, apply.status, user.name, "
			"user.nick, user.sex from friend_apply as apply join user_table as user on apply.from_uid = user.uid where apply.to_uid = ? "
			"and apply.id > ? order by apply.id ASC LIMIT ? ").bind(touid,begin,limit).execute();

		// 遍历结果集
		for (auto row : result) {
			int from_uid = row[0].get<int>();
			int status = row[1].get<int>();
			std::string name = row[2].get<std::string>();
			std::string nick = row[3].get<std::string>();
			int sex = row[4].get<int>();
			auto apply = std::make_shared<ApplyInfo>();
			apply->_uid = from_uid;
			apply->_status = status;
			apply->_name = name;
			apply->_nick = nick;
			apply->_sex = sex;
			applyList.push_back(apply);
		}
		return true;
	}
	catch (mysqlx::Error& e) {
		std::cerr << "SQLException: " << e.what();
		return false;
	}
}

bool MysqlDao::AddFriend(int uid, int to_uid, std::string nickname)
{
	auto conn = _pool->getConn();
	if (conn == nullptr) {
		return false;
	}

	Defer defer([this, &conn]() {
		_pool->returnConn(std::move(conn));
		});

	try {
		// 1. 开启事务保证原子性
		conn->_session->startTransaction();

		// 2. 更新好友申请状态 (0: 申请中, 1: 已同意)
		// 注意：如果是 to_uid 同意 uid 的申请，条件通常为 from_uid = uid AND to_uid = to_uid
		conn->_session->sql(
			"UPDATE friend_apply SET status = 1 WHERE from_uid = ? AND to_uid = ?"
		).bind(uid, to_uid).execute();

		// 3. 插入 A -> B 的好友关系 (去除 WHERE 关键字)
		conn->_session->sql(
			"INSERT IGNORE INTO friend (self_id, friend_id, back) VALUES (?, ?, ?)"
		).bind(uid, to_uid, nickname).execute();

		// 4. 插入 B -> A 的好友关系 (对方对当前用户的备注默认为空 "")
		conn->_session->sql(
			"INSERT IGNORE INTO friend (self_id, friend_id, back) VALUES (?, ?, '')"
		).bind(to_uid, uid).execute();

		// 5. 提交事务
		conn->_session->commit();
		return true;
	}
	catch (mysqlx::Error& e) {
		// 发生异常回滚事务
		conn->_session->rollback();
		std::cerr << "AddFriend SQLException: " << e.what() << std::endl;
		return false;
	}
}

std::shared_ptr<UserInfo> MysqlDao::GetUserByEmail(const std::string& email) {
	auto conn = _pool->getConn();
	if (conn == nullptr) {
		return nullptr;
	}
	Defer defer([this, &conn]() {
		_pool->returnConn(std::move(conn));
		});
	try {
		auto result = conn->_session->sql("SELECT name,pwd,uid from user_table where email=?").bind(email).execute();
		auto row = result.fetchOne();
		if (!row) {
			return nullptr;
		}
		std::shared_ptr<UserInfo> user_ptr = std::make_shared<UserInfo>();
		user_ptr->name = row[0].get<std::string>();
		user_ptr->passwd = row[1].get<std::string>();
		user_ptr->uid = row[2].get<int>();
		user_ptr->email = email;
		return user_ptr;
	}
	catch (mysqlx::Error& e) {
		//	_pool->returnConn(std::move(conn));
		std::cerr << "Sql error: " << e.what();
		return nullptr;
	}
	
}

bool MysqlDao::GetFriendList(int uid, std::vector<std::shared_ptr<UserInfo>>& friend_list)
{
	auto conn = _pool->getConn();
	if (conn == nullptr) {
		return false;
	}
	Defer defer([this, &conn]() {
		_pool->returnConn(std::move(conn));
		});

	try {
		// 一次性关联查询好友信息与备注
		auto result = conn->_session->sql(
			"SELECT friend.back, user.name, user.uid, user.`desc`, user.sex, user.icon "
			"FROM user_table AS user "
			"JOIN friend ON friend.friend_id = user.uid "
			"WHERE friend.self_id = ?"
		).bind(uid).execute();

		for (auto row : result) {
			auto f = std::make_shared<UserInfo>();
			f->nick = row[0].isNull() ? "" : row[0].get<std::string>();
			f->name = row[1].isNull() ? "" : row[1].get<std::string>();
			f->uid = row[2].isNull() ? 0 : row[2].get<int>();
			f->desc = row[3].isNull() ? "" : row[3].get<std::string>();
			f->sex = row[4].isNull() ? 0 : row[4].get<int>();
			f->icon = row[5].isNull() ? "" : row[5].get<std::string>();

			if (f->nick.empty()) {
				f->nick = f->name; // 无备注时默认展示用户名
			}
			friend_list.push_back(f);
		}
		return true;
	}
	catch (const mysqlx::Error& e) {
		std::cerr << "Sql error in GetFriendList: " << e.what() << std::endl;
		return false;
	}
}

