#include "chatbridge.h"

void ChatBridge::sendMessageFromQml(const QString& targetUser, const QString& text) {
    qDebug() << "🛰️ [C++ 后台] 收到发给" << targetUser << "的消息:" << text;
    // 在这里调用你的网络总线发送：
    // TcpMgr::getInstance()->sendMsg(targetUser, text);
}