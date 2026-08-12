#include "MysqlMgr.h"
int MysqlMgr::RegUser(const std::string& name, const std::string& email, const std::string& passwd) {
	return _dao.RegUser(name, email, passwd);
}

MysqlMgr::~MysqlMgr() {
}

bool MysqlMgr::CheckEmail(const std::string& name, const std::string& email) {
	return _dao.CheckEmail(name, email);
}

bool MysqlMgr::UpdatePwd(const std::string& name, const std::string& email, const std::string& passwd) {
	return _dao.UpdatePwd(name, email, passwd);
}

bool MysqlMgr::CheckPwd(const std::string& email, const std::string& passwd, UserInfo& userinfo) {
	return _dao.CheckPwd(email, passwd, userinfo);
}

bool MysqlMgr::AddFriendApply(const int& from_uid, const int& to_uid)
{
	return _dao.AddFriendApply(from_uid,to_uid);
}

std::shared_ptr<UserInfo> MysqlMgr::GetUser(int uid)
{
	return _dao.GetUser(uid);
}

std::shared_ptr<UserInfo> MysqlMgr::GetUser(const std::string& name)
{
	return _dao.GetUser(name);
}

bool MysqlMgr::GetApplyList(int to_uid, std::vector<std::shared_ptr<ApplyInfo>>& list, int begin, int limit)
{

	return _dao.GetApplyList(to_uid,list,begin,limit);
}

std::shared_ptr<UserInfo> MysqlMgr::GetUserByEmail(const std::string& email)
{
	return _dao.GetUserByEmail(email);
}
