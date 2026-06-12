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

enum class ReqId{
    ID_GET_VERIFY_CODE=1001,//code
    ID_REG_USER=1002,//user signin

};

enum class Modules{
    REGISTERMOD=0,
};

enum class ErrorCode{
    SUCCESS=0,
    Err_JSON=1,
    Err_NETWORK=2,
};

extern std::function <void(QWidget*)>repolish;

#endif // GLOBAL_H
