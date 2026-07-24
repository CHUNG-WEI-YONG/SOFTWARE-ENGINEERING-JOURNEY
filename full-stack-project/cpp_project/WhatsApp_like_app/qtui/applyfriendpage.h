#ifndef APPLYFRIENDPAGE_H
#define APPLYFRIENDPAGE_H

#include <QWidget>
#include <memory>
#include "userdata.h"
#include <QJsonArray>
#include <unordered_map>

class ApplyFriendItem;

namespace Ui {
class ApplyFriendPage;
}

class ApplyFriendPage : public QWidget
{
    Q_OBJECT

public:
    explicit ApplyFriendPage(QWidget *parent = nullptr);
    ~ApplyFriendPage();
    void AddNewFriendApply(std::shared_ptr<AddFriendApply> apply);

protected:
    void paintEvent(QPaintEvent *event);


private:
    Ui::ApplyFriendPage *ui;
    void loadApplyList();
    std::unordered_map<int, ApplyFriendItem*> _unauth_items;

public slots:
    void slot_auth_rsp(std::shared_ptr<AuthRsp> rsp);

signals:
    void sig_show_search(bool);
};

#endif // APPLYFRIENDPAGE_H
