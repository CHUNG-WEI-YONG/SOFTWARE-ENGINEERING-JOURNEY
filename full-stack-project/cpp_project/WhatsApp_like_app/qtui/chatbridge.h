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
    Q_INVOKABLE void sendMessageFromQml(const QString& targetUser, const QString& text);
signals:
    void sig_user_switched(QString name, bool isOnline, QString lastTime,QString logoPath, QVariantList history);

    // 当底层 TCP 收到好友发来的实时新消息时，轰鸣触发这个信号
    void sig_new_message_received(QString sender, QString message);
};

#endif // CHATBRIDGE_H
