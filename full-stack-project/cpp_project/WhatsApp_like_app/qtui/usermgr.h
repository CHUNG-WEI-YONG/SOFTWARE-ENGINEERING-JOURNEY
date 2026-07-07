#ifndef USERMGR_H
#define USERMGR_H
#include "global.h"
#include "Singleton.h"
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
private:
    int _uid;
    QString email;
    QString _name;
    QString _token;



};

#endif // USERMGR_H
