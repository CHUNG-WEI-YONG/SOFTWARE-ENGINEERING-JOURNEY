#include "usermgr.h"
#include <QJsonArray>

UserMgr::UserMgr():_user_info(nullptr),_chat_loaded(0),_contact_loaded(0){}
UserMgr::~UserMgr()
{

}

void UserMgr::Reset()
{
        // 1. 基础认证与身份信息归零
        _uid = 0;
        _token.clear();
        _logoPath.clear();
        _user_info.reset(); // 释放 UserInfo 智能指针

        // 2. 好友与申请关系容器清空
        _friend_map.clear();
        _friend_list.clear();
        _applications.clear();

        // 3. 分页与列表游标计数复位
        _chat_loaded = 0;
        _contact_loaded = 0;
        last_thread_id = 0;

        // 4. 会话路由与聚合根内存缓存释放 (最关键，防止跨账号数据污染)
        _thread_map.clear();
        _uid_to_thread_id.clear();
        _thread_data_map.clear(); // 释放所有 ChatThreadData 及其内部挂载的多态消息

}

// void UserMgr::SetName(QString name)
// {
//     _name = name;
// }

void UserMgr::SetUid(int uid)
{
    _uid = uid;
}

void UserMgr::SetToken(QString token)
{
    _token = token;
}

std::shared_ptr<UserInfo> UserMgr::GetUserInfo()
{

    return _user_info;
}

QString UserMgr::returnName ()const
{
    return _user_info->_name;
}

void UserMgr::Setlogo(QString path)
{
    _logoPath=path;
}

QString UserMgr::returnLogo() const
{
    return _logoPath;
}

int UserMgr::GetUid() const
{
    return _user_info->_uid;
}

bool UserMgr::has_added(int uid)
{
    for(auto apply:_applications){
        if(apply->_uid==uid){
            return true;
        }
    }
    return false;
}

void UserMgr::add_apply(std::shared_ptr<ApplyInfo> apply)
{
    _applications.push_back(apply);
}

QString UserMgr::GetName() const{
    return _user_info->_name;
}

void UserMgr::SetUserInfo(std::shared_ptr<UserInfo> user)
{
    _user_info=user;
}

void UserMgr::AppendApplyList(QJsonArray array)
{
    for(const QJsonValue &value:array){
        auto name=value["name"].toString();
        auto icon=value["icon"].toString();
        auto desc=value["desc"].toString();
        auto nick=value["nick"].toString();
        auto sex=value["sex"].toInt();
        auto status=value["status"].toInt();
        auto uid=value["uid"].toInt();
        auto info=std::make_shared<ApplyInfo>(uid,name,desc,icon,nick,sex,status);
        _applications.push_back(info);

    }
}

bool UserMgr::CheckFriendById(int uid)
{
    auto iter=_friend_map.find(uid);
    if(iter==_friend_map.end()){
        return false;
    }
    return true;
}

void UserMgr::AddFriend(std::shared_ptr<AuthInfo>auth)
{
    FriendInfo new_friend(auth);
    _friend_map[auth->_uid]=std::make_shared<FriendInfo>(new_friend);
}

void UserMgr::AddFriend(std::shared_ptr<AuthRsp>auth)
{
    FriendInfo new_friend(auth);
    _friend_map[auth->_uid]=std::make_shared<FriendInfo>(new_friend);
}

std::shared_ptr<FriendInfo> UserMgr::getFriend(int uid)
{
    auto iter=_friend_map.find(uid);
    if(iter==_friend_map.end()){
        return nullptr;
    }
    else{
        return iter->second;
    }
}

std::shared_ptr<FriendInfo> UserMgr::getFriendByName(QString name)
{
    for(auto iter=_friend_map.begin();iter!=_friend_map.end();++iter){
        if(name==iter->second->_name){
            return iter->second;
        }
    }
    return nullptr;
}

void UserMgr::AppendFriendList(QJsonArray array)
{
    for(const QJsonValue &value:array){
        auto name=value["name"].toString();
        auto icon=value["icon"].toString();
        auto desc=value["desc"].toString();
        auto nick=value["nick"].toString();
        auto sex=value["sex"].toInt();
        auto status=value["status"].toInt();
        auto uid=value["uid"].toInt();
        // Full constructor
        auto info=std::make_shared<FriendInfo>(uid,name,nick,icon,sex,desc,"","");
        _friend_map[uid]=info;
        _friend_list.push_back(info);

    }
}

