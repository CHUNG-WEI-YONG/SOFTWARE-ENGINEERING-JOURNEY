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
    std::shared_ptr<FriendInfo> getFriendByName(QString name);
    void AppendFriendList(QJsonArray array);
    std::vector<std::shared_ptr<FriendInfo>> GetChatListPerPage();
    bool isLoadChatFin();
    void UpdateChatLoadedCount();
    std::vector<std::shared_ptr<FriendInfo>>GetConListPerPage();
    bool isLoadConFin();
    void UpdateConLoadedCount();
    bool hasHistoryCache(int uid);
    void AppendHistoryMsg(int friend_uid, const ChatMsg& msg) {
        _history_cache[friend_uid].append(msg);
    }

    void PrependHistoryBatch(int friend_uid, const QList<ChatMsg>& msgs) {
        auto& list = _history_cache[friend_uid];
        for (int i = msgs.size() - 1; i >= 0; --i) {
            list.prepend(msgs[i]);
        }
    }

    QList<ChatMsg> GetHistoryMsgs(int friend_uid) const {
        return _history_cache.value(friend_uid);
    }

private:
    QString _token;
    QString _logoPath;
    std::vector<std::shared_ptr<ApplyInfo>> _applications;
    std::shared_ptr<UserInfo> _user_info;
    std::map<int,std::shared_ptr<FriendInfo>> _friend_map;
    std::vector<std::shared_ptr<FriendInfo>>_friend_list;
    int _uid;
    int _chat_loaded;
    int _contact_loaded;
    QMap<int, QList<ChatMsg>> _history_cache;



};



#endif // USERMGR_H
