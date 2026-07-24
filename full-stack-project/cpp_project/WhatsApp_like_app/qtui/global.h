#ifndef GLOBAL_H
#define GLOBAL_H
#include <QWidget>
#include <functional>
#include "QStyle"
#include <QRegularExpression>
#include <memory.h>
#include <iostream>
#include <mutex>
#include <QByteArray>
#include <QNetworkReply>
#include <QJsonObject>
#include <QDir>
#include <QSettings>
#include <QString>
#include <QVariantMap>
#include <QDateTime>

enum class ReqId{
    ID_GET_VERIFY_CODE=1001,//code
    ID_REG_USER=1002,//user signin
    ID_RESET_PWD=1003,
    ID_LOGIN_USER=1004,
    ID_CHAT_LOGIN=1005,
    ID_CHAT_LOGIN_RSP=1006,

};

enum class Modules{
    REGISTERMOD=0,
    RESETMOD=1,
    LOGINMOD=2,
};

enum class ErrorCode{
    SUCCESS=0,
    Err_JSON=1,
    Err_NETWORK=2,
};

enum TipErr{
    TIP_SUCCESS = 0,
    TIP_EMAIL_ERR = 1,
    TIP_PWD_ERR = 2,
    TIP_CONFIRM_ERR = 3,
    TIP_PWD_CONFIRM = 4,
    TIP_VARIFY_ERR = 5,
    TIP_USER_ERR = 6
};

enum ClickLbState{
    Normal=0,
    Selected=1,
};

enum ChatUIMode{
    SearchMode,
    ContactMode,
    ChatMode,
};

enum ListItemType{
    ChatUserItem,
    ContactUserItem,
    SearchUserItem,
    Add_User_Tip_Item,
    InvalidItem,
    Group_Tip_Item,
    LineItem,
    Apply_Friend_Item
};

struct Serverinfo{
    int uid;
    QString host;
    QString port;
    QString token;
};

struct MessageEntity {
    QString msgId;      // 消息唯一UUID（防止重发或去重）
    QString fromUid;    // 发送者ID
    QString toUid;      // 接收者ID
    QString sender;     // 对齐QML标签："me" 或 "other"
    QString type;       // "text" 或 "image" 或 "file"
    QString content;    // 文字内容或物理路径
    QString timeStr;    // 格式化后的时间："10:45 AM"

    // ──► 🎯 核心黑科技：一键把 C++ 结构体熔炼成 QML 认识的盲包 ◄──
    QVariantMap toQmlItem() const {
        QVariantMap map;
        map["msgId"] = msgId;
        map["sender"] = sender;
        map["type"] = type;
        map["content"] = content;
        map["timeStr"] = timeStr;
        return map;
    }
};

const int MIN_APPLY_LABEL_ED_LEN = 40;

const QString add_prefix = "Add Tags ";

const int  tip_offset = 5;


const std::vector<QString>  strs ={"hello world !",
                                   "nice to meet u",
                                   "New year，new life",
                                   "You have to love yourself",
                                   "My love is written in the wind ever since the whole world is you"};

const std::vector<QString> heads = {
    ":/rc/chat_picture/head_1.jpg",
    ":/rc/chat_picture/head_2.jpg",
    ":/rc/chat_picture/head_3.jpg",
    ":/rc/chat_picture/head_4.jpg",
    ":/rc/chat_picture/head_5.jpg"
};

const std::vector<QString> names = {
    "HanMeiMei",
    "Lily",
    "Ben",
    "Androw",
    "Max",
    "Summer",
    "Candy",
    "Hunter"
};

const int CHAT_COUNT_PER_PAGE = 13;

enum MsgStatus{
    UN_READ = 0,  //对方未读
    SEND_FAILED = 1,  //发送失败
    READED = 2  //对方已读
};

//聊天形式，私聊和群聊
enum class ChatFormType {
    PRIVATE = 0,
    GROUP = 1
};

//聊天消息类型，文本，图片，文件等
enum class ChatMsgType {
    TEXT = 0,
    PIC = 1,
    FILE = 2
};


extern std::function <void(QWidget*)>repolish;
extern QString gate_url_prefix;
extern bool ValidatePasswordStyle(const std::string& password, std::string& error_msg);
extern std::function<QString(QString)> xorString;

#endif // GLOBAL_H
