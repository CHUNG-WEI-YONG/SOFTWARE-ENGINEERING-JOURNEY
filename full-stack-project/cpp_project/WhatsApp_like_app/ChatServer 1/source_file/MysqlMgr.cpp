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

bool MysqlMgr::AddFriend(int uid, int to_uid, std::string nickname, std::vector <std::shared_ptr<message::AddFriendMsg>>& chat_data,int& thread_id)
{
	return _dao.AddFriend(uid,to_uid,nickname,chat_data,thread_id);
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

bool MysqlMgr::GetFriendList(int uid, std::vector<std::shared_ptr<UserInfo>>& friend_list)
{
	return _dao.GetFriendList(uid,friend_list);
}

bool MysqlMgr::GetUserThread(int uid, int last_id, int page_size, std::vector<std::shared_ptr<ChatThreadInfo>>& threads, bool& load_more, int& next_last_id)
{
	return _dao.GetUserThread(uid,last_id,page_size, threads,load_more,next_last_id);
}

bool MysqlMgr::CreatePrivateChatThread(int uid_1, int uid_2, int& thread_id)
{
	return _dao.CreatePrivateChatThread(uid_1,uid_2,thread_id);
}

bool MysqlMgr::SaveNewMsg(int from_uid, int thread_id, std::string content, int type, int& chat_msg_id)
{
	return _dao.SaveNewMsg(from_uid,thread_id,content,type,chat_msg_id);
}

std::shared_ptr<PageResult> MysqlMgr::LoadChatMsg(int thread_id, int last_msg_id, int pgsize)
{
	return _dao.LoadChatMsg(thread_id,last_msg_id,pgsize);
}

std::shared_ptr<UserInfo> MysqlMgr::GetUserByEmail(const std::string& email)
{
	return _dao.GetUserByEmail(email);
}
