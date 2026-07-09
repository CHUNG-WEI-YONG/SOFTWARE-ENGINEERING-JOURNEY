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
    GroupTipItem
};

struct Serverinfo{
    int uid;
    QString host;
    QString port;
    QString token;
};


extern std::function <void(QWidget*)>repolish;
extern QString gate_url_prefix;
extern bool ValidatePasswordStyle(const std::string& password, std::string& error_msg);
extern std::function<QString(QString)> xorString;

#endif // GLOBAL_H
