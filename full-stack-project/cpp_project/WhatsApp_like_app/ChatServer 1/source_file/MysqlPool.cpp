#include "MysqlPool.h"
#include "ConfigMgr.h"

MysqlPool::MysqlPool(std::string& url,std::string& user, std::string& passwd,std::size_t size,std::string& schema):
_url(url),_user(user),_passwd(passwd),_Poolsize(size),_schema(schema),_b_stop(false),_fail_conn(0)
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
				CheckConnectionPro();
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

void MysqlPool::CheckConnectionPro() {
	size_t targetCount;
	{
		std::lock_guard<mutex> lock(_mutex);
		targetCount = _conns.size();

	}

	size_t processedCount = 0;
	auto now = std::chrono::system_clock::now().time_since_epoch();
	long long time_stamp = std::chrono::duration_cast<std::chrono::seconds>(now).count();
	bool healthy = true;
	while (processedCount < targetCount) {
		std::unique_ptr<SqlConnection> con;
		{
			std::lock_guard<mutex> lock(_mutex);
			if (_conns.empty()) {
				break;
			}
			con = std::move(_conns.front());
			_conns.pop();

		}

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
				healthy = false;
				_fail_conn++;
			}
		}
		if(healthy)
		{
			std::lock_guard<mutex> lock(_mutex);
			_conns.push(std::move(con));
			_cv.notify_one();
		}

		processedCount++;
	}

	while (_fail_conn > 0) {
		auto b_success=reconnect(time_stamp);
		if (b_success) {
			_fail_conn--;
		}
		else {
			break;
		}
	}
	
}

bool MysqlPool::reconnect(long long timestamp) {
	try {
		auto session = std::make_unique<mysqlx::Session>(_url);
		session->sql("USE " + _schema).execute();
		{
			std::lock_guard<mutex> lock(_mutex);
			_conns.push(std::make_unique<SqlConnection>(std::move(session), timestamp));
			
		}
		std::cout << "Mysql Connection reconnect" << endl;
		return true;

	}
	catch (mysqlx::Error& e) {
		std::cout << "Reconnect failed on mysql" << endl;
		return false;
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

bool MysqlDao::AddFriend(int uid, int to_uid, std::string nickname, std::vector<std::shared_ptr<message::AddFriendMsg>>& chat_data,int &thread_id)
{
	chat_data.clear();
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
		std::string reversed_back = "";
		std::string apply_desc = "";

		// 加上反引号避免保留字冲突（或根据实际表结构使用 descs / back_name）
		std::string sel_apply_sql =
			"SELECT `nick_name`, `desc` FROM friend_apply "
			"WHERE from_uid = ? AND to_uid = ? "
			"FOR UPDATE;";

		auto result = conn->_session->sql(sel_apply_sql).bind(to_uid, uid).execute();
		auto row = result.fetchOne();
		if (!row) {
			conn->_session->rollback();
			return false;
		}

		if (!row[0].isNull()) {
			reversed_back = row[0].get<std::string>();
		}
		if (!row[1].isNull()) {
			apply_desc = row[1].get<std::string>();
		}

		// 2. 更新申请状态为已通过 (status = 1)
		std::string upd_apply_sql =
			"UPDATE friend_apply SET status = 1 WHERE from_uid = ? AND to_uid = ?;";
		auto upd_res = conn->_session->sql(upd_apply_sql).bind(to_uid, uid).execute();
		if (upd_res.getAffectedItemsCount() == 0) {
			conn->_session->rollback();
			return false;
		}

		// 3. 建立双向好友记录
		std::string insert_friend_sql = "INSERT IGNORE INTO friend (self_id, friend_id, back) VALUES (?, ?, ?);";
		conn->_session->sql(insert_friend_sql).bind(uid, to_uid, nickname).execute();
		conn->_session->sql(insert_friend_sql).bind(to_uid, uid, reversed_back).execute();

		// 4. 规整 UID 顺序创建/查询私聊会话
		int64_t u1 = std::min(uid, to_uid);
		int64_t u2 = std::max(uid, to_uid);
		thread_id = 0;

		std::string check_thread_sql =
			"SELECT thread_id FROM private_chat WHERE user1_id = ? AND user2_id = ? FOR UPDATE;";
		auto check_res = conn->_session->sql(check_thread_sql).bind(u1, u2).execute();
		auto thread_row = check_res.fetchOne();

		if (thread_row) {
			thread_id = thread_row[0].get<int64_t>();
		}
		else {
			std::string insert_thread_sql =
				"INSERT INTO chat_thread (type, created_at) VALUES ('private', NOW());";
			auto th_res = conn->_session->sql(insert_thread_sql).execute();
			thread_id = static_cast<int64_t>(th_res.getAutoIncrementValue());
			if (thread_id == 0) {
				conn->_session->rollback();
				return false;
			}

			std::string insert_pc_sql =
				"INSERT INTO private_chat(thread_id, user1_id, user2_id, created_at) VALUES (?, ?, ?, NOW());";
			conn->_session->sql(insert_pc_sql).bind(thread_id, u1, u2).execute();
		}

		// 5. 消息插入 SQL（严格匹配当前 chat_message 表的列结构）
		std::string insert_msg_sql =
			"INSERT INTO chat_message(thread_id, sender_id, content, created_at, updated_at, status) "
			"VALUES (?, ?, ?, NOW(), NOW(), 0);";

		// 插入附带的验证描述
		if (!apply_desc.empty()) {
			auto msg_res = conn->_session->sql(insert_msg_sql).bind(thread_id, to_uid, apply_desc).execute();
			uint64_t msg_id = msg_res.getAutoIncrementValue();
			auto tx_data = std::make_shared<message::AddFriendMsg>();
			tx_data->set_sender_id(to_uid);
			tx_data->set_msg_id(static_cast<int64_t>(msg_id));
			tx_data->set_msgcontent(apply_desc);
			tx_data->set_thread_id(thread_id);
			tx_data->set_unique_id("");
			chat_data.push_back(tx_data);
		}

		// 6. 插入通过好友默认打招呼消息（✅ 修正：使用 insert_msg_sql）
		std::string welcome_str = "Hello Friend , Nice to meet you!";
		auto wel_res = conn->_session->sql(insert_msg_sql).bind(thread_id, uid, welcome_str).execute();
		uint64_t wel_msg_id = wel_res.getAutoIncrementValue();

		auto wel_data = std::make_shared<message::AddFriendMsg>();
		wel_data->set_sender_id(uid);
		wel_data->set_msg_id(static_cast<int64_t>(wel_msg_id));
		wel_data->set_msgcontent(welcome_str);
		wel_data->set_thread_id(thread_id);
		wel_data->set_unique_id("");
		chat_data.push_back(wel_data);

		// 7. 提交整个事务
		conn->_session->commit();
		return true;
	}
	catch (const mysqlx::Error& e) {
		try {
			conn->_session->rollback();
		}
		catch (...) {}
		std::cerr << "AddFriend MySQL XDevAPI Exception: " << e.what() << std::endl;
		return false;
	}
	catch (const std::exception& e) {
		try {
			conn->_session->rollback();
		}
		catch (...) {}
		std::cerr << "AddFriend General Exception: " << e.what() << std::endl;
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

bool MysqlDao::GetUserThread(int uid, int last_id, int page_size, std::vector<std::shared_ptr<ChatThreadInfo>>& threads, bool& load_more, int& next_last_id)
{
	load_more = false;
	next_last_id = last_id;
	threads.clear();
	auto conn = _pool->getConn();
	if (conn == nullptr) {
		return false;
	}
	Defer defer([this, &conn]() {
		_pool->returnConn(std::move(conn));
		});

	try {
		
		std::string query =
			"WITH all_threads AS ( "
			// 1. 私聊分支：显式将 0 转为有符号整数 SIGNED
			"  SELECT thread_id, 'private' AS type, user1_id, user2_id, "
			"         '' AS group_name, CAST(0 AS SIGNED) AS owner_id, '' AS avatar_url "
			"    FROM private_chat "
			"   WHERE (user1_id = ? OR user2_id = ?) "
			"     AND thread_id > ? "
			"  UNION ALL "
			// 2. 群聊分支
			"  SELECT m.thread_id, 'group' AS type, "
			"         CAST(0 AS SIGNED) AS user1_id, CAST(0 AS SIGNED) AS user2_id, "
			"         g.name AS group_name, g.owner_id, g.avatar_url "
			"    FROM group_chat_member m "
			"    JOIN group_chat g ON m.thread_id = g.thread_id "
			"   WHERE m.user_id = ? "
			"     AND m.thread_id > ? "
			") "
			"SELECT thread_id, type, user1_id, user2_id, group_name, owner_id, avatar_url "
			"  FROM all_threads "
			" ORDER BY thread_id ASC "
			" LIMIT ?;";



		auto sql_stmt = conn->_session->sql(query);
		// 绑定参数：user1_id, user2_id, thread_id 游标, 取 page_size + 1 条
		sql_stmt.bind(uid, uid, last_id,uid,last_id, page_size + 1);

		auto res = sql_stmt.execute();


		//std::vector<std::shared_ptr<ChatThreadInfo>> temp;
		//.getting check for the data type 
		auto get_safe_int64 = [](const mysqlx::Value& val) -> int64_t {
			if (val.isNull() || val.getType() == mysqlx::Value::Type::VNULL) {
				return 0;
			}

			switch (val.getType()) {
			case mysqlx::Value::Type::UINT64:
				return static_cast<int64_t>(val.get<uint64_t>());

			case mysqlx::Value::Type::INT64:
				return val.get<int64_t>();

			case mysqlx::Value::Type::STRING: {
				std::string s = val.get<std::string>();
				return s.empty() ? 0 : std::stoll(s);
			}

			case mysqlx::Value::Type::DOUBLE:
				return static_cast<int64_t>(val.get<double>());

			case mysqlx::Value::Type::FLOAT:
				return static_cast<int64_t>(val.get<float>());

			default:
				return 0;
			}
			};
		std::vector<std::shared_ptr<ChatThreadInfo>> temp;
		for (auto row : res) {
			auto cti = std::make_shared<ChatThreadInfo>();

			// 0. thread_id (bigint unsigned)
			cti->_thread_id = get_safe_int64(row[0]);

			// 1. type (string)
			if (!row[1].isNull() && row[1].getType() != mysqlx::Value::Type::VNULL) {
				cti->_type = row[1].get<std::string>();
			}

			// 2. user1_id (bigint unsigned)
			cti->_user1_id = get_safe_int64(row[2]);

			// 3. user2_id (bigint unsigned)
			cti->_user2_id = get_safe_int64(row[3]);

			// 4. group_name
			if (!row[4].isNull() && row[4].getType() != mysqlx::Value::Type::VNULL) {
				cti->_group_name = row[4].get<std::string>();
			}

			// 5. owner_id
			cti->_owner_id = get_safe_int64(row[5]);

			// 6. avatar_url
			if (!row[6].isNull() && row[6].getType() != mysqlx::Value::Type::VNULL) {
				cti->_avatar_url = row[6].get<std::string>();
			}

			temp.push_back(cti);
		}

		if ((int)temp.size() > page_size) {
			temp.pop_back();
			load_more = true;
		}

		if (!temp.empty()) {
			next_last_id = static_cast<int>(temp.back()->_thread_id);
		}
		threads = std::move(temp);
		return true;

	} 
	catch (const mysqlx::Error& err) {
		std::cerr << "MySQL X DevAPI error in GetUserThread: " << err.what() << std::endl;
		return false;
	}
	catch (const std::exception& e) {
		std::cerr << "Exception in GetUserThread: " << e.what() << std::endl;
		return false;
	}
}

bool MysqlDao::CreatePrivateChatThread(int uid, int to_uid, int& thread_id)
{
	auto conn = _pool->getConn();
	if (!conn) {
		return false;
	}
	Defer defer([&conn, this]() {
		_pool->returnConn(std::move(conn));
	});
	try {
		conn->_session->startTransaction();

		int user_1 = std::min(uid, to_uid);
		int user_2 = std::max(uid, to_uid);

		std::string check_sql = "SELECT thread_id from private_chat where user1_id=? and user2_id=? for update;";
		auto check_res = conn->_session->sql(check_sql).bind(user_1).bind(user_2).execute();
		auto row = check_res.fetchOne();
		if (row) {
			thread_id = static_cast<int>(row[0].get<int64_t>());
			conn->_session->commit();
			return true;
		}

		std::string insert_thread = "INSERT INTO chat_thread (type, created_at) VALUES ('private', NOW());";
		auto res = conn->_session->sql(insert_thread).execute();
		uint64_t new_thread_id = res.getAutoIncrementValue();
		if (new_thread_id == 0) {
			conn->_session->rollback();
			return false;
		}
		thread_id = static_cast<int>(new_thread_id);

		// 3. 将会话关联记录插入 private_chat 表
		std::string insert_private_sql =
			"INSERT INTO private_chat (thread_id, user1_id, user2_id, created_at) "
			"VALUES (?, ?, ?, NOW());";

		conn->_session->sql(insert_private_sql)
			.bind(new_thread_id, user_1, user_2)
			.execute();

		// 提交事务
		conn->_session->commit();
		return true;

	}
	catch (const mysqlx::Error& err) {
		std::cerr << "[MysqlDao::CreatePrivateChat] MySQL XDevAPI Error: "
			<< err.what() << std::endl;
		try {
			conn->_session->rollback();
		}
		catch (...) {}
		return false;
	}
	catch (const std::exception& e) {
		std::cerr << "[MysqlDao::CreatePrivateChat] Exception: "
			<< e.what() << std::endl;
		try {
			conn->_session->rollback();
		}
		catch (...) {}
		return false;
	}
}

bool MysqlDao::SaveNewMsg(int from_uid, int thread_id, std::string content, int type, int& chat_msg_id)
{
	auto conn = _pool->getConn();
	if (!conn) {
		return false;
	}

	Defer defer([this, &conn]() {
		_pool->returnConn(std::move(conn));
		});

	try {
		auto& session = conn->_session;

		// 🎯 修正：字段名必须与数据库的 msg_type 严格保持一致
		std::string sql_str = "INSERT INTO chat_message (thread_id, sender_id, content, msg_type, created_at, updated_at, status) "
			"VALUES (?, ?, ?, ?, NOW(), NOW(), 1)";

		auto rsp = session->sql(sql_str)
			.bind(thread_id, from_uid, content, type)
			.execute();

		auto row = rsp.getAffectedItemsCount();
		if (row == 0) {
			return false;
		}

		// 提取自增生成的真实主键 ID
		chat_msg_id = static_cast<int>(rsp.getAutoIncrementValue());
		return true;
	}
	catch (const mysqlx::Error& err) {
		std::cerr << "[MysqlDao::SaveNewMsg] MySQL XDevAPI Error: "
			<< err.what() << std::endl;
		return false;
	}
	catch (const std::exception& e) {
		std::cerr << "[MysqlDao::SaveNewMsg] Exception: "
			<< e.what() << std::endl;
		return false;
	}
}

std::shared_ptr<PageResult> MysqlDao::LoadChatMsg(int thread_id, int last_msg_id, int pgsize)
{
	auto conn = _pool->getConn();
	if (!conn) {
		return nullptr;
	}
	Defer defer([this, &conn]() {
		_pool->returnConn(std::move(conn));
		});

	try {
		// 修正字段名、表别名、逗号连接以及歧义列
		std::string sqlprompt =
			"SELECT msg.message_id, msg.thread_id, msg.sender_id, msg.msg_type, msg.content, "
			"       DATE_FORMAT(msg.created_at, '%Y-%m-%d %H:%i:%s') AS created_at_str, "
			"       DATE_FORMAT(msg.updated_at, '%Y-%m-%d %H:%i:%s') AS updated_at_str, "
			"       msg.status, "
			"       file.file_name, file.file_size, file.file_type, file.file_url, "
			"       file.token, file.md5 "
			"  FROM chat_message AS msg "
			"  LEFT JOIN message_attachment AS file ON msg.message_id = file.message_id "
			" WHERE msg.thread_id = ? AND msg.message_id > ? "
			" ORDER BY msg.message_id ASC "
			" LIMIT ?;";

		auto sql_stmt = conn->_session->sql(sqlprompt);
		sql_stmt.bind(
			static_cast<uint64_t>(thread_id),
			static_cast<uint64_t>(last_msg_id),
			static_cast<uint64_t>(pgsize + 1)
		);

		auto res = sql_stmt.execute();

		auto page_result = std::make_shared<PageResult>();
		page_result->load_more = false;
		page_result->next_cursor = last_msg_id;

		// 安全提取函数，兼容 UINT64 / INT64，返回 64 位整型避免文件大小溢出
		auto get_safe_int64 = [](const mysqlx::Value& val) -> int64_t {
			if (val.isNull() || val.getType() == mysqlx::Value::Type::VNULL) {
				return 0;
			}
			switch (val.getType()) {
			case mysqlx::Value::Type::UINT64:
				return static_cast<int64_t>(val.get<uint64_t>());
			case mysqlx::Value::Type::INT64:
				return val.get<int64_t>();
			case mysqlx::Value::Type::STRING: {
				std::string s = val.get<std::string>();
				return s.empty() ? 0 : std::stoll(s);
			}
			default:
				return 0;
			}
			};

		for (auto row : res) {
			ChatMessage msg;
			msg.message_id = static_cast<int>(get_safe_int64(row[0]));
			msg.thread_id = static_cast<int>(get_safe_int64(row[1]));
			msg.sender_id = static_cast<int>(get_safe_int64(row[2]));
			msg.type = static_cast<int>(get_safe_int64(row[3]));

			if (!row[4].isNull()) {
				msg.content = row[4].get<std::string>();
			}
			if (!row[5].isNull()) {
				msg.created_at = row[5].get<std::string>();
			}
			if (!row[6].isNull()) {
				msg.updated_at = row[6].get<std::string>();
			}
			msg.status = static_cast<int>(get_safe_int64(row[7]));
			msg.unique_id = "";

			// 提取附件相关属性 (若为普通文本行，LEFT JOIN 结果为 NULL)
			if (!row[8].isNull())  msg.filename = row[8].get<std::string>();
			if (!row[9].isNull())  msg.filesz = get_safe_int64(row[9]);
			if (!row[10].isNull()) msg.file_type = row[10].get<std::string>();
			if (!row[11].isNull()) msg.file_url = row[11].get<std::string>();
			if (!row[12].isNull()) msg.token = row[12].get<std::string>();
			if (!row[13].isNull()) msg.md5 = row[13].get<std::string>();

			page_result->messages.push_back(std::move(msg));
		}

		// 分页游标核心判断逻辑：
		if (static_cast<int>(page_result->messages.size()) > pgsize) {
			// 查到了 pgsize + 1 条，表示还有下一页
			page_result->load_more = true;
			// 弹出多查的那 1 条哨兵记录
			page_result->messages.pop_back();
			// 弹出后最后一条正好是第 pgsize 条，用它的 message_id 作为下次游标
			page_result->next_cursor = page_result->messages.back().message_id;
		}
		else {
			// 数量 <= pgsize，没有下一页
			page_result->load_more = false;
			if (!page_result->messages.empty()) {
				page_result->next_cursor = page_result->messages.back().message_id;
			}
		}

		return page_result;
	}
	catch (const mysqlx::Error& e) {
		std::cerr << "[MysqlDao::LoadChatMsg] MySQL XDevAPI Error: "
			<< e.what() << std::endl;
		return nullptr;
	}
	catch (const std::exception& e) {
		std::cerr << "[MysqlDao::LoadChatMsg] Exception: "
			<< e.what() << std::endl;
		return nullptr;
	}
}


