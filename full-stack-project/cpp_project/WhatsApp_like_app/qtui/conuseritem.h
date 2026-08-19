#ifndef CONUSERITEM_H
#define CONUSERITEM_H

#include <QWidget>
#include "listitembase.h"
#include "userdata.h"

namespace Ui {
class ConUserItem;
}

class ConUserItem : public ListItemBase
{
    Q_OBJECT

public:
    explicit ConUserItem(QWidget *parent = nullptr);
    ~ConUserItem();
    void setInfo(std::shared_ptr<AuthInfo> auth_info);
    void setInfo(std::shared_ptr<AuthRsp> auth_rsp);
    void setInfo(int uid,QString name,QString icon);
    void showRedPoint(bool bshow=false);
    std::shared_ptr<UserInfo> GetInfo();
    std::shared_ptr<UserInfo> _info;
    QSize sizeHint() const;

private:
    Ui::ConUserItem *ui;

};

#endif // CONUSERITEM_H
