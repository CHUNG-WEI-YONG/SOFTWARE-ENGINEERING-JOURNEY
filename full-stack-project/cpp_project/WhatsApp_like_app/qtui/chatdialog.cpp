#include "chatdialog.h"
#include "ui_chatdialog.h"
#include <QAction>
#include <QRandomGenerator>
#include <chatuserwid.h>
#include "loadingdialog.h"
#include <QWidget>
#include <QQmlContext>
#include "chatbridge.h"
#include "usermgr.h"
#include <QTimer>
#include "contactuserlist.h"
#include "tcpmgr.h"
#include "conuseritem.h"
#include "friendinfopage.h"
#include "friendinfopage.h"
#include <QFileInfo>
#include "FileUploader.h"
#include <QThread>
#include <QFileDialog>
#include <QStandardPaths>
#include "filedownloader.h"
#include <QMessageBox>
#include "logindialog.h"
#include <QTimer>
#include "userdata.h"

ChatDialog::ChatDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ChatDialog)
    , _mode(ChatUIMode::ChatMode)
    , _state(ChatUIMode::ChatMode),_curr_chat_uid(0),_b_loading(false),
    _last_widget(nullptr)
{
    ui->setupUi(this);

    ui->add_btn->SetState("normal", "hover", "press");
    ui->add_btn->setProperty("state", "normal");
    ui->search_edit->SetMax(25);

    QAction* searchaction = new QAction(ui->search_edit);
    searchaction->setIcon(QIcon(":/rc/chat_picture/search.png"));
    ui->search_edit->addAction(searchaction, QLineEdit::LeadingPosition);
    ui->search_edit->setPlaceholderText(QStringLiteral("Search list"));

    // ──► 🎯 核心修正 1：直接把局部变量焊死到类成员指针 _clear_action ◄──
    _clear_action = new QAction(ui->search_edit);
    _clear_action->setIcon(QIcon(":/rc/chat_picture/close_transparent.png"));
    ui->search_edit->addAction(_clear_action, QLineEdit::TrailingPosition);


    //connect(ui->con_user_list,&ContactUserList::sig_switch_apply_friend_page,this,&ChatDialog::slot_text_changed);
    // ──► 🎯 核心修正 2：唯一绑定 textChanged 信号到 slot_text_changed，绝不搞双重绑定 ◄──
    connect(ui->search_edit, &QLineEdit::textChanged, this, [this](QString const& str){
        if (!_clear_action) return;

        if (!str.isEmpty()) {
            _clear_action->setIcon(QIcon(":/rc/chat_picture/close_search.png"));
            slot_text_changed(str);
        } else {
            // 当被清空时，小叉号瞬间遁形透明，并完美退回原视窗
            _clear_action->setIcon(QIcon(":/rc/chat_picture/close_transparent.png"));
            slot_text_changed(str);
        }
    });

    // ──► 🎯 核心修正 3：极简清除总线，只做输入清空与焦点释放，绝不越权调用 ShowSearch ◄──
    connect(_clear_action, &QAction::triggered, this, [this](){
        ui->search_edit->clear(); // 🚀 这一行会自动引爆 slot_text_changed 里的清除与退回逻辑！
        ui->search_edit->clearFocus();
    });

    // ... 5. 实例化 QML 通信桥梁至构造函数末尾保持不变 ...
    ChatBridge* bridge = new ChatBridge(this);
    _bridge=bridge;
    QQmlContext* context = ui->chat_quickwid->rootContext();
    QString currentUserName = UserMgr::getInstance()->returnName();
    QString myOwnLogoPath = "qrc:/rc/chat_picture/head_5.jpg";

    context->setContextProperty("cppBridge", bridge);
    //context->setContextProperty("currentChatUserName", "Select a friend...");
    context->setContextProperty("myOwnAvatar", myOwnLogoPath);
    context->setContextProperty("currentChatUserName", currentUserName);

    ui->chat_quickwid->setSource(QUrl(QStringLiteral("qrc:/style/ChatPage.qml")));
    ui->chat_quickwid->setResizeMode(QQuickWidget::SizeRootObjectToView);
    qDebug() << "🔍 [QML 状态诊断] Status:" << ui->chat_quickwid->status();
    qDebug() << "🔍 [QML 错误列表]:" << ui->chat_quickwid->errors();
    ui->stackedWidget->setCurrentIndex(0);


    ShowSearch(false);
    connect(ui->chat_user_list, &ChatUserList::sig_loading_user, this, &ChatDialog::slot_loading_user);
    AddUserlist();

    QPixmap pixmap(":/rc/chat_picture/head_1.jpg");
    QPixmap scaledPixmap = pixmap.scaled(ui->side_head_lb->size(), Qt::KeepAspectRatio);
    ui->side_head_lb->setPixmap(scaledPixmap);
    ui->side_head_lb->setScaledContents(true);

    ui->side_chat_lb->setProperty("state","normal");
    ui->side_chat_lb->SetState("normal","hover","pressed","selected_normal","selected_hover","selected_pressed");
    ui->side_contact_lb->SetState("normal","hover","pressed","selected_normal","selected_hover","selected_pressed");

    ui->side_setting->setProperty("state","normal");
    ui->side_setting->SetState("normal","hover","pressed","selected_normal","selected_hover","selected_pressed");

    AddLBGroup(ui->side_chat_lb);
    AddLBGroup(ui->side_contact_lb);
    AddLBGroup(ui->side_setting);

    connect(ui->side_chat_lb,&StateWidget::clicked,this,&ChatDialog::slot_side_chat);
    connect(ui->side_contact_lb,&StateWidget::clicked,this,&ChatDialog::slot_side_contact);
    connect(ui->side_setting,&StateWidget::clicked,this,&ChatDialog::slot_side_setting);
    ShowSearch(false);

    QTimer::singleShot(0, this, [this](){
        ui->search_edit->clearFocus();
        this->setFocus();
    });
    this->installEventFilter(this);
    ui->side_chat_lb->SetSelected(true);
    //ui->search_user_list->SetSearchEdit(ui->search_edit);
    SetSelectedChatItem();
    SetSelectedChatPage();

    ui->search_user_list->SetSearchEdit(ui->search_edit);
    connect(TcpMgr::getInstance().get(),&TcpMgr::sig_friend_apply,this,&ChatDialog::slot_friend_apply);

    connect(TcpMgr::getInstance().get(),&TcpMgr::sig_auth_rsp,this,&ChatDialog::slot_auth_rsp);
    connect(TcpMgr::getInstance().get(),&TcpMgr::sig_add_auth_friend,this,&ChatDialog::slot_add_auth_friend);
    connect(ui->search_user_list,&SearchList::sig_jump_chat_item,this,&ChatDialog::slot_jump_chat_item);
    connect(ui->con_user_list,&ContactUserList::sig_loading_contact_user,this,&ChatDialog::slot_loading_contact_user);
    connect(ui->con_user_list,&ContactUserList::sig_switch_friend_info_page,this ,&ChatDialog::slot_switch_info_page);
    connect(ui->con_user_list,&ContactUserList::sig_switch_apply_friend_page,this,&ChatDialog::slot_switch_apply_friend_page);
    connect(ui->chat_info_page,&FriendInfoPage::sig_jump_chat_item,this,&ChatDialog::slot_jump_chat_item_from_info_page);

    ui->stackedWidget->setCurrentWidget(ui->chat_widget);
    connect(ui->chat_user_list,&QListWidget::itemClicked,this,&ChatDialog::slot_item_click);
    connect(_bridge,&ChatBridge::sig_req_more_history,this,&ChatDialog::slot_loadMoreHistory);
    connect(TcpMgr::getInstance().get(),&TcpMgr::sig_load_history_finish,this,&ChatDialog::slot_load_history_finish);
    connect(_bridge,&ChatBridge::sig_send_msg,this,&ChatDialog::slot_send_msg);
    connect(TcpMgr::getInstance().get(),&TcpMgr::sig_text_chat_msg,this,&ChatDialog::slot_text_chat_msg);
    connect(_bridge,&ChatBridge::sig_req_upload_file,this,&ChatDialog::slot_req_upload_file);
    connect(TcpMgr::getInstance().get(),&TcpMgr::sig_upload_file,this,&ChatDialog::slot_upload_file);
    connect(_bridge,&ChatBridge::sig_req_download_file,this,&ChatDialog::slot_req_download_file);
    connect(TcpMgr::getInstance().get(), &TcpMgr::sig_download_file_rsp, this, &ChatDialog::slot_download_file);
    connect(TcpMgr::getInstance().get(),&TcpMgr::sig_load_chat_thread,this,&ChatDialog::slot_load_chat_thread);
    connect(TcpMgr::getInstance().get(),&TcpMgr::sig_create_private_chat,this,&ChatDialog::slot_create_private_chat);
    connect(TcpMgr::getInstance().get(),&TcpMgr::sig_text_chat_msg_rsp,this,&ChatDialog::slot_text_chat_msg_rsp);
    connect(TcpMgr::getInstance().get(),&TcpMgr::sig_text_chat_msg_fail,this,&ChatDialog::slot_text_chat_msg_fail);
}

