#ifndef USERDATA_H
#define USERDATA_H
#include <QString>
#include <memory>
#include <QVariantMap>
#include <QJsonObject>


// class UserData
// {
// public:
//     UserData();
// };

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

struct ChatMsg {
    int msg_id{0};
    QString content{""};
    int from_uid{0};
    int to_uid{0};
    QString type{"text"};       // "text" 或 "file"
    QString timeStr{""};

    // ──► 📁 文件专属元数据字段 ◄──
    QString filename{""};       // 文件名 (如 "resume.pdf")
    qint64 filesz{0};           // 文件字节数
    QString fileszStr{""};      // 格式化后的大小 (如 "12.4 MB")
    QString token{""};          // 供接收方下载使用的 Token / 凭证
    QString md5{""};            // 校验和

    // 格式化文件大小辅助函数
    static QString formatFileSize(qint64 bytes) {
        if (bytes <= 0) return "0 B";
        double sizeMb = bytes / (1024.0 * 1024.0);
        if (sizeMb >= 1.0) {
            return QString::number(sizeMb, 'f', 2) + " MB";
        }
        double sizeKb = bytes / 1024.0;
        if (sizeKb >= 1.0) {
            return QString::number(sizeKb, 'f', 1) + " KB";
        }
        return QString::number(bytes) + " B";
    }

    // 序列化给 QML 消费（无论是历史记录还是动态切换都会用到）
    QVariantMap toVariantMap(int my_uid) const {
        QVariantMap map;
        map["msg_id"]   = msg_id;
        map["sender"]   = (from_uid == my_uid) ? "me" : "other";
        map["type"]     = type;
        map["content"]  = (type == "file" && !filename.isEmpty()) ? filename : content;
        map["fileSize"] = fileszStr;
        map["fileUrl"]  = token;     // QML 点击下载时，把 token 传递给 C++ 接口
        map["timeStr"]  = timeStr;
        return map;
    }

    // 从通用扁平 QJsonObject 解析
    static ChatMsg fromJson(const QJsonObject& obj) {
        ChatMsg msg;
        msg.msg_id   = obj["msg_id"].toInt();
        msg.from_uid = obj["from_uid"].toInt();
        msg.to_uid   = obj["to_uid"].toInt();
        msg.type     = obj["type"].toString("text");
        msg.content  = obj["content"].toString();
        msg.timeStr  = obj["time"].toString();
        if (msg.timeStr.isEmpty()) {
            msg.timeStr = QTime::currentTime().toString("hh:mm AP");
        }

        // 解析可能存在的文件属性
        msg.parseFileAttributes(obj);
        return msg;
    }

    // 从服务端接收到的消息元素与上下文解析
    static ChatMsg fromJsonObject(const QJsonObject& itemObj, int from_uid, int to_uid) {
        ChatMsg msg;
        msg.msg_id   = itemObj["msg_id"].toInt();
        msg.content  = itemObj["content"].toString();
        msg.from_uid = from_uid;
        msg.to_uid   = to_uid;
        msg.type     = itemObj["type"].toString("text");
        msg.timeStr  = itemObj["time"].toString();
        if (msg.timeStr.isEmpty()) {
            msg.timeStr = QTime::currentTime().toString("hh:mm AP");
        }

        // 解析文件属性（优先读外层字段，若没有则尝试解析 content 内部的 JSON 字符串）
        msg.parseFileAttributes(itemObj);
        return msg;
    }

private:
    // 统一解析文件相关的字段
    void parseFileAttributes(const QJsonObject& obj) {
        if (type == "file") {
            // 1. 如果外层直接携带了 filename、filesz、token
            if (obj.contains("filename") || obj.contains("token")) {
                filename = obj["filename"].toString();
                filesz   = obj["filesz"].toVariant().toLongLong();
                token    = obj["token"].toString();
                md5      = obj["md5"].toString();
            }
            // 2. 如果外层没有，但 content 包含的是文件 JSON 字符串（gRPC 转发场景）
            else if (!content.isEmpty() && content.startsWith('{')) {
                QJsonDocument doc = QJsonDocument::fromJson(content.toUtf8());
                if (!doc.isNull() && doc.isObject()) {
                    QJsonObject subObj = doc.object();
                    filename = subObj["filename"].toString();
                    filesz   = subObj["filesz"].toVariant().toLongLong();
                    token    = subObj["token"].toString();
                    md5      = subObj["md5"].toString();
                }
            }

            // 兜底处理：若文件名为空则拿 content 替代
            if (filename.isEmpty()) {
                filename = content;
            }
            fileszStr = formatFileSize(filesz);
        }
    }
};


class ChatData{
public:
    int _uid{0};
    QString nick{""};
    QString name{""};
    QString icon{""};

    bool _is_online{false};      // 是否在线
    QString _last_time{""};      // 最后消息时间 (如 "10:30 AM")
    QString _last_msg{""};       // 最后一条消息预览文本
    int _unread_count{0};

    QVector<ChatMsg> _msg;

    ChatData()=default;
    ChatData(int uid,QString name,QString icon):_uid(uid),name(name),icon(icon){};
    QVariantList getHistoryVariantList(int my_uid) const {
        QVariantList list;
        for (const auto& msg : _msg) {
            list.append(msg.toVariantMap(my_uid)); // 👈 直接复用你写的 toVariantMap
        }
        return list;
    }

    void AppendMsg(const ChatMsg& msg){
        _msg.append(msg);
        _last_time=msg.timeStr;
        _last_msg=msg.content;
    }


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


#endif // USERDATA_H
