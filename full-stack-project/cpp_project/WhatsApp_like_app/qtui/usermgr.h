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
    // 基础用户信息
    void SetUid(int uid);
    int GetUid() const;
    void SetToken(QString token);
    void Setlogo(QString path);
    QString returnLogo() const;
    QString returnName() const;
    QString GetName() const;
    void SetUserInfo(std::shared_ptr<UserInfo> user);
    std::shared_ptr<UserInfo> GetUserInfo();
    void Reset();

    // 好友与申请列表管理
    bool has_added(int uid);
    void add_apply(std::shared_ptr<ApplyInfo> apply);
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
    std::vector<std::shared_ptr<FriendInfo>> GetConListPerPage();
    bool isLoadConFin();
    void UpdateConLoadedCount();

    // ──► 🎯 核心改造 1：会话关系管理 (Thread ID) ◄──
    void SetLastChatThreadId(int last_id);
    int GetLastChatThreadId() const;
    void SetFriendThreadId(std::shared_ptr<ChatThreadInfo> info, int friend_id);
    int GetFriendThreadId(int friend_id);
    std::shared_ptr<ChatThreadData> GetThreadDataByUid(int friend_id);
    void AddThreadData(std::shared_ptr<ChatThreadData> data,int friend_id);

    // ──► 🎯 核心改造 2：接入 ChatThreadData 替代旧的 _history_cache ◄──
    // 获取或按需创建指定会话上下文
    std::shared_ptr<ChatThreadData> GetThreadData(int thread_id);
    void AddThreadData(int thread_id, std::shared_ptr<ChatThreadData> data);
    bool HasThreadHistory(int thread_id);

    // 追加单条已发送/接收的消息
    void AppendMsgToThread(int thread_id, std::shared_ptr<ChatDataBase> msg);

    // 批量追加拉取回来的历史消息
    void AppendHistoryBatch(int thread_id, const std::vector<std::shared_ptr<ChatDataBase>>& msgs);





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
    QMap<int,std::shared_ptr<ChatThreadData>>_thread_data_map;
    int last_thread_id{0};
    QMap<int,std::shared_ptr<ChatThreadInfo>> _thread_map;
    QMap<int , int >_uid_to_thread_id;



};



#endif // USERMGR_H
