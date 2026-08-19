#include "chatuserwid.h"
#include "ui_chatuserwid.h"

ChatUserWid::ChatUserWid(QWidget *parent) :
    ListItemBase(parent),
    ui(new Ui::ChatUserWid)
{
    ui->setupUi(this);
    SetItem(ListItemType::ChatUserItem);
}

ChatUserWid::~ChatUserWid()
{
    delete ui;
}

// void ChatUserWid::SetInfo(QString name, QString head, QString msg)
// {
//     _name = name;
//     _head = head;
//     _msg = msg;
//     // 加载图片
//     QPixmap pixmap(_head);

//     // 设置图片自动缩放
//     ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
//     ui->icon_lb->setScaledContents(true);

//     ui->user_name_lb->setText(_name);
//     ui->user_chat_lb->setText(_msg);
// }

// void ChatUserWid::SetInfo(std::shared_ptr<UserInfo> user)
// {
//     _name = name;
//     _head = head;
//     _msg = msg;
//     // 加载图片
//     QPixmap pixmap(_head);

//     // 设置图片自动缩放
//     ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
//     ui->icon_lb->setScaledContents(true);

//     ui->user_name_lb->setText(_name);
//     ui->user_chat_lb->setText(_msg);
// }

QString ChatUserWid::GetUserName() {
    if (ui && ui->user_name_lb) { // 替换为你内部真实的 QLabel 变量名
        return ui->user_name_lb->text();
    }
    return QString();
}

QString ChatUserWid::GetUserIcon()  {
    // 假设你内部有一个存储当前头像路径的成员变量（例如 m_iconPath）
    // 或者直接从 QLabel 绑定的资产路径里拿。这里返回你 SetInfo 时存的那个路径字符串即可。
    return _user->_icon;
}

void ChatUserWid::SetInfo(std::shared_ptr<UserInfo> user)
{
    _user=user;
    QPixmap pixmap(_user->_icon);

    //     // 设置图片自动缩放
         ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
         ui->icon_lb->setScaledContents(true);

         ui->user_name_lb->setText(_user->_name);
         ui->user_chat_lb->setText(_user->_last_msg);
}

std::shared_ptr<UserInfo> ChatUserWid::GetUserInfo()
{
    return _user;
}