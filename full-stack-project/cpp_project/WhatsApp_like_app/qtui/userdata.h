#ifndef USERDATA_H
#define USERDATA_H
#include <QString>
#include <memory>

// class UserData
// {
// public:
//     UserData();
// };



struct SearchInfo {
    int _uid;
    QString _name;
    QString _nick;
    QString _desc;
    int _sex;
    QString _icon;

    SearchInfo();
    // 统一顺序：(uid, name, nick, desc, sex, icon)
    SearchInfo(int uid, QString name, QString nick, QString desc, int sex, QString icon)
        : _uid(uid), _name(name), _nick(nick), _desc(desc), _sex(sex), _icon(icon) {}
};

struct AuthInfo{
    int _uid;
    QString _name;
    int _sex;
    int thread_id;
    QString _nick;
    QString _icon;

    AuthInfo(int uid,QString name,QString nick,QString icon,int sex):_uid(uid),_name(name),
        _sex(sex),_nick(nick),thread_id(0),_icon(icon){};
};

struct AuthRsp{
    int _uid;
    QString _name;
    int _sex;
    int thread_id;
    QString _nick;
    QString _icon;

    AuthRsp(int uid,QString name,QString nick,QString icon,int sex):_uid(uid),_name(name),
        _sex(sex),_nick(nick),thread_id(0),_icon(icon){};
};
struct FriendInfo {
    // Default constructor
    FriendInfo() = default;

    // Full constructor
    FriendInfo(int uid, QString name, QString nick, QString icon,
               int sex, QString desc, QString back, QString last_msg = "")
        : _uid(uid)
        , _name(name)
        , _nick(nick)
        , _icon(icon)
        , _sex(sex)
        , _desc(desc)
        , _back(back)
        , _last_msg(last_msg) {}

    // Constructor from AuthInfo pointer
    FriendInfo(std::shared_ptr<AuthInfo> auth_info)
        : _uid(auth_info->_uid)
        , _name(auth_info->_name)
        , _nick(auth_info->_nick)
        , _icon(auth_info->_icon)
        , _sex(auth_info->_sex) {}

    // Constructor from AuthRsp pointer
    FriendInfo(std::shared_ptr<AuthRsp> auth_rsp)
        : _uid(auth_rsp->_uid)
        , _name(auth_rsp->_name)
        , _nick(auth_rsp->_nick)
        , _icon(auth_rsp->_icon)
        , _sex(auth_rsp->_sex) {}

    // Member Variables
    int _uid{0};
    QString _name;
    QString _nick;
    QString _icon;
    int _sex{0};
    QString _desc;
    QString _back;
    QString _last_msg;
};

struct UserInfo {
    int _uid;
    QString _name;
    QString _nick;
    QString _desc;
    int _sex;
    QString _icon;
    QString _last_msg;

    // 保持与 SearchInfo 一致
    UserInfo(int uid, QString name, QString nick, QString desc, int sex, QString icon,QString msg)
        : _uid(uid), _name(name), _nick(nick), _desc(desc), _sex(sex), _icon(icon),_last_msg(msg) {}

    UserInfo(std::shared_ptr<SearchInfo> search_info)
        : _uid(search_info->_uid), _name(search_info->_name), _nick(search_info->_nick),
        _desc(search_info->_desc), _sex(search_info->_sex), _icon(search_info->_icon),_last_msg(""){}

    UserInfo(std::shared_ptr<AuthInfo> auth): _uid(auth->_uid),_name(auth->_name),_nick(auth->_nick),
        _icon(auth->_icon),_sex(auth->_sex),_desc(""),_last_msg(""){}

    UserInfo(std::shared_ptr<AuthRsp> auth): _uid(auth->_uid),_name(auth->_name),_nick(auth->_nick),
        _icon(auth->_icon),_sex(auth->_sex),_desc(""),_last_msg(""){};
    UserInfo(int uid, QString name, QString icon):
        _uid(uid), _name(name), _icon(icon),_nick(_name),
        _sex(0),_desc(""),_last_msg(""){
    }

    UserInfo(std::shared_ptr<FriendInfo> friend_info)
        : _uid(friend_info->_uid)
        , _name(friend_info->_name)
        , _nick(friend_info->_nick)
        , _icon(friend_info->_icon)
        , _sex(friend_info->_sex)
        , _last_msg("") {}


};


class AddFriendApply {
public:
    AddFriendApply(int from_uid, QString name, QString desc,
                   QString icon, QString nick, int sex);
    int _from_uid;
    QString _name;
    QString _desc;
    QString _icon;
    QString _nick;
    int     _sex;
};

struct ApplyInfo {
    ApplyInfo(int uid, QString name, QString desc,
              QString icon, QString nick, int sex, int status)
        :_uid(uid),_name(name),_desc(desc),
        _icon(icon),_nick(nick),_sex(sex),_status(status){}

    ApplyInfo(std::shared_ptr<AddFriendApply> addinfo)
        :_uid(addinfo->_from_uid),_name(addinfo->_name),
        _desc(addinfo->_desc),_icon(addinfo->_icon),
        _nick(addinfo->_nick),_sex(addinfo->_sex),
        _status(0)
    {}
    void SetIcon(QString head){
        _icon = head;
    }
    int _uid;
    QString _name;
    QString _desc;
    QString _icon;
    QString _nick;
    int _sex;
    int _status;
};


#endif // USERDATA_H
