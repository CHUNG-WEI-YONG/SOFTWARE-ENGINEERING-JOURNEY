#ifndef USERMGR_H
#define USERMGR_H
#include "global.h"
#include "Singleton.h"
#include "userdata.h"
class UserMgr:public QObject ,public Singleton<UserMgr>,public std::enable_shared_from_this<UserMgr>
{
    Q_OBJECT
public:
    UserMgr();
    ~UserMgr();
    friend Singleton<UserMgr>;
    //void SetName(QString name);
    void SetUid(int uid);
    void SetToken(QString token);
    QString returnName();
    void Setlogo(QString path);
    QString returnLogo();
    int GetUid();
    bool has_added(int uid);
    void add_apply(std::shared_ptr<ApplyInfo> apply);
    QString GetName();
    void SetUserInfo(std::shared_ptr<UserInfo> user);
    void AppendApplyList(QJsonArray array);
    bool CheckFriendById(int uid);
    void AddFriend(std::shared_ptr<AuthInfo>);
    void AddFriend(std::shared_ptr<AuthRsp>);
    std::shared_ptr<FriendInfo> getFriend(int uid);

private:
    QString _token;
    QString _logoPath;
    std::vector<std::shared_ptr<ApplyInfo>> _applications;
    std::shared_ptr<UserInfo> _user_info;
    std::map<int,std::shared_ptr<FriendInfo>> _friend_list;
    int _uid;



};



#endif // USERMGR_H
