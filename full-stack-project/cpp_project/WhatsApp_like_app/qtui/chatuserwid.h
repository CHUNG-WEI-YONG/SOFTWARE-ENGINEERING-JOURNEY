#ifndef CHATUSERWID_H
#define CHATUSERWID_H

#include <QWidget>
#include "listitembase.h"
#include "userdata.h"
namespace Ui {
class ChatUserWid;
}

class ChatUserWid : public ListItemBase
{
    Q_OBJECT

public:
    explicit ChatUserWid(QWidget *parent = nullptr);
    ~ChatUserWid();
    QString GetUserName();

    QSize sizeHint() const override {
        return QSize(250, 70); // 返回自定义的尺寸
    }
    QString GetUserIcon();

    //void SetInfo(QString name, QString head, QString msg);
    void SetInfo(std::shared_ptr<UserInfo> user);

private:
    Ui::ChatUserWid *ui;
    // QString _name;
    // QString _head;
    // QString _msg;
    QVariantList m_chatHistory;
    std::shared_ptr<UserInfo> _user;

    // 初始化时或者构造时加两条假数据方便你直接测试：
    void initFakeData(const QString& name) {
        QVariantMap msg1, msg2;
        msg1["sender"] = "other";
        msg1["message"] = QString("Hello, I am %1! Nice to meet you.").arg(name);
        msg2["sender"] = "me";
        msg2["message"] = "Hi there!";
        m_chatHistory.append(msg1);
        m_chatHistory.append(msg2);
    }
};

#endif // CHATUSERWID_H
