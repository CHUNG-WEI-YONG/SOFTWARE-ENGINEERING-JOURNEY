#include "Singleton.h"
#include <unordered_map>
#include <memory>
#include <mutex>

class CSession;
class UserMgr :public Singleton<UserMgr> {
	friend class Singleton<UserMgr>;

public:
	void SetUserSession(int uid, std::shared_ptr<CSession> session);
	std::shared_ptr<CSession> GetSession(int uid);
	~UserMgr();
	void DelSession(int uid);
	void RmvUserSession(int uid,std::string session_id);
private:
	std::unordered_map<int, std::shared_ptr<CSession>> _session;
	std::mutex _mutex;
	UserMgr();

};