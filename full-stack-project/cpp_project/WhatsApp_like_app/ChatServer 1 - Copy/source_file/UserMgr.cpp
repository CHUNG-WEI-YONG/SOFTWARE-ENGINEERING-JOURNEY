#include "UserMgr.h"
#include "CSession.h"

void UserMgr::SetUserSession(int uid, std::shared_ptr<CSession> session) {
	std::lock_guard<std::mutex> lock(_mutex);
	_session[uid] = session;
}

std::shared_ptr<CSession>UserMgr::GetSession(int uid) {
	std::lock_guard<std::mutex> lock(_mutex);
	auto iter = _session.find(uid);
	if (iter != _session.end()) {
		return iter->second;
	}
	return nullptr;
}

UserMgr::~UserMgr() {
	_session.clear();
}

void UserMgr::DelSession(int uid) {
	auto uid_str = std::to_string(uid);
	std::lock_guard<std::mutex> lock(_mutex);
	_session.erase(uid);
}

void UserMgr::RmvUserSession(int uid, std::string session_id)
{
	{
		std::lock_guard<std::mutex> lock(_mutex);
		auto iter = _session.find(uid);
		if (iter == _session.end()) {
			return;
		}

		auto session_id_ = iter->second->GetSessionId();
		//不相等说明是其他地方登录了
		if (session_id_ != session_id) {
			return;
		}
		_session.erase(uid);
	}

}

UserMgr::UserMgr() {
	// 可为空，或初始化你的成员变量
}