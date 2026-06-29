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