#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QDialog>
#include "customizeedit.h"
#include "global.h"
#include "statewidget.h"
#include "userdata.h"
#include <QListWidgetItem>
class ChatBridge;

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
    void SetSelectedChatItem(int uid=0);
    void SwitchToUserChat(std::shared_ptr<UserInfo> user);


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
    QWidget* _last_widget;
    QMap<int, int> _user_history_cursor;
    ChatBridge* _bridge{nullptr};
    //QMap<int, QVariantList> _user_history_cache;
    //QMap<int,QList<ChatMsg>> _history_cache;

protected:
    bool eventFilter(QObject *watched,QEvent *event)override;
    void handleGlobalMousePress(QMouseEvent *event);

signals:
    void sig_load_user_history(int uid,int start,int len);

public slots:
    void slot_loading_user();
    void slot_side_contact();
    void slot_side_chat();
    void slot_text_changed(const QString& str="Find");
    void slot_friend_apply(std::shared_ptr<AddFriendApply>);
    void slot_auth_rsp(std::shared_ptr<AuthRsp>);
    void slot_add_auth_friend(std::shared_ptr<AuthInfo>);
    void slot_jump_chat_item(std::shared_ptr<SearchInfo> si);
    void slot_loading_contact_user();
    void slot_switch_info_page(std::shared_ptr<UserInfo> si);
    void slot_switch_apply_friend_page();
    void slot_jump_chat_item_from_info_page(std::shared_ptr<UserInfo>);
    void slot_item_click(QListWidgetItem* item);
    void slot_loadMoreHistory(const QString& user);
    void slot_load_history_finish(int from_uid,QList<ChatMsg> historyList,int next_last_msg_id);
    void slot_send_msg(QString target ,QString text);
    void slot_text_chat_msg(std::shared_ptr<ChatMsg> msg);


};


#endif // CHATDIALOG_H
