#include "chatbridge.h"
#include "tcpmgr.h"

void ChatBridge::sendMessageFromQml(const QString& targetUser, const QString& text) {
    qDebug() << "🛰️ [C++ 后台] 收到发给" << targetUser << "的消息:" << text;
    // 在这里调用你的网络总线发送：
    emit sig_send_msg(targetUser,text);
    //TcpMgr::getInstance()->sendMsg(targetUser, text);
}

void ChatBridge::loadMoreHistoryFromQML(const QString &target)
{
    qDebug()<<"Until top, starting to get older message";
    emit sig_req_more_history(target);
}