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
    void SetName(QString name);
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

private:
    int _uid;
    QString email;
    QString _name;
    QString _token;
    QString _logoPath;
    std::vector<std::shared_ptr<ApplyInfo>> _applications;
    std::shared_ptr<UserInfo> _user_info;



};



#endif // USERMGR_H
