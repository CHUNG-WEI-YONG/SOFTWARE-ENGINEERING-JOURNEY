#include "const.h"
#include "MysqlPool.h"
#include "UserMgr.h"
class MysqlMgr :public Singleton<MysqlMgr> {
	friend class Singleton<MysqlMgr>;

public:
	~MysqlMgr();
	int RegUser(const std::string& name, const std::string& email, const std::string& passwd);
	bool CheckEmail(const std::string& name, const std::string& email);
	bool UpdatePwd(const std::string& name, const std::string& email, const std::string& passwd);
	bool CheckPwd(const std::string& email, const std::string& passwd, UserInfo& userinfo);
	bool AddFriendApply(const int& from_uid, const int& to_uid);
	bool AddFriend(int uid, int to_uid, std::string nickname);
	std::shared_ptr<UserInfo> GetUser(int uid);
	std::shared_ptr<UserInfo> GetUserByEmail(const std::string& email);
	std::shared_ptr<UserInfo> GetUser(const std::string& name);
	bool GetApplyList(int  to_uid, std::vector < std::shared_ptr<ApplyInfo>>& list, int begin, int limit = 10);
	bool GetFriendList(int uid, std::vector<std::shared_ptr<UserInfo>>& friend_list);

private:
	MysqlMgr()=default;
	MysqlDao _dao;
};