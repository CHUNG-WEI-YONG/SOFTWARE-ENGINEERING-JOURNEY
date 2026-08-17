#include "usermgr.h"
#include <QJsonArray>

UserMgr::UserMgr() {}
UserMgr::~UserMgr()
{

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

QString UserMgr::returnName()
{
    return _user_info->_name;
}

void UserMgr::Setlogo(QString path)
{
    _logoPath=path;
}

QString UserMgr::returnLogo()
{
    return _logoPath;
}

int UserMgr::GetUid()
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

QString UserMgr::GetName(){
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
    auto iter=_friend_list.find(uid);
    if(iter==_friend_list.end()){
        return false;
    }
    return true;
}

void UserMgr::AddFriend(std::shared_ptr<AuthInfo>auth)
{
    FriendInfo new_friend(auth);
    _friend_list[auth->_uid]=std::make_shared<FriendInfo>(new_friend);
}

void UserMgr::AddFriend(std::shared_ptr<AuthRsp>auth)
{
    FriendInfo new_friend(auth);
    _friend_list[auth->_uid]=std::make_shared<FriendInfo>(new_friend);
}

std::shared_ptr<FriendInfo> UserMgr::getFriend(int uid)
{
    auto iter=_friend_list.find(uid);
    if(iter==_friend_list.end()){
        return nullptr;
    }
    else{
        return iter->second;
    }
}