std::vector<std::shared_ptr<FriendInfo>> UserMgr::GetChatListPerPage(){
    int start=_chat_loaded;
    int end=_chat_loaded+MAX_COUNT_PER_PAGE;
    if(start>_friend_list.size()){
        return _friend_list;
    }
    if(end>_friend_list.size()){
        return std::vector<std::shared_ptr<FriendInfo>>(_friend_list.begin()+start,_friend_list.end());
    }
    return std::vector<std::shared_ptr<FriendInfo>>(_friend_list.begin()+start,_friend_list.begin()+end);

}

bool UserMgr::isLoadChatFin(){
    return (_chat_loaded>=_friend_list.size());
}

void UserMgr::UpdateChatLoadedCount(){
    int start=_chat_loaded;
    int end=std::min(_chat_loaded+MAX_COUNT_PER_PAGE,static_cast<int>(_friend_list.size()));
    _chat_loaded=end;

}

std::vector<std::shared_ptr<FriendInfo>>UserMgr::GetConListPerPage(){
    int start=_contact_loaded;
    int end=_contact_loaded+MAX_COUNT_PER_PAGE;
    if(start>_friend_list.size()){
        return {};
    }
    if(end>_friend_list.size()){
        return std::vector<std::shared_ptr<FriendInfo>>(_friend_list.begin()+start,_friend_list.end());
    }
    return std::vector<std::shared_ptr<FriendInfo>>(_friend_list.begin()+start,_friend_list.begin()+end);
}

bool UserMgr::isLoadConFin(){
    return (_contact_loaded>=_friend_list.size());
}

void UserMgr::UpdateConLoadedCount(){
    int start=_contact_loaded;
    int end=std::min(_contact_loaded+MAX_COUNT_PER_PAGE,static_cast<int>(_friend_list.size()));
    _contact_loaded=end;
}



void UserMgr::SetLastChatThreadId(int last_id)
{
    last_thread_id=last_id;
}

void UserMgr::SetFriendThreadId(std::shared_ptr<ChatThreadInfo> info, int friend_id)
{
    _thread_map[info->_thread_id]=info;
    _uid_to_thread_id[friend_id]=info->_thread_id;
}

int UserMgr::GetFriendThreadId(int friend_id)
{

    auto iter=_uid_to_thread_id.find(friend_id);
    if(iter==_uid_to_thread_id.end()){
        return -1;
    }
    return iter.value();
}

std::shared_ptr<ChatThreadData> UserMgr::GetThreadDataByUid(int friend_id)
{
    auto thread=GetFriendThreadId(friend_id);
    return GetThreadData(thread);
}

std::shared_ptr<ChatThreadData> UserMgr::GetThreadData(int thread_id)
{
    auto iter=_thread_data_map.find(thread_id);
    if(iter==_thread_data_map.end()){
        return nullptr;
    }
    return iter.value();
}

void UserMgr::AddThreadData( std::shared_ptr<ChatThreadData> data,int friend_id){
    auto thread_id=UserMgr::getInstance()->GetFriendThreadId(friend_id);
    if(thread_id<=0){
        return;
    }
    _thread_data_map[thread_id]=data;
}
void UserMgr::AddThreadData(int thread_id, std::shared_ptr<ChatThreadData> data)
{
    _thread_data_map[thread_id]=data;
}

bool UserMgr::HasThreadHistory(int thread_id)
{
    auto iter=_thread_data_map.find(thread_id);
    if(iter==_thread_data_map.end())return false;
    return true;
}

void UserMgr::AppendMsgToThread(int thread_id, std::shared_ptr<ChatDataBase> msg)
{
    if (thread_id <= 0 || !msg) return;
    auto data = GetThreadData(thread_id);
    if(!data){
        int other_uid=(msg->GetSendUid()==_uid)?0:msg->GetSendUid();
        data = std::make_shared<ChatThreadData>(other_uid, thread_id, msg->GetMsgId());
        _thread_data_map[thread_id] = data;
    }
    data->AddMsg(msg);
}

void UserMgr::AppendHistoryBatch(int thread_id, const std::vector<std::shared_ptr<ChatDataBase>>& msgs)
{
    if (thread_id <= 0 || msgs.empty()) return;

    auto data = GetThreadData(thread_id);
    if (!data) {
        data = std::make_shared<ChatThreadData>(0, thread_id, 0);
        _thread_data_map[thread_id] = data;
    }

    for (const auto& msg : msgs) {
        if (msg) {
            data->AppendMsg(msg->GetMsgId(), msg);
        }
    }
}

std::shared_ptr<ChatThreadData> UserMgr::GetCurrLoadThread()
{

}
