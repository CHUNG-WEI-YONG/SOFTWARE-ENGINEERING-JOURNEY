#ifndef CHATPAGE_H
#define CHATPAGE_H

#include <QWidget>
#include "userdata.h"

namespace Ui {
class ChatPage;
}

class ChatPage : public QWidget
{
    Q_OBJECT

public:
    explicit ChatPage(QWidget *parent = nullptr);
    ~ChatPage();
    void SetUserInfo(std::shared_ptr<UserInfo> user);

protected:
    void paintEvent(QPaintEvent *event);

private:
    Ui::ChatPage *ui;
    std::shared_ptr<UserInfo> _user_info;
};

#endif // CHATPAGE_H