ChatDialog::~ChatDialog()
{
    if(_timer){
        _timer->stop();
    }
    delete ui;
}

void ChatDialog::AddUserlist()
{
    // auto friend_list=UserMgr::getInstance()->GetChatListPerPage();
    // if(!friend_list.empty()){
    //     for(auto& f:friend_list){
    //         auto chat_data=std::make_shared<ChatData>(f->_uid,f->_name,f->_icon);
    //         auto iter=_chat_items_added.find(f->_uid);
    //         if(iter!=_chat_items_added.end()){
    //             return;
    //         }
    //         auto* chat_user_wid=new ChatUserWid();
    //         std::shared_ptr<UserInfo> f_ele=std::make_shared<UserInfo>(f);
    //         chat_user_wid->SetInfo(f_ele);
    //         chat_user_wid->SetChatData(chat_data);
    //         auto *item=new QListWidgetItem();
    //         item->setSizeHint(chat_user_wid->sizeHint());
    //         ui->chat_user_list->addItem(item);
    //         ui->chat_user_list->setItemWidget(item,chat_user_wid);
    //         _chat_items_added[f->_uid]=item;
    //     }
    // }
    // UserMgr::getInstance()->UpdateChatLoadedCount();

    _timer=new QTimer(this);
    connect(_timer,&QTimer::timeout,this,[this](){
        auto user=UserMgr::getInstance()->GetUserInfo();
        QJsonObject obj;
        obj["from_uid"]=user->_uid;
        QJsonDocument doc(obj);
        QByteArray send=doc.toJson(QJsonDocument::Compact);
        emit TcpMgr::getInstance()->sig_send_data(ReqId::ID_HEART_BEAT_REQ,send);
    });

    // _timer->start(10000);
}

void ChatDialog::ClearState(StateWidget *lb)
{
    for(auto &ele:_lb_list){
        if(ele==lb){
            continue;
        }
        ele->ClearState();
    }
}

void ChatDialog::SetSelectedChatPage(int thread_id)
{
    if (ui->chat_user_list->count() <= 0) return;

    QListWidgetItem* targetItem = nullptr;
    if (thread_id <= 0) {
        targetItem = ui->chat_user_list->item(0);
    } else {
        auto iter = _chat_thread_added.find(thread_id);
        if (iter != _chat_thread_added.end()) {
            targetItem = iter.value();
        }
    }

    if (!targetItem) return;

    QWidget* w = ui->chat_user_list->itemWidget(targetItem);
    auto chat_wid = qobject_cast<ChatUserWid*>(w);
    if (chat_wid && chat_wid->GetUserInfo()) {
        _curr_thread_id = chat_wid->getThreadInfo()->_thread_id;
        _curr_chat_uid = chat_wid->GetUserInfo()->_uid;
        SwitchToUserChat(chat_wid->GetUserInfo());
    }
}




void ChatDialog::SetSelectedChatItem(int thread_id)
{
    if (ui->chat_user_list->count() <= 0) return;

    if (thread_id <= 0) {
        // 默认选中第 0 行
        ui->chat_user_list->setCurrentRow(0);
        auto item = ui->chat_user_list->item(0);
        if (!item) return;
        auto wid = qobject_cast<ChatUserWid*>(ui->chat_user_list->itemWidget(item));
        if (wid && wid->GetUserInfo()) {
            _curr_chat_uid = wid->GetUserInfo()->_uid;
            _curr_thread_id = wid->getThreadInfo()->_thread_id; // ChatUserWid 需存有 thread_id
        }
        return;
    }

    auto iter = _chat_thread_added.find(thread_id);
    if (iter != _chat_thread_added.end()) {
        int row = ui->chat_user_list->row(iter.value());
        if (row >= 0) {
            ui->chat_user_list->setCurrentRow(row);
            _curr_thread_id = thread_id;
            auto wid = qobject_cast<ChatUserWid*>(ui->chat_user_list->itemWidget(iter.value()));
            if (wid && wid->GetUserInfo()) {
                _curr_chat_uid = wid->GetUserInfo()->_uid;
            }
        }
    }
}

void ChatDialog::SwitchToUserChat(std::shared_ptr<UserInfo> user)
{
    if (!user) return;
    qDebug() << "👉 [探针 4] 进入 SwitchToUserChat, 当前 _curr_chat_uid=" << _curr_chat_uid << "目标 uid=" << user->_uid;
    if (_curr_chat_uid == user->_uid) {
        qDebug() << "⚠️ [探针 4 拦截] _curr_chat_uid 相同，被拦截退出!";
        return;
    }

    int uid = UserMgr::getInstance()->GetUid();

    QVariantList history;
    auto thread_id=UserMgr::getInstance()->GetFriendThreadId(user->_uid);
    auto thread_data=UserMgr::getInstance()->GetThreadData(thread_id);

    _curr_chat_uid = user->_uid;
    _curr_thread_id=thread_id;

    // 1. 先判断是否有缓存，绝不在判断前提前调用 GetHistoryMsgs
    if (thread_data&& !thread_data->GetMsgMapRef().isEmpty()) {

        // 首次打开：初始化分页游标并触发异步拉取
        _thread_history_cursor[thread_id] = 0;

        // 🚀 恢复发射信号：通知后台线程或 SQLite 查询历史（先显示空历史，查完再追加）
        emit sig_load_user_history(thread_id, 0, 20);
    } else {
        // 2. 命中内存缓存：安全提取数据并格式化
        auto history_cache = thread_data->GetMsgMapRef();
        for (const auto& msg : history_cache) {
            history.append(msg->toVariantMap(uid));
        }
    }
    qDebug() << "👉 [探针 5] C++ 准备发射 sig_user_switched! 历史消息数量:" << history.size();

    // 3. 通知前端切换会话（若无缓存，此时传递的是空的 history，等待后续槽函数追加）
    emit _bridge->sig_user_switched(user->_name,
                                    true,
                                    "today",
                                    user->_icon,
                                    history);

    ui->stackedWidget->setCurrentIndex(0);
}


void ChatDialog::slot_loading_contact_user()
{
    qDebug()<<"load more contact user";
    if(_b_loading){
        return;
    }
    _b_loading=true;
    LoadingDialog* loading=new LoadingDialog(this,"Loading your contact");
    loading->setModal(true);
    loading->show();
    LoadMoreContactUser();
    loading->deleteLater();
    _b_loading=false;
}

void ChatDialog::slot_switch_info_page(std::shared_ptr<UserInfo> si)
{
    qDebug()<<"Receive switch to friend info page";
    _last_widget=ui->chat_info_page;
    ui->stackedWidget->setCurrentWidget(ui->chat_info_page);
    ui->chat_info_page->SetInfo(si);


}

void ChatDialog::slot_switch_apply_friend_page()
{
    qDebug()<<"switch to apply friend page";
    _last_widget=ui->friend_apply_page;
    ui->stackedWidget->setCurrentWidget(_last_widget);
}

void ChatDialog::slot_jump_chat_item_from_info_page(std::shared_ptr<UserInfo>info)
{
    qDebug()<<"from chat item to info page";
    auto thread_id=UserMgr::getInstance()->GetFriendThreadId(info->_uid);
    if(thread_id!=-1){
        auto iter=_chat_thread_added.find(thread_id);
        if(iter!=_chat_thread_added.end()){
            qDebug()<<"found chat item";
            ui->chat_user_list->scrollToItem(iter.value());
            ui->side_chat_lb->SetSelected(true);
            SetSelectedChatItem(info->_uid);
            SetSelectedChatPage(info->_uid);
            slot_side_chat();
            return ;
        }

        auto *chat_user_wid=new ChatUserWid();
        chat_user_wid->SetInfo(info);
        QListWidgetItem *item=new QListWidgetItem;
        item->setSizeHint(chat_user_wid->sizeHint());
        ui->chat_user_list->insertItem(0,item);
        ui->chat_user_list->setItemWidget(item,chat_user_wid);

        _chat_thread_added[thread_id]=item;

        ui->side_chat_lb->SetSelected(true);
        SetSelectedChatItem(info->_uid);
        SetSelectedChatPage(info->_uid);
        slot_side_chat();
        return;
    }

    int user1_id=UserMgr::getInstance()->GetUid();
    int user2_id= info->_uid;
    QJsonObject obj;
    obj["uid"]=user1_id;
    obj["other_uid"]=user2_id;

    QJsonDocument doc(obj);
    QByteArray data=doc.toJson(QJsonDocument::Compact);
    emit TcpMgr::getInstance()->sig_send_data(ReqId::ID_CREATE_PRIVATE_CHAT_REQ,data);


}

void ChatDialog::slot_item_click(QListWidgetItem *item)
{
    qDebug() << "👉 [探针 1] 点击了列表项 item:" << item;
    QWidget* widget = ui->chat_user_list->itemWidget(item); // 获取自定义widget对象
    if (!widget) {
        qDebug() << "slot item clicked widget is nullptr";
        return;
    }

    // 对自定义widget进行操作， 将item 转化为基类ListItemBase
    ListItemBase* customItem = qobject_cast<ListItemBase*>(widget);
    if (!customItem) {
        qDebug() << "slot item clicked widget is nullptr";
        return;
    }

    auto itemType = customItem->getItem();
    qDebug() << "👉 [探针 2] itemType 值为:" << static_cast<int>(itemType);
    if (itemType == ListItemType::InvalidItem
        || itemType == ListItemType::Group_Tip_Item) {
        qDebug() << "slot invalid item clicked ";
        return;
    }


    if (itemType == ListItemType::ChatUserItem) {
        // 创建对话框，提示用户
        qDebug() << "contact user item clicked ";

        auto chat_wid = qobject_cast<ChatUserWid*>(customItem);

        auto user_info = chat_wid->GetUserInfo();
        if (!user_info) return;

        // 2. 记录当前活跃聊天的 UID
        //_curr_chat_uid = user_info->_uid;
        qDebug() << "👉 [探针 3] 准备切换用户:" << user_info->_name << " UID:" << user_info->_uid;
        SwitchToUserChat(user_info);

        // int myuid=UserMgr::getInstance()->GetUid();
        // QVariantList history;
        // auto _history_cache=UserMgr::getInstance()->GetHistoryMsgs(_curr_chat_uid);
        // for(auto const& msg:_history_cache){
        //         history.append(msg.toVariantMap(myuid));
        //     }


        // //跳转到聊天界面
        // emit _bridge->sig_user_switched(
        //                 user_info->_name,
        //                 false,
        //                 "today",
        //                 user_info->_icon,
        //                 history);
        // ui->stackedWidget->setCurrentIndex(0);
        return;
    }
}

void ChatDialog::slot_loadMoreHistory(const QString &user)
{
    auto friend_info=UserMgr::getInstance()->getFriendByName(user);
    if(!friend_info){
        qDebug()<<"This friend not exist";
        return;
    }
    int to_uid=friend_info->_uid;
    auto self_uid=UserMgr::getInstance()->GetUid();
    int last_msg_id = 0;
    int thread_id=UserMgr::getInstance()->GetFriendThreadId(to_uid);
    if(thread_id<=0){
        qDebug()<<"Cannot find the thread id for "<<friend_info->_name;
        return;
    }
    if (_thread_history_cursor.contains(to_uid)) {
        last_msg_id = _thread_history_cursor[to_uid];
    }

    QJsonObject root;
    root["uid"]=self_uid;
    root["to_uid"]=to_uid;
    root["last_msg_id"]=last_msg_id;
    root["pgsize"]=MAX_COUNT_PER_PAGE;
    QJsonDocument doc(root);
    QByteArray sendData = doc.toJson(QJsonDocument::Compact);

    TcpMgr::getInstance()->sig_send_data(ReqId::ID_LOAD_CHAT_MSG_REQ,sendData);
}


void ChatDialog::slot_load_history_finish(int thread_id,const std::vector<std::shared_ptr<ChatDataBase>>& historyList,int next_last_msg_id)
{

    _thread_history_cursor[thread_id]=next_last_msg_id;
    auto thread_data=UserMgr::getInstance()->GetThreadData(thread_id);
    //UserMgr::getInstance()->get
    if (!thread_data) {
        int my_uid=UserMgr::getInstance()->GetUid();
        int peer_uid=0;
        for (const auto& m : historyList) {
            if (m && m->GetSendUid() != my_uid) {
                peer_uid = m->GetSendUid();
                break;
            }
        }
        thread_data = std::make_shared<ChatThreadData>(peer_uid, thread_id, 0);
        UserMgr::getInstance()->AddThreadData(thread_id, thread_data);
    }

    int my_uid = UserMgr::getInstance()->GetUid();
    QVariantList qmlBatch;
    for (const auto& msg : historyList) {
        if(!msg)continue;
        thread_data->AppendMsg(msg->GetMsgId(),msg);
        if(_curr_thread_id==thread_id){
            qmlBatch.append(msg->toVariantMap(my_uid));
        }

    }
    if (_curr_thread_id == thread_id && !qmlBatch.isEmpty()) {
        emit _bridge->sig_append_history_batch(qmlBatch);
    }


}

void ChatDialog::slot_send_msg(QString target, QString text)
{
    Q_UNUSED(target); // 消除未引用参数警告

    int my_uid = UserMgr::getInstance()->GetUid();
    int friend_uid = _curr_chat_uid;
    if (_curr_chat_uid == 0 || text.trimmed().isEmpty()) {
        return;
    }

    // 1. 解析会话 ID，并进行前置有效性校验
    int thread_id = UserMgr::getInstance()->GetFriendThreadId(friend_uid);
    if (thread_id <= 0) {
        qWarning() << "[ChatDialog::slot_send_msg] Invalid thread_id for uid:" << friend_uid;
        return;
    }

    // 2. 生成本地唯一 ID 与当前时间戳
    int uuid = static_cast<int>(QDateTime::currentMSecsSinceEpoch() & 0x7FFFFFFF);
    QString timeStr = QTime::currentTime().toString("hh:mm AP");
    QString unique_id = QString::number(uuid);

    // 3. 构建多态文本消息（status = 0 表示发送中）
    auto text_msg = std::make_shared<TextChatData>(
        uuid,
        thread_id,
        ChatFormType::PRIVATE,
        text,
        my_uid,
        0,        // status: 0
        timeStr
        );
    text_msg->SetUniqueId(unique_id);

    // 4. 存入会话聚合根 ChatThreadData
    auto thread_data = UserMgr::getInstance()->GetThreadData(thread_id);
    if (!thread_data) {
        thread_data = std::make_shared<ChatThreadData>(friend_uid, thread_id, 0);
        UserMgr::getInstance()->AddThreadData(thread_id, thread_data);
    }
    thread_data->AppendUnRspMsg(unique_id, text_msg);

    // 5. 更新左侧会话项最后消息预览并置顶
    auto iter = _chat_thread_added.find(thread_id);
    if (iter != _chat_thread_added.end()) {
        auto item = iter.value();
        auto wid = qobject_cast<ChatUserWid*>(ui->chat_user_list->itemWidget(item));
        if (wid) wid->updateLastMsg(text);

        int row = ui->chat_user_list->row(item);
        if (row > 0) {
            ui->chat_user_list->takeItem(row);
            ui->chat_user_list->insertItem(0, item);
            ui->chat_user_list->setItemWidget(item, wid);
            ui->chat_user_list->setCurrentItem(item);
        }
    }

    // 6. 发送网络数据包
    QJsonObject obj;
    obj["from_uid"]  = my_uid;
    obj["to_uid"]    = friend_uid;
    obj["type"]      = "text";
    obj["content"]   = text;
    obj["msg_id"]    = uuid;
    obj["thread_id"] = thread_id;

    QJsonDocument doc(obj);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    emit TcpMgr::getInstance()->sig_send_data(ReqId::ID_TEXT_CHAT_MSG_REQ, data);
}

void ChatDialog::slot_text_chat_msg(std::shared_ptr<ChatDataBase> msg)
{
    if (!msg) return;

    auto from_uid = msg->GetSendUid();
    int thread_id=UserMgr::getInstance()->GetFriendThreadId(from_uid);
    if (thread_id <= 0) {
        thread_id = UserMgr::getInstance()->GetFriendThreadId(from_uid);
    }
    if (thread_id <= 0) {
        qWarning() << "[ChatDialog::slot_text_chat_msg] Cannot resolve thread_id for uid:" << from_uid;
        return;
    }

    auto thread_data=UserMgr::getInstance()->GetThreadDataByUid(from_uid);
    thread_data->AddMsg(msg);

    auto iter = _chat_thread_added.find(from_uid);
    ChatUserWid* chat_user_wid = nullptr;
    QListWidgetItem* item = nullptr;

    if (iter == _chat_thread_added.end()||thread_id<0) {
        qDebug() << "Friend message, add new chat item, id is " << from_uid;
        auto f = UserMgr::getInstance()->getFriend(from_uid);
        if (!f) {
            qDebug() << "Cannot find friend info for uid:" << from_uid;
            return;
        }

        //auto chat_data = std::make_shared<ChatThreadData>(f->_uid, f->_name, f->_icon);
        chat_user_wid = new ChatUserWid(); // 👈 修正：直接为外部指针赋值，避免变量遮蔽

        std::shared_ptr<UserInfo> f_user=std::make_shared<UserInfo>(f);
        chat_user_wid->SetInfo(f_user);
        chat_user_wid->SetChatData(thread_data);

        item = new QListWidgetItem();
        item->setSizeHint(chat_user_wid->sizeHint());
        ui->chat_user_list->insertItem(0, item); // 新来消息的好友条目建议直接插入最顶端 (Row 0)
        ui->chat_user_list->setItemWidget(item, chat_user_wid);
        _chat_thread_added[from_uid] = item;
    } else {
        item = iter.value();
        QWidget* widget = ui->chat_user_list->itemWidget(item);
        chat_user_wid = qobject_cast<ChatUserWid*>(widget);
    }

    if (chat_user_wid) {
        QString previewText=msg->GetContent();
        if(msg->GetMsgType()==ChatMsgType::FILE){
            previewText = QString("[File] %1").arg(msg->GetContent());
        }
        // 更新最后一条消息预览
        chat_user_wid->updateLastMsg(previewText);

        // 收到消息时将已有会话项置顶
        if (item) {
            int row = ui->chat_user_list->row(item);
            if (row > 0) {
                ui->chat_user_list->takeItem(row);
                ui->chat_user_list->insertItem(0, item);
                ui->chat_user_list->setItemWidget(item, chat_user_wid);
            }
        }
    }

    // 路由分发：如果在当前会话则推入气泡，否则点亮未读红点
    if (_curr_chat_uid == from_uid) {
        QString sender_name = chat_user_wid ? chat_user_wid->GetUserName() : "Friend";
        if(msg->GetMsgType()==ChatMsgType::FILE){
            auto file_msg = std::dynamic_pointer_cast<FileChatData>(msg);
            if (file_msg) {
                QVariantMap fileMap = file_msg->toVariantMap(UserMgr::getInstance()->GetUid());
                emit _bridge->sig_new_file_arrive(
                    sender_name,
                    fileMap["content"].toString(),
                    fileMap["fileSize"].toString(),
                    "",                          // 本地暂存路径为空（待接收方下载）
                    fileMap["timeStr"].toString(),
                    fileMap["fileUrl"].toString() // 文件下载 Token
                    );
            }
        }

        else{
            emit _bridge->sig_new_message_received(sender_name, msg->GetContent());
        }
    } else {
        if (chat_user_wid) {
            chat_user_wid->ShowRedPoint(true);
        }
    }
}

void ChatDialog::slot_req_upload_file(const QString &friendName, const QString &filePath)
{
    auto friend_ptr=UserMgr::getInstance()->getFriendByName(friendName);
    if (!friend_ptr) {
        qWarning() << "[ChatDialog] Cannot find about the friend: " << friendName;
        return;
    }

    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        qWarning() << "[ChatDialog] File path is not existing or it is not a file:" << filePath;
        return;
    }
    int touid=friend_ptr->_uid;
    QString filename=fileInfo.fileName();
    qint64 filesz=fileInfo.size();

    _pending_upload_path=filePath;
    auto md5=CalculateFileMD5(filePath);
    QJsonObject obj;
    obj["to_uid"]=touid;
    obj["from_uid"]=UserMgr::getInstance()->GetUid();
    obj["filename"]=filename;
    obj["filesz"]=filesz;
    obj["md5"]=md5;

    QJsonDocument doc(obj);
    QByteArray jsonStr = doc.toJson(QJsonDocument::Compact);

    TcpMgr::getInstance()->sig_send_data(ReqId::ID_SNED_FILE_REQ,jsonStr);
    qDebug() << "已向 ChatServer 申请上传凭据，目标 UID:" << friend_ptr->_uid
             << "文件名:" << filename << "文件大小:" << filesz << "Bytes";
}

void ChatDialog::slot_upload_file(std::shared_ptr<FileToken>token)
{
    if (!token) {
        qWarning() << "[ChatDialog] FileToken is null!";
        return;
    }
    qDebug() << "[ChatDialog] Starting upload task for:" << token->filename
             << "to" << token->host << ":" << token->port;

    auto localFilePath=_pending_upload_path;
    if(localFilePath.isEmpty()){
        qDebug()<<"Error in getting the file";
        return;
    }

    addInitialFileBubbleUi(token);
    FileUploader *uploader=new FileUploader(*token , localFilePath);
    QThread *thread=new QThread;
    uploader->moveToThread(thread);
    connect(thread,&QThread::started,uploader,&FileUploader::startUpload);
    connect(uploader, &FileUploader::sig_finished, thread, &QThread::quit);
    connect(thread, &QThread::finished, uploader, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    connect(uploader, &FileUploader::sig_progress, this, [this,token](qint64 sent, qint64 total) {
        int percent = (total > 0) ? static_cast<int>((sent * 100) / total) : 0;
        // 更新 UI 进度条
        updateFileProgressUI(percent,token->to_uid);
        qDebug() << "[ChatDialog] Upload progress:" << percent << "% (" << sent << "/" << total << ")";
    }, Qt::QueuedConnection);

    connect(uploader, &FileUploader::sig_finished, this, [this, token](bool success, const QString& reason) {
        auto fri_ptr = UserMgr::getInstance()->getFriend(token->to_uid);
        QString friendName = fri_ptr ? fri_ptr->_name : "";

        if (success) {
            qDebug() << "[ChatDialog] File successfully uploaded:" << token->filename;

            // ① 更新 UI 气泡为完成状态
            emit _bridge->sig_file_upload_complete(friendName, true, token->token);

            // ② 通知 ChatServer 转发给对端好友
            sendFileMsgToChatServer(token);
        } else {
            qWarning() << "[ChatDialog] File upload failed:" << reason;

            // 更新 UI 为失败状态
            emit _bridge->sig_file_upload_complete(friendName, false, reason);
        }
    }, Qt::QueuedConnection);

    // ──► 5. 启动子线程开始推流 ◄──
    thread->start();
}

void ChatDialog::slot_req_download_file(const QString &fileToken, const QString &fileName)
{
    // 1. 让用户选择保存到本地哪一个文件
    QString defaultDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    QString defaultPath = defaultDir + "/" + fileName;

    QString saveFilePath = QFileDialog::getSaveFileName(
        this,
        tr("Save File As"),
        defaultPath,
        tr("All Files (*.*)")
        );

    if (saveFilePath.isEmpty()) {
        qDebug() << "[ChatDialog] Download canceled by user.";
        return;
    }

    // 暂存本地路径，等待 ChatServer 告诉我们 FileServer 的 IP 和端口
    _pending_download_save_path = saveFilePath;

    // 2. 构造协议发给 ChatServer：申请下载该文件
    QJsonObject req;
    req["uid"] = UserMgr::getInstance()->GetUid();
    req["token"] = fileToken;       // 文件的下载凭据/标识
    req["filename"] = fileName;

    QJsonDocument doc(req);
    QByteArray sendData = doc.toJson(QJsonDocument::Compact);

    // 发给常驻聊天服务器（假设定义为 ID_DOWNLOAD_FILE_REQ 或你的下载申请协议）
    TcpMgr::getInstance()->sig_send_data(ReqId::ID_DOWNLOAD_FILE_REQ, sendData);
    qDebug() << "[ChatDialog] Requesting download ticket from ChatServer for token:" << fileToken;
}

void ChatDialog::slot_download_file(std::shared_ptr<FileToken> token)
{

    if (!token) {
        qWarning() << "[ChatDialog] Download FileToken is null!";
        return;
    }

    QString saveFilePath = _pending_download_save_path;
    if (saveFilePath.isEmpty()) {
        qWarning() << "[ChatDialog] Save file path is empty!";
        return;
    }

    qDebug() << "[ChatDialog] Starting download for:" << token->filename
             << "from" << token->host << ":" << token->port
             << "Total Size:" << token->filesz << "Bytes";

    FileDownloader *downloader=new FileDownloader(token->host,token->port,token->token,token->filesz,saveFilePath);
    QThread* thread = new QThread;
    downloader->moveToThread(thread);

    connect(thread, &QThread::started, downloader, &FileDownloader::StartDownloaded);
    connect(downloader, &FileDownloader::sig_finished, thread, &QThread::quit);
    connect(thread, &QThread::finished, downloader, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    // 3. 下载进度回调 -> 刷新 QML 进度条
    connect(downloader, &FileDownloader::sig_progress, this, [this](qint64 recv, qint64 total) {
        int percent = (total > 0) ? static_cast<int>((recv * 100) / total) : 0;
        auto fri_ptr = UserMgr::getInstance()->getFriend(_curr_chat_uid);
        QString friendName = fri_ptr ? fri_ptr->_name : "";
        emit _bridge->sig_file_upload_progress(friendName, percent);
        qDebug() << "[ChatDialog] Download Progress:" << percent << "% (" << recv << "/" << total << ")";
    }, Qt::QueuedConnection);

    // 4. 下载完成/失败回调
    connect(downloader, &FileDownloader::sig_finished, this, [this, saveFilePath](bool success, const QString& reason) {
        auto fri_ptr = UserMgr::getInstance()->getFriend(_curr_chat_uid);
        QString friendName = fri_ptr ? fri_ptr->_name : "";

        emit _bridge->sig_file_upload_complete(friendName, success, saveFilePath);

        if (success) {
            qDebug() << "[ChatDialog] File successfully saved to:" << saveFilePath;
        } else {
            qWarning() << "[ChatDialog] File download failed:" << reason;
        }
    }, Qt::QueuedConnection);

    // 5. 启动子线程开始拉流
    thread->start();


}

void ChatDialog::slot_load_chat_thread(bool load_more,int last_thread_id,std::vector<std::shared_ptr<ChatThreadInfo> > threads)
{
    for(auto& chat:threads){
        if(chat->_type=="group"){
            continue;
        }

        int uid_1,other_uid;
        uid_1=chat->_user1_id;
        int user_uid=UserMgr::getInstance()->GetUid();
        if(user_uid==uid_1){
            other_uid=chat->_user2_id;
        }
        else{
            other_uid=chat->_user1_id;
        }

        auto friend_info=UserMgr::getInstance()->getFriend(other_uid);
        if(!friend_info){
            continue;
        }

        qDebug() << "1. friend_info found:" << other_uid;
        ChatUserWid *wid = new ChatUserWid();
        qDebug() << "2. ChatUserWid created";
        auto friend_ptr = std::make_shared<UserInfo>(friend_info);
        qDebug() << "3. friend_ptr converted";
        wid->SetInfo(friend_ptr);
        wid->SetThreadInfo(chat);
        qDebug() << "4. SetInfo done";
        qDebug() << "5. Before ui->chat_user_list check:" << ui->chat_user_list;
        QListWidgetItem *item = new QListWidgetItem();
        item->setSizeHint(wid->sizeHint());
        ui->chat_user_list->addItem(item);
        ui->chat_user_list->setItemWidget(item, wid);
        _chat_thread_added[chat->_thread_id] = item;
        UserMgr::getInstance()->SetFriendThreadId(chat, other_uid);
        qDebug() << "6. Item added to chat_user_list done";
    }

    UserMgr::getInstance()->SetLastChatThreadId(last_thread_id);
    qDebug() << "7. Loop finished. Count:" << ui->chat_user_list->count();

    if(load_more){
        QJsonObject obj;
        obj["last_thread_id"]=last_thread_id;
        obj["uid"]=UserMgr::getInstance()->GetUid();

        QJsonDocument doc(obj);
        QByteArray data=doc.toJson(QJsonDocument::Compact);

        emit TcpMgr::getInstance()->sig_send_data(ReqId::ID_LOAD_CHAT_THREAD_REQ,data);
        return;
    }
    qDebug() << "8. Calling SetSelectedChatItem()";
    SetSelectedChatItem();
    qDebug() << "9. Calling SetSelectedChatPage()";
    SetSelectedChatPage();
    qDebug() << "10. Calling SetLoading(false)";
    SetLoading(false);
    qDebug() << "11. Calling loadChatMsg()";
    loadChatMsg();
    qDebug() << "12. slot_load_chat_thread finished successfully";


}

void ChatDialog::slot_create_private_chat(int id1, int id2, int thread_id)
{
    auto chat_wid=new ChatUserWid();
    int user_id=UserMgr::getInstance()->GetUid();
    int other_id;
    if(user_id==id1){
        other_id=id2;
    }
    else{
        other_id=id1;
    }
    auto info=UserMgr::getInstance()->getFriend(other_id);
    auto friend_info=std::make_shared<UserInfo>(info);
    chat_wid->SetInfo(friend_info);
    QListWidgetItem *item=new QListWidgetItem();
    item->setSizeHint(chat_wid->sizeHint());
    ui->chat_user_list->insertItem(0,item);
    ui->chat_user_list->setItemWidget(item,chat_wid);
    _chat_thread_added.insert(thread_id,item);

    auto thread_info=std::make_shared<ChatThreadInfo>();
    thread_info->_thread_id=thread_id;
    thread_info->_user1_id=user_id;
    thread_info->_user2_id=other_id;
    thread_info->_type="private";
    UserMgr::getInstance()->SetFriendThreadId(thread_info,other_id);

    ui->side_chat_lb->SetSelected(true);
    SetSelectedChatItem(info->_uid);
    SetSelectedChatPage(info->_uid);
    slot_side_chat();
    return;


}

void ChatDialog::slot_text_chat_msg_rsp(QString unique_id, int real_msg_id, int thread_id)
{
    auto thread_data=UserMgr::getInstance()->GetThreadData(thread_id);
    if(!thread_data){
        return;
    }
    //auto unrsp=thread_data->GetUnRspMsg(unique_id);
    thread_data->ConfirmMsg(unique_id,real_msg_id);
    if(_curr_thread_id==thread_id && _bridge){
        emit _bridge->sig_msg_send_success(unique_id,real_msg_id);
    }

}

void ChatDialog::slot_text_chat_msg_fail(QString unique_id)
{
    emit _bridge->sig_msg_send_fail(unique_id);
}


void ChatDialog::sendFileMsgToChatServer(std::shared_ptr<FileToken> token)
{
    // 上传到 FileServer 成功后，给 ChatServer 发一条消息，让 ChatServer 转发给对方
    QJsonObject fileDetail;
    fileDetail["filename"] = token->filename;
    fileDetail["filesz"]   = static_cast<qint64>(token->filesz);
    fileDetail["md5"]      = token->md5;
    fileDetail["token"]    = token->token;

    // 转为字符串作为 content
    QString contentStr = QString::fromUtf8(QJsonDocument(fileDetail).toJson(QJsonDocument::Compact));

    // 2. 组装发往 ChatServer 的顶层消息包
    int uuid = static_cast<int>(QDateTime::currentMSecsSinceEpoch() & 0x7FFFFFFF);
    QJsonObject msg_json;
    msg_json["from_uid"] = static_cast<qint64>(token->from_uid);
    msg_json["to_uid"]   = static_cast<qint64>(token->to_uid);
    msg_json["type"]     = "file";
    msg_json["content"]  = contentStr; // 核心：包含文件详情的 JSON 字符串
    msg_json["msg_id"]   = uuid;

    QJsonDocument doc(msg_json);
    TcpMgr::getInstance()->slot_send_data(ReqId::ID_TEXT_CHAT_MSG_REQ, doc.toJson(QJsonDocument::Compact));
}

void ChatDialog::updateFileProgressUI(int percent,int to_uid)
{
    auto fri_ptr=UserMgr::getInstance()->getFriend(to_uid);
    emit _bridge->sig_file_upload_progress(fri_ptr->_name,percent);
}

void ChatDialog::addInitialFileBubbleUi(std::shared_ptr<FileToken> token)
{
    auto fri_ptr=UserMgr::getInstance()->getFriend(token->to_uid);
    if (!fri_ptr) return;
    auto thread_id=UserMgr::getInstance()->GetFriendThreadId(fri_ptr->_uid);


    double sizeMb=token->filesz/(1024.0*1024.0);
    QString sizeStr = (sizeMb >= 1.0)
                          ? QString::number(sizeMb, 'f', 2) + " MB"
                          : QString::number(token->filesz / 1024.0, 'f', 1) + " KB";

    int uuid = static_cast<int>(QDateTime::currentMSecsSinceEpoch() & 0x7FFFFFFF);
    QString timeStr = QTime::currentTime().toString("hh:mm AP");
    int my_uid = UserMgr::getInstance()->GetUid();

    auto file_msg = std::make_shared<FileChatData>(
        uuid,
        thread_id,
        ChatFormType::PRIVATE,
        token->filename,
        token->filesz,
        token->token,
        token->md5,
        my_uid,
        0,          // status = 0 (上传/发送中)
        timeStr
        );

    auto thread_data = UserMgr::getInstance()->GetThreadData(thread_id);
    if (!thread_data) {
        thread_data = std::make_shared<ChatThreadData>(fri_ptr->_uid, thread_id, 0);
        UserMgr::getInstance()->AddThreadData(thread_id, thread_data);
    }
    thread_data->AddMsg(file_msg);


    auto iter=_chat_thread_added.find(thread_id);
    if (iter != _chat_thread_added.end()) {
        auto item=iter.value();
        auto wid = qobject_cast<ChatUserWid*>(ui->chat_user_list->itemWidget(iter.value()));
        if (wid) {
            wid->updateLastMsg(QString("[File] %1").arg(token->filename));
        }

        int row = ui->chat_user_list->row(item);
        if (row > 0) {
            ui->chat_user_list->takeItem(row);
            ui->chat_user_list->insertItem(0, item);
            ui->chat_user_list->setItemWidget(item, wid);
            ui->chat_user_list->setCurrentItem(item);
        }
    }

    emit _bridge->sig_new_file_arrive(
        fri_ptr->_name,
        token->filename,
        sizeStr,
        _pending_upload_path,
        timeStr,
        token->token
    );


}

void ChatDialog::SetLoading(bool now_loading)
{
    if(!now_loading){
        _b_loading=false;
    }
    qDebug()<<"load more contact user";
    if(_b_loading){
        return;
    }
    _b_loading=true;
    LoadingDialog* loading=new LoadingDialog(this,"Loading your contact");
    loading->setModal(true);
    loading->show();
    //LoadMoreContactUser();
    loading->deleteLater();
    _b_loading=false;
}

void ChatDialog::LoadChatList()
{

    int uid=UserMgr::getInstance()->GetUid();
    QJsonObject jsonObj;
    jsonObj["uid"]=uid;

    QJsonDocument doc(jsonObj);
    QByteArray sendStr=doc.toJson(QJsonDocument::Compact);

    emit TcpMgr::getInstance()->sig_send_data(ReqId::ID_LOAD_CHAT_THREAD_REQ,sendStr);
}

void ChatDialog::loadChatMsg()
{

    if (ui->chat_user_list->count() <= 0 || _curr_chat_uid <= 0) {
        qDebug() << "[ChatDialog::loadChatMsg] No active chat session to load.";
        return;
    }

    qDebug() << "[ChatDialog::loadChatMsg] Trigger loading initial messages for UID:" << _curr_chat_uid;
    int thread_id=UserMgr::getInstance()->GetFriendThreadId(_curr_chat_uid);
    if(thread_id<=0){
        qWarning() << "[ChatDialog::loadChatMsg] Failed to find thread_id for UID:" << _curr_chat_uid;
        return;
    }
    _curr_thread_id=thread_id;

    int my_uid = UserMgr::getInstance()->GetUid();

    auto friend_info = UserMgr::getInstance()->getFriend(_curr_chat_uid);
    QString friend_name = friend_info ? friend_info->_name : "";
    QString friend_icon = friend_info ? friend_info->_icon : "";


    // 2. 检查内存缓存：如果当前好友的消息已经存在于本地缓存中，直接提取给 QML
    if (UserMgr::getInstance()->HasThreadHistory(thread_id)) {
        auto thread_data = UserMgr::getInstance()->GetThreadData(thread_id);
        if(!thread_data)return;

        QVariantList history;
        const auto& msg_map = thread_data->GetMsgMapRef();
        for (const auto& msg : msg_map) {
            if (msg) {
                history.append(msg->toVariantMap(my_uid));
            }
        }
        // 通知 QML 页面刷新消息列表


        emit _bridge->sig_user_switched(friend_name, true, "today", friend_icon, history);
        return;
    }
    emit _bridge->sig_user_switched(friend_name, true, "today", friend_icon, QVariantList());

    // 3. 本地无缓存，向服务端发送分页拉取请求
    int self_uid = UserMgr::getInstance()->GetUid();

    // 初始化该好友的游标位置（从 0 开始表示拉取最新的第一页）
    int last_cursor=_thread_history_cursor.value(thread_id,0);


    QJsonObject root;
    root["uid"]         = self_uid;
    root["to_uid"]      = _curr_chat_uid;
    root["last_msg_id"] = last_cursor;                   // 游标为 0 代表最新消息起点
    root["pgsize"]      = MAX_COUNT_PER_PAGE;  // 每页条数（如 20）
    root["thread_id"] = thread_id;


    QJsonDocument doc(root);
    QByteArray sendData = doc.toJson(QJsonDocument::Compact);


    emit TcpMgr::getInstance()->sig_send_data(ReqId::ID_LOAD_CHAT_MSG_REQ, sendData);
}




void ChatDialog::ShowSearch(bool b_search)
{
    if(b_search){
        if (ui->stackedWidget->currentWidget() != ui->chat_widget) {
            ui->stackedWidget->setCurrentWidget(ui->chat_widget);
        }
        ui->chat_user_list->hide();
        ui->con_user_list->hide();
        ui->search_user_list->show();
        _mode = ChatUIMode::SearchMode;
    }else if(_state == ChatUIMode::ChatMode){
        ui->chat_user_list->show();
        ui->con_user_list->hide();
        ui->search_user_list->hide();
        _mode = ChatUIMode::ChatMode;
    }else if(_state == ChatUIMode::ContactMode){
        ui->chat_user_list->hide();
        ui->search_user_list->hide();
        ui->con_user_list->show();
        _mode = ChatUIMode::ContactMode;
    }
}

void ChatDialog::AddLBGroup(StateWidget *lb)
{
    _lb_list.push_back(lb);
}

void ChatDialog::LoadMoreChatItem()
{
    auto friend_list=UserMgr::getInstance()->GetChatListPerPage();
    if(!friend_list.empty()){
        for(auto& f:friend_list){
            int thread_id=UserMgr::getInstance()->GetFriendThreadId(f->_uid);
            auto iter=_chat_thread_added.find(thread_id);
            if(iter!=_chat_thread_added.end()){
                return;
            }
            auto* chat_user_wid=new ChatUserWid();
            std::shared_ptr<UserInfo> f_ele=std::make_shared<UserInfo>(f);
            chat_user_wid->SetInfo(f_ele);
            auto *item=new QListWidgetItem();
            item->setSizeHint(chat_user_wid->sizeHint());
            ui->chat_user_list->addItem(item);
            ui->chat_user_list->setItemWidget(item,chat_user_wid);
            _chat_thread_added[thread_id]=item;
        }
    }
    UserMgr::getInstance()->UpdateChatLoadedCount();
}

void ChatDialog::LoadMoreContactUser()
{
    auto con_list=UserMgr::getInstance()->GetConListPerPage();
    if(con_list.empty()){
        return;
    }
    for(auto con:con_list){
        auto* con_user=new ConUserItem();
        con_user->setInfo(con->_uid,con->_name,con->_icon);
        auto *item=new QListWidgetItem();
        item->setSizeHint(con_user->sizeHint());
        ui->con_user_list->addItem(item);
        ui->con_user_list->setItemWidget(item,con_user);
    }
    UserMgr::getInstance()->UpdateConLoadedCount();
}

bool ChatDialog::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type()==QEvent::MouseButtonPress){
        QMouseEvent* mEvent=static_cast<QMouseEvent*> (event);
        handleGlobalMousePress(mEvent);
    }
    return QDialog::eventFilter(watched,event);
}

void ChatDialog::handleGlobalMousePress(QMouseEvent *event)
{
    // ──► 🎯 安全防线 1：如果当前根本没在搜索，或者输入框与列表指针为空，直接拦截，杜绝闪退 ◄──
    if (_mode != ChatUIMode::SearchMode || !ui->search_user_list || !ui->search_edit) {
        return;
    }

    // ──► 🎯 安全防线 2：只有当搜索列表在屏幕上处于物理可见状态时，点击外面才有意义 ◄──
    if (!ui->search_user_list->isVisible()) {
        return;
    }

    // 1. 将物理全局坐标精准降维平移至搜索列表的局部视窗坐标中
    QPointF point = ui->search_user_list->mapFromGlobal(event->globalPosition());

    // 2. 检查点击是否逸出了搜索结果列表的物理矩形边缘
    bool clickedOutsideList = !ui->search_user_list->rect().contains(point.toPoint());

    // 3. 将物理全局坐标平移至上方搜索编辑框的局部视窗坐标中
    QPointF editPoint = ui->search_edit->mapFromGlobal(event->globalPosition());

    // 4. 检查点击是否逸出了输入框本尊的物理矩形边缘
    bool clickedOutsideEdit = !ui->search_edit->rect().contains(editPoint.toPoint());

    // ──► 🎯 终极隔离判定：只有当鼠标同时偏离了输入框和列表构成的“搜索生态圈”，才执行撤退 ◄──
    if (clickedOutsideList && clickedOutsideEdit) {
        qDebug() << "🖱️ [全局鼠标总线] 捕捉到生态圈外点击. 清洗搜索框并平滑退回主视图.";

        // 阻塞输入框文本改变信号的瞬间爆发（可选防护，防止两重踩踏）
        ui->search_edit->blockSignals(true);
        ui->search_edit->clear();
        ui->search_edit->blockSignals(false);

        // 彻底释放焦点，优雅触发外壳视窗切盘归位
        ui->search_edit->clearFocus();
        ShowSearch(false);
    }
}
void ChatDialog::slot_loading_user()
{
    if(_b_loading){
        return;
    }
    _b_loading=true;
    LoadingDialog* loading=new LoadingDialog(this,"Loading the user information");
    loading->setModal(true);
    loading->show();
    qDebug()<<"add new data to list";
    //this->AddUserlist();
    LoadMoreChatItem();
    loading->deleteLater();

    _b_loading=false;

}

void ChatDialog::slot_side_chat()
{
    qDebug() << "▶️ [侧边栏总线] 点击了【聊天】会话图标";
    if (!ui->search_edit->text().isEmpty()) {
        ui->search_edit->clear();
    }

    // 1. 互斥清理：清除除了【聊天】按钮外，其余所有标签的选中高亮状态
    ClearState(ui->side_chat_lb);

    // 2. 指针对齐切盘：根据树状图，将最外层 stackedWidget 的视窗直接翻转到聊天复合面板
    ui->stackedWidget->setCurrentWidget(ui->chat_widget);

    // 3. 状态机翻转对齐：两套状态变量必须严格咬合进入聊天流派
    _state = ChatUIMode::ChatMode;
    _mode  = ChatUIMode::ChatMode;

    // 4. 重置视图：安全隐藏或展现对应的搜索框与好友列表
    ShowSearch(false);
}

void ChatDialog::slot_side_setting()
{
    qDebug()<<"Change to side setting";
    ClearState(ui->side_setting);
    ui->stackedWidget->setCurrentWidget(ui->user_info_page);

}

void ChatDialog::slot_text_changed(const QString &str)
{
    if (!str.isEmpty()) {
        ShowSearch(true);
    } else {
        ShowSearch(false);
    }
}

void ChatDialog::slot_friend_apply(std::shared_ptr<AddFriendApply> add)
{
    qDebug()<<"Apply friend receive from user uid "<<add->_from_uid<<" name is "<<add->_name<<" desc is "<<add->_desc;
    bool success=UserMgr::getInstance()->has_added(add->_from_uid);
    if(success){
        qDebug()<<"Already been added";
        return;
    }
    else{
        UserMgr::getInstance()->add_apply(std::make_shared<ApplyInfo>(add));
    }
    ui->side_bar->show();
    ui->side_contact_lb->ShowRedPoint(true);
    ui->con_user_list->ShowRedPoint();
    ui->friend_apply_page->AddNewFriendApply(add);

}

void ChatDialog::slot_add_auth_friend(std::shared_ptr<AuthInfo>auth_info)
{
    // bool isFriend = UserMgr::getInstance()->CheckFriendById(auth_info->_uid);
    // if(isFriend){
    //     return;
    // }
    int thread_id=UserMgr::getInstance()->GetFriendThreadId(auth_info->_uid);
    auto iter = _chat_thread_added.find(thread_id);
    if (iter != _chat_thread_added.end()) {
        return; // 已经存在于左侧会话列表，直接返回
    }
    // 在 groupitem 之后插入新项


    auto *chat_user_wid = new ChatUserWid();
    auto user_info=std::make_shared<UserInfo>(auth_info);
    chat_user_wid->SetInfo(user_info);
    QListWidgetItem *item = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item->setSizeHint(chat_user_wid->sizeHint());

    ui->chat_user_list->insertItem(0,item);
    ui->chat_user_list->setItemWidget(item,chat_user_wid);
    _chat_thread_added.insert(thread_id,item);
}

void ChatDialog::slot_jump_chat_item(std::shared_ptr<SearchInfo> si)
{
    auto friend_info=std::make_shared<UserInfo>(si);
    slot_jump_chat_item_from_info_page(friend_info);

}

void ChatDialog::slot_auth_rsp(std::shared_ptr<AuthRsp> auth_rsp)
{
    if (!auth_rsp) return;

    qDebug() << "[slot_auth_rsp] friend auth success, UID:" << auth_rsp->_uid
             << "thread_id:" << auth_rsp->_thread_id;

    // 1. 防重复判断
    if (UserMgr::getInstance()->CheckFriendById(auth_rsp->_uid)) {
        return;
    }

    // 2. 本地内存添加好友关系（包括 UID <-> thread_id 映射）
    UserMgr::getInstance()->AddFriend(auth_rsp);

    // 3. 构建聚合根并写入初始打招呼/欢迎消息
    auto chat_thread_data = std::make_shared<ChatThreadData>(auth_rsp->_uid, auth_rsp->_thread_id, 0);
    UserMgr::getInstance()->AddThreadData(chat_thread_data, auth_rsp->_uid);

    for (auto& chat_msg : auth_rsp->_chat_datas) {
        if (chat_msg) {
            chat_thread_data->AppendMsg(chat_msg->GetMsgId(), chat_msg);
        }
    }

    // 4. 绑定 UI 并填写真实好友资料与聚合根
    auto* chat_user_wid = new ChatUserWid();
    auto user_info = std::make_shared<UserInfo>(auth_rsp);
    chat_user_wid->SetInfo(user_info);
    chat_user_wid->SetChatData(chat_thread_data); // 内部提取 preview 并显示

    // 5. 挂载到左侧列表首行并记录 Item 索引
    QListWidgetItem* item = new QListWidgetItem;
    item->setSizeHint(chat_user_wid->sizeHint());
    ui->chat_user_list->insertItem(0, item);
    ui->chat_user_list->setItemWidget(item, chat_user_wid);

    _chat_thread_added.insert(auth_rsp->_thread_id, item);
}

void ChatDialog::slot_side_contact()
{
    qDebug() << "▶️ [侧边栏总线] 点击了【通讯录】好友申请图标";

    // 1. ──► 🎯 核心修正 1：传入当前点击的 side_contact_lb 图标指针，让聊天图标退选 ◄──
    ClearState(ui->side_contact_lb);

    // 2. 指针对齐切盘：将主控视窗顺畅翻转到好友申请管理页
    ui->stackedWidget->setCurrentWidget(ui->friend_apply_page);

    // 3. ──► 🎯 核心修正 2：状态机变量必须在同一物理时间轴上精准翻转为通讯录模式 ◄──
    _state = ChatUIMode::ContactMode;
    _mode  = ChatUIMode::ContactMode;

    // 4. 重置视图：此时调用 ShowSearch 将会顺畅执行 ui->con_user_list->show() 逻辑
    ShowSearch(false);
}