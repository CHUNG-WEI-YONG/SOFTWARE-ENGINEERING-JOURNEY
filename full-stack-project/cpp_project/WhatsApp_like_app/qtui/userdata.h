#ifndef USERDATA_H
#define USERDATA_H
#include <QString>
#include <memory>
#include <QVariantMap>
#include <QJsonObject>
#include "global.h"

class ChatDataBase;


struct FileToken {
    qint64 from_uid{ 0 };
    qint64 to_uid{ 0 };
    qint64 filesz{ 0 };       // 文件大小，必须用 qint64 防止大文件溢出
    quint16 port{ 0 };        // FileServer TCP 数据端口
    QString token;            // 鉴权凭证 UUID
    QString filename;         // 文件名
    QString host;             // FileServer IP 或域名 (合并之前的 ip 与 host)
    QString md5;              // 文件 MD5，用于秒传与校验

    // 默认构造
    FileToken() = default;

    // 从 QJsonObject 反序列化（解析服务器回包）
    static FileToken fromJson(const QJsonObject& json) {
        FileToken ft;
        ft.from_uid = json["from_uid"].toVariant().toLongLong();
        ft.to_uid   = json["to_uid"].toVariant().toLongLong();
        ft.filesz   = json["filesz"].toVariant().toLongLong();
        ft.port     = static_cast<quint16>(json["port"].toInt());
        ft.token    = json["token"].toString();
        ft.filename = json["filename"].toString();
        // 兼容处理：若服务端返回的是 "ip"，优先读取 "ip"，否则读 "host"
        ft.host     = json.contains("ip") ? json["ip"].toString() : json["host"].toString();
        ft.md5      = json["md5"].toString();
        return ft;
    }

    // 转为 QJsonObject（方便打印日志或存储）
    QJsonObject toJsonObject() const {
        QJsonObject json;
        json["from_uid"] = from_uid;
        json["to_uid"]   = to_uid;
        json["filesz"]   = filesz;
        json["port"]     = port;
        json["token"]    = token;
        json["filename"] = filename;
        json["host"]     = host;
        json["md5"]      = md5;
        return json;
    }

    // 转为格式化 JSON 字符串（排查排错 debug 打印极方便）
    QString toJsonString() const {
        return QJsonDocument(toJsonObject()).toJson(QJsonDocument::Compact);
    }
};


struct ChatThreadInfo {
    int64_t     _thread_id{0};
    QString _type;          // "private" 或 "group"

    // 私聊特有字段
    int64_t     _user1_id{0};
    int64_t     _user2_id{0};

    // 群聊特有字段
    std::string _group_name;
    int64_t     _owner_id{0};
    std::string _avatar_url;

    ChatThreadInfo() = default;

    ChatThreadInfo(int64_t thread_id, const QString& type, int64_t u1 = 0, int64_t u2 = 0)
        : _thread_id(thread_id), _type(type), _user1_id(u1), _user2_id(u2), _owner_id(0) {}

    // 辅助方法：快速获取私聊中的对方 UID
    int64_t GetPeerUid(int64_t my_uid) const {
        if (_type != "private") return 0;
        return (_user1_id == my_uid) ? _user2_id : _user1_id;
    }

    bool IsGroup() const {
        return _type == "group";
    }
};




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

struct AuthInfo {
    int _uid;
    QString _name;
    QString _nick;
    QString _icon;
    int _sex;
    int _thread_id;
    std::vector<std::shared_ptr<ChatDataBase>> _chat_datas;

    AuthInfo(int uid, QString name, QString nick, QString icon, int sex, int thread_id = 0)
        : _uid(uid)
        , _name(name)
        , _nick(nick)
        , _icon(icon)
        , _sex(sex)
        , _thread_id(thread_id)
    {}
};

struct AuthRsp{
    int _uid;
    QString _name;
    QString _nick;
    QString _icon;
    int _sex;
    int _thread_id;
    std::vector<std::shared_ptr<ChatDataBase>> _chat_datas;

    AuthRsp(int uid, QString name, QString nick, QString icon, int sex, int thread_id = 0)
        : _uid(uid)
        , _name(name)
        , _nick(nick)
        , _icon(icon)
        , _sex(sex)
        , _thread_id(thread_id)
    {}
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

    UserInfo()
        : _uid(0), _name(""), _nick(""), _desc(""), _sex(0), _icon(""),_last_msg("") {}

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


class ChatDataBase {
public:
    ChatDataBase(int msg_id, int thread_id, ChatFormType form_type, ChatMsgType msg_type,
                 const QString& content, int send_uid, int status, const QString& chat_time = "")
        : _msg_id(msg_id)
        , _thread_id(thread_id)
        , _form_type(form_type)
        , _msg_type(msg_type)
        , _content(content)
        , _send_uid(send_uid)
        , _status(status)
        , _chat_time(chat_time) {}

    virtual ~ChatDataBase() = default;

    // 基础属性 Getter
    int GetMsgId() const { return _msg_id; }
    int GetThreadId() const { return _thread_id; }
    int GetSendUid() const { return _send_uid; }
    int GetStatus() const { return _status; }
    QString GetContent() const { return _content; }
    QString GetMsgContent() const { return _content; } // 🎯 修正：内联实现，防止链接错误
    QString GetChatTime() const { return _chat_time; }
    ChatMsgType GetMsgType() const { return _msg_type; }
    ChatFormType GetFormType() const { return _form_type; }

    // 唯一临时标识 Getter / Setter
    void SetUniqueId(const QString& unique_id) { _unique_id = unique_id; }
    QString GetUniqueId() const { return _unique_id; } // 🎯 修正：移除冲突的 int 返回版本

    // 状态 Setter
    void SetMsgId(int msg_id) { _msg_id = msg_id; }
    void SetStatus(int status) { _status = status; }
    void SetChatTime(const QString& chat_time) { _chat_time = chat_time; }

    // 🎯 核心虚函数：供 QML 直接渲染统一的数据字典
    virtual QVariantMap toVariantMap(int my_uid) const {
        QVariantMap map;
        map["msg_id"]   = _msg_id;
        map["sender"]   = (_send_uid == my_uid) ? "me" : "other";
        map["timeStr"]  = _chat_time;
        map["status"]   = _status; // 0: sending, 1: success, 2: failed
        return map;
    }

protected:
    QString _unique_id{""};
    int _msg_id{0};
    int _thread_id{0};
    ChatFormType _form_type{ChatFormType::PRIVATE};
    ChatMsgType _msg_type{ChatMsgType::TEXT};
    QString _content{""};
    int _send_uid{0};
    int _status{1};
    QString _chat_time{""};
};

// ──► 2. 文本消息派生类 ◄──
class TextChatData : public ChatDataBase {
public:
    TextChatData(int msg_id, int thread_id, ChatFormType form_type,
                 const QString& content, int send_uid, int status = 1, const QString& chat_time = "")
        : ChatDataBase(msg_id, thread_id, form_type, ChatMsgType::TEXT, content, send_uid, status, chat_time) {}

    // 兼容显式指定 msg_type 的构造
    TextChatData(int msg_id, int thread_id, ChatFormType form_type, ChatMsgType msg_type,
                 const QString& content, int send_uid, int status = 1, const QString& chat_time = "")
        : ChatDataBase(msg_id, thread_id, form_type, msg_type, content, send_uid, status, chat_time) {}

    QVariantMap toVariantMap(int my_uid) const override {
        QVariantMap map = ChatDataBase::toVariantMap(my_uid);
        map["type"]    = "text";
        map["content"] = _content;
        return map;
    }
};

// ──► 3. 文件消息派生类 ◄──
class FileChatData : public ChatDataBase {
public:
    FileChatData(int msg_id, int thread_id, ChatFormType form_type, const QString& filename,
                 qint64 filesz, const QString& token, const QString& md5,
                 int send_uid, int status = 1, const QString& chat_time = "")
        : ChatDataBase(msg_id, thread_id, form_type, ChatMsgType::FILE, filename, send_uid, status, chat_time)
        , _filename(filename)
        , _filesz(filesz)
        , _token(token)
        , _md5(md5) {
        _fileszStr = formatFileSize(_filesz);
    }

    QVariantMap toVariantMap(int my_uid) const override {
        QVariantMap map = ChatDataBase::toVariantMap(my_uid);
        map["type"]     = "file";
        map["content"]  = _filename;
        map["fileSize"] = _fileszStr;
        map["fileUrl"]  = _token;
        return map;
    }

    static QString formatFileSize(qint64 bytes) {
        if (bytes <= 0) return "0 B";
        double sizeMb = bytes / (1024.0 * 1024.0);
        if (sizeMb >= 1.0) return QString::number(sizeMb, 'f', 2) + " MB";
        double sizeKb = bytes / 1024.0;
        if (sizeKb >= 1.0) return QString::number(sizeKb, 'f', 1) + " KB";
        return QString::number(bytes) + " B";
    }

    QString GetFileName() const { return _filename; }
    qint64 GetFileSize() const { return _filesz; }
    QString GetFileToken() const { return _token; }
    QString GetMd5() const { return _md5; }

private:
    QString _filename;
    qint64  _filesz{0};
    QString _fileszStr{""};
    QString _token{""};
    QString _md5{""};
};

// ──► 4. 会话上下文聚合根 ◄──
class ChatThreadData {
public:
    ChatThreadData(int other_id, int thread_id, int last_msg_id = 0)
        : _other_id(other_id)
        , _last_msg_id(last_msg_id)
        , _thread_id(thread_id)
        , _last_msg("")
        , _group_name("") {}

    // 1. 添加一条已确认的消息（拉取历史记录回填或收到对端消息时使用）
    void AddMsg(std::shared_ptr<ChatDataBase> msg) {
        if (!msg) return;
        int msg_id = msg->GetMsgId();
        _msg_map.insert(msg_id, msg);

        QString preview = msg->GetContent();
        if (msg->GetMsgType() == ChatMsgType::FILE) {
            preview = QString("[File] %1").arg(msg->GetContent());
        }
        _last_msg = preview;

        if (msg_id > _last_msg_id) {
            _last_msg_id = msg_id;
        }
    }

    // 2. 收到服务端 ACK 后，完成临时 ID 替换、状态更新与队列迁移
    void ConfirmMsg(const QString& unique_id, int real_msg_id) {
        if (unique_id.isEmpty()) return;

        auto iter = _msg_unrsp_map.find(unique_id);
        if (iter == _msg_unrsp_map.end()) {
            return;
        }

        auto msg = iter.value();
        if (msg) {
            // (1) 如果发送前把它塞进过 _msg_map，先清除旧的临时 Key 节点
            int temp_msg_id = msg->GetMsgId();
            _msg_map.remove(temp_msg_id);

            // (2) 更改内部属性
            msg->SetMsgId(real_msg_id);
            msg->SetStatus(1); // 1: 发送成功/已送达

            // (3) 用真实的 MySQL 自增 ID 重新插入 _msg_map
            _msg_map.insert(real_msg_id, msg);

            // (4) 维护最大游标与最新预览
            if (real_msg_id > _last_msg_id) {
                _last_msg_id = real_msg_id;
            }
            QString preview = msg->GetContent();
            if (msg->GetMsgType() == ChatMsgType::FILE) {
                preview = QString("[File] %1").arg(msg->GetContent());
            }
            _last_msg = preview;
        }

        // (5) 从未回执字典移除
        _msg_unrsp_map.erase(iter);
    }

    // 3. 追加未收到服务端回执的消息（挂载菊花/发送中状态）
    void AppendUnRspMsg(const QString& unique_id, std::shared_ptr<ChatDataBase> base_msg) {
        if (unique_id.isEmpty() || !base_msg) return;
        base_msg->SetUniqueId(unique_id);
        _msg_unrsp_map.insert(unique_id, base_msg);

        // 同时放进 _msg_map 供本地 UI 立即渲染（带有转圈状态）
        _msg_map.insert(base_msg->GetMsgId(), base_msg);

        QString preview = base_msg->GetContent();
        if (base_msg->GetMsgType() == ChatMsgType::FILE) {
            preview = QString("[File] %1").arg(base_msg->GetContent());
        }
        _last_msg = preview;
    }

    // 4. 批量追加历史消息
    void AppendMsg(int msg_id, std::shared_ptr<ChatDataBase> base_msg) {
        if (!base_msg) return;
        _msg_map.insert(msg_id, base_msg);
        if (msg_id >= _last_msg_id) {
            _last_msg_id = msg_id;
            QString preview = base_msg->GetContent();
            if (base_msg->GetMsgType() == ChatMsgType::FILE) {
                preview = QString("[File] %1").arg(base_msg->GetContent());
            }
            _last_msg = preview;
        }
    }

    // 查找待响应消息（便于发送失败时置为 -1）
    std::shared_ptr<ChatDataBase> GetUnRspMsg(const QString& unique_id) {
        auto iter = _msg_unrsp_map.find(unique_id);
        return (iter != _msg_unrsp_map.end()) ? iter.value() : nullptr;
    }

    void SetLastMsgId(int msg_id) { _last_msg_id = msg_id; }
    int GetLastMsgId() const { return _last_msg_id; }
    void SetOtherId(int other_id) { _other_id = other_id; }
    int GetOtherId() const { return _other_id; }
    int GetThreadId() const { return _thread_id; }
    QString GetLastMsg() const { return _last_msg; }
    void SetLastMsg(const QString& last_msg) { _last_msg = last_msg; }
    QString GetGroupName() const { return _group_name; }
    void SetGroupName(const QString& group_name) { _group_name = group_name; }

    QMap<int, std::shared_ptr<ChatDataBase>> GetMsgMap() const { return _msg_map; }
    QMap<int, std::shared_ptr<ChatDataBase>>& GetMsgMapRef() { return _msg_map; }
    const QMap<int, std::shared_ptr<ChatDataBase>>& GetMsgMapRef() const { return _msg_map; }
    QMap<QString, std::shared_ptr<ChatDataBase>>& GetMsgUnRspRef() { return _msg_unrsp_map; }
    const QMap<QString, std::shared_ptr<ChatDataBase>>& GetMsgUnRspRef() const { return _msg_unrsp_map; }

    void SetGroupMembers(const std::vector<int>& members) { _group_members = members; }
    const std::vector<int>& GetGroupMembers() const { return _group_members; }

private:
    int _other_id{0};
    int _last_msg_id{0};
    int _thread_id{0};
    QString _last_msg{""};
    std::vector<int> _group_members;
    QString _group_name{""};

    QMap<int, std::shared_ptr<ChatDataBase>> _msg_map;
    QMap<QString, std::shared_ptr<ChatDataBase>> _msg_unrsp_map;
};
#endif // USERDATA_H
