#include "usermgr.h"

UserMgr::UserMgr() {}
UserMgr::~UserMgr()
{

}

void UserMgr::SetName(QString name)
{
    _name = name;
}

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
    return _name;
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
    return _uid;
}

bool UserMgr::has_added(int uid)
{
    for(auto apply:_applications){
        if(apply->_from_uid==uid){
            return true;
        }
    }
    return false;
}

void UserMgr::add_apply(std::shared_ptr<AddFriendApply> apply)
{
    _applications.push_back(apply);
}

QString UserMgr::GetName(){
    return _name;
}
