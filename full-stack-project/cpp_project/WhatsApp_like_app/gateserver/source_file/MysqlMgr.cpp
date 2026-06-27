#include "MysqlMgr.h"
int MysqlMgr::RegUser(const std::string& name, const std::string& email, const std::string& passwd) {
	return _dao.RegUser(name, email, passwd);
}

MysqlMgr::~MysqlMgr() {
}