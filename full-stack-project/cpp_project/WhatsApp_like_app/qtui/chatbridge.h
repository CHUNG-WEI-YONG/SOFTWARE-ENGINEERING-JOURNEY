#ifndef CHATBRIDGE_H
#define CHATBRIDGE_H
#include <QObject>
#include <QVariantMap>
#include <QVariantList>
class ChatBridge:public QObject
{
    Q_OBJECT
public:
    ChatBridge(QObject *parent=nullptr):QObject(parent){}
    Q_INVOKABLE void sendMessageFromQml(const QString& targetUser, const QString& text) {
        qDebug() << "🛰️ [C++ 后台] 收到发给" << targetUser << "的消息:" << text;
        // 在这里调用你的网络总线发送：
        // TcpMgr::getInstance()->sendMsg(targetUser, text);
    }
signals:
    // ──► 🎯 核心招式 2：当左侧好友被点击，C++ 发射此信号通知 QML 刷新皮肤与历史记录 ◄──
    void sig_user_switched(QString name, bool isOnline, QString lastTime, QVariantList history);

    // 当底层 TCP 收到好友发来的实时新消息时，轰鸣触发这个信号
    void sig_new_message_received(QString sender, QString message);
};

#endif // CHATBRIDGE_H
