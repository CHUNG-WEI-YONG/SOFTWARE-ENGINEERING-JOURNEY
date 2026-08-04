#include "const.h"
#include "MysqlPool.h"

class MysqlMgr :public Singleton<MysqlMgr> {
	friend class Singleton<MysqlMgr>;

public:
	~MysqlMgr();
	int RegUser(const std::string& name, const std::string& email, const std::string& passwd);
	bool CheckEmail(const std::string& name, const std::string& email);
	bool UpdatePwd(const std::string& name, const std::string& email, const std::string& passwd);
	bool CheckPwd(const std::string& email, const std::string& passwd, UserInfo& userinfo);
	std::shared_ptr<UserInfo> GetUser(int uid);
	std::shared_ptr<UserInfo> GetUserByEmail(const std::string& email);

private:
	MysqlMgr()=default;
	MysqlDao _dao;
};