#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QDialog>
#include "customizeedit.h"
#include "global.h"
#include "statewidget.h"
#include "userdata.h"
#include <QListWidgetItem>
namespace Ui {
class ChatDialog;
}

class ChatDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChatDialog(QWidget *parent = nullptr);
    ~ChatDialog();
    void AddUserlist();
    void ClearState(StateWidget *lb);
    void SetSelectedChatPage(int uid=0);


private:
    void ShowSearch(bool b_search=false);
    void AddLBGroup(StateWidget *lb);
    void LoadMoreChatItem();
    void LoadMoreContactUser();
    ChatUIMode _mode;
    ChatUIMode _state;
    bool _b_loading;
    Ui::ChatDialog *ui;
    QList<StateWidget*> _lb_list;
    QAction* _clear_action = nullptr;
    QMap<int,QListWidgetItem*> _chat_items_added;
    int _curr_chat_uid;

protected:
    bool eventFilter(QObject *watched,QEvent *event)override;
    void handleGlobalMousePress(QMouseEvent *event);

public slots:
    void slot_loading_user();
    void slot_side_contact();
    void slot_side_chat();
    void slot_text_changed(const QString& str="Find");
    void slot_friend_apply(std::shared_ptr<AddFriendApply>);
    void slot_auth_rsp(std::shared_ptr<AuthRsp>);
    void slot_add_auth_friend(std::shared_ptr<AuthInfo>);
    void slot_jump_chat_item(std::shared_ptr<SearchInfo> si);
    void SetSelectedChatItem(int uid);
    void slot_loading_contact_user();
};


#endif // CHATDIALOG_H
