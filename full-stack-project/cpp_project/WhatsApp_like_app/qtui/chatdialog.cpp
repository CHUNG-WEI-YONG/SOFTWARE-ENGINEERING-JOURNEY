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

}

ChatDialog::~ChatDialog()
{
    _timer->start(10000);
    delete ui;
}

void ChatDialog::AddUserlist()
{
    auto friend_list=UserMgr::getInstance()->GetChatListPerPage();
    if(!friend_list.empty()){
        for(auto& f:friend_list){
            auto chat_data=std::make_shared<ChatData>(f->_uid,f->_name,f->_icon);
            auto iter=_chat_items_added.find(f->_uid);
            if(iter!=_chat_items_added.end()){
                return;
            }
            auto* chat_user_wid=new ChatUserWid();
            std::shared_ptr<UserInfo> f_ele=std::make_shared<UserInfo>(f);
            chat_user_wid->SetInfo(f_ele);
            chat_user_wid->SetChatData(chat_data);
            auto *item=new QListWidgetItem();
            item->setSizeHint(chat_user_wid->sizeHint());
            ui->chat_user_list->addItem(item);
            ui->chat_user_list->setItemWidget(item,chat_user_wid);
            _chat_items_added[f->_uid]=item;
        }
    }
    UserMgr::getInstance()->UpdateChatLoadedCount();


    std::vector<QString> strs = { "hello world !", "nice to meet u", "New year，new life",
                                 "You have to love yourself", "My love is written in the wind ever since the whole world is you" };

    // ──► 🎯 核心修正 1：数据源彻底统一回传统的冒号格式，服务于 QWidget ◄──
    std::vector<QString> heads = {
        ":/rc/chat_picture/head_1.jpg",
        ":/rc/chat_picture/head_2.jpg",
        ":/rc/chat_picture/head_3.jpg",
        ":/rc/chat_picture/head_4.jpg",
        ":/rc/chat_picture/head_5.jpg"
    };

    std::vector<QString> names = { "chung", "zack", "golang", "cpp", "java", "nodejs", "python", "rust" };

    for(int i = 0; i < 13; i++){
        int randomValue = QRandomGenerator::global()->bounded(100);
        int str_i = randomValue % strs.size();
        int head_i = randomValue % heads.size();
        int name_i = randomValue % names.size();

        auto *chat_user_wid = new ChatUserWid();

        // 🚀 直接喂入冒号路径，左侧 ItemWidget 瞬间精准找到图片，不再裂开！
        QString baseHeadPath = heads[head_i];
        auto user_info=std::make_shared<UserInfo>(i,names[name_i],"","",0,heads[head_i],strs[str_i]);
        chat_user_wid->SetInfo(user_info);

        // 💡 盲包也存冒号路径，保持实体属性的 100% 纯正统一
        QListWidgetItem *item = new QListWidgetItem;
        item->setData(Qt::UserRole, baseHeadPath);

        item->setSizeHint(chat_user_wid->sizeHint());
        ui->chat_user_list->addItem(item);
        ui->chat_user_list->setItemWidget(item, chat_user_wid);
    }

    _timer=new QTimer(this);
    connect(_timer,&QTimer::timeout,this,[this](){
        auto user=UserMgr::getInstance()->GetUserInfo();
        QJsonObject obj;
        obj["from_uid"]=user->_uid;
        QJsonDocument doc(obj);
        QByteArray send=doc.toJson(QJsonDocument::Compact);
        emit TcpMgr::getInstance()->sig_send_data(ReqId::ID_HEART_BEAT_REQ,send);
    });

    _timer->start(10000);
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

void ChatDialog::SetSelectedChatPage(int uid)
{
    if(ui->chat_user_list->count()<=0){
        return;
    }
    if(uid==0){
        auto item=ui->chat_user_list->item(0);
        QWidget* widget=ui->chat_user_list->itemWidget(item);
        if(!widget){
            return;
        }

        auto con_item=qobject_cast<ChatUserWid*>(widget);
        if(!con_item){
            return;
        }

        std::shared_ptr<UserInfo> user=con_item->GetUserInfo();
        ui->chat_user_list->setCurrentItem(item);
        SwitchToUserChat(user);
        return;

    }

    auto iter=_chat_items_added.find(uid);
    if(iter==_chat_items_added.end()){
        return;
    }
    auto widget=iter.value();
    QWidget* w=ui->chat_user_list->itemWidget(widget);
    if(!w){
        return;
    }
    auto customItem=qobject_cast<ListItemBase*>(w);
    auto type=customItem->getItem();
    if(type==ListItemType::ChatUserItem){
        auto conn=qobject_cast<ChatUserWid*>(w);
        auto user=conn->GetUserInfo();
        SwitchToUserChat(user);
        return;
    }



}

void ChatDialog::SetSelectedChatItem(int uid)
{
    if(ui->chat_user_list->count()<=0){
        return;
    }

    if(uid==0){
        QListWidgetItem* item=ui->chat_user_list->item(0);
        ui->chat_user_list->setCurrentRow(0);
        if(item==nullptr){
            return;
        }
        QWidget *widget=ui->chat_user_list->itemWidget(item);
        if(!widget){
            return;
        }

        auto con_user=qobject_cast<ChatUserWid*>(widget);
        if(!con_user){
            return;
        }
        //_curr_chat_uid=con_user->GetUserInfo()->_uid;
    }

    for (int i = 0; i < ui->chat_user_list->count(); ++i) {
        QListWidgetItem* item = ui->chat_user_list->item(i);
        QWidget *widget = ui->chat_user_list->itemWidget(item);
        if (!widget) continue;

        auto chat_user = qobject_cast<ChatUserWid*>(widget);
        if (chat_user && chat_user->GetUserInfo()->_uid == uid) {
            ui->chat_user_list->setCurrentRow(i); // 选中该行
            _curr_chat_uid = uid;                 // 更新当前聊天 UID
            return;
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
    _curr_chat_uid = user->_uid;

    QVariantList history;

    // 1. 先判断是否有缓存，绝不在判断前提前调用 GetHistoryMsgs
    if (!UserMgr::getInstance()->hasHistoryCache(_curr_chat_uid)) {
        // 首次打开：初始化分页游标并触发异步拉取
        _user_history_cursor[_curr_chat_uid] = 0;

        // 🚀 恢复发射信号：通知后台线程或 SQLite 查询历史（先显示空历史，查完再追加）
        emit sig_load_user_history(_curr_chat_uid, 0, 20);
    } else {
        // 2. 命中内存缓存：安全提取数据并格式化
        auto history_cache = UserMgr::getInstance()->GetHistoryMsgs(_curr_chat_uid);
        for (const auto& msg : history_cache) {
            history.append(msg.toVariantMap(uid));
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
    LoadingDialog* loading=new LoadingDialog();
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
    auto iter=_chat_items_added.find(info->_uid);
    if(iter!=_chat_items_added.end()){
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

    _chat_items_added[info->_uid]=item;
    ui->side_chat_lb->SetSelected(true);
    SetSelectedChatItem(info->_uid);
    SetSelectedChatPage(info->_uid);
    slot_side_chat();

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
    int to_uid=friend_info->_uid;
    if(!friend_info){
        qDebug()<<"This friend not exist";
        return;
    }
    auto self_uid=UserMgr::getInstance()->GetUid();
    int last_msg_id = 0;
    if (_user_history_cursor.contains(to_uid)) {
        last_msg_id = _user_history_cursor[to_uid];
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

void ChatDialog::slot_load_history_finish(int from_uid, QList<ChatMsg> historyList, int next_last_msg_id)
{
    _user_history_cursor[from_uid]=next_last_msg_id;
    UserMgr::getInstance()->PrependHistoryBatch(from_uid,historyList);
    //auto cachedList=UserMgr::getInstance()->GetHistoryMsgs(from_uid);

    if(_curr_chat_uid!=from_uid){
        return;
    }
    int my_uid = UserMgr::getInstance()->GetUid();
    QVariantList qmlBatch;
    for (const auto& msg : historyList) {
        qmlBatch.append(msg.toVariantMap(my_uid));
    }
    emit _bridge->sig_append_history_batch(qmlBatch);


}

void ChatDialog::slot_send_msg(QString target, QString text)
{
    int my_uid = UserMgr::getInstance()->GetUid();
    int friend_uid = _curr_chat_uid; // The current active chat friend UID
    if (_curr_chat_uid == 0 || text.trimmed().isEmpty()) {
        return;
    }

    // 1. Build ChatMsg entity for yourself
    int uuid=static_cast<int>(QDateTime::currentMSecsSinceEpoch() & 0x7FFFFFFF);
    ChatMsg sent_msg;
    sent_msg.msg_id   = uuid; // Assigned by server or local DB increment
    sent_msg.from_uid = my_uid;
    sent_msg.to_uid   = friend_uid;
    sent_msg.type     = "text";
    sent_msg.content  = text;
    sent_msg.timeStr  = QTime::currentTime().toString("hh:mm AP");

    // 2. 🚀 Store inside the C++ _history_cache map!
    UserMgr::getInstance()->AppendHistoryMsg(friend_uid,sent_msg);
    //_history_cache[friend_uid].append(sent_msg);

    // 3. (Optional) Save to local SQLite database for persistence
    // SqliteMgr::getInstance()->saveMsg(sent_msg);

    // 4. Update left list preview snippet
    auto iter = _chat_items_added.find(friend_uid);
    if (iter != _chat_items_added.end()) {
        auto item=iter.value();
        auto wid = qobject_cast<ChatUserWid*>(ui->chat_user_list->itemWidget(iter.value()));
        if (wid) wid->updateLastMsg(text);

        int row=ui->chat_user_list->row(item);
        if(row>0){
            ui->chat_user_list->takeItem(row);
            ui->chat_user_list->insertItem(0, item);
            ui->chat_user_list->setItemWidget(item, wid);
            ui->chat_user_list->setCurrentItem(item);
        }
    }




    QJsonObject obj;
    obj["from_uid"]=my_uid;
    obj["to_uid"]=friend_uid;
    obj["type"]="text";
    obj["content"]=text;
    obj["msg_id"]=uuid;
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    emit TcpMgr::getInstance()->sig_send_data(ReqId::ID_TEXT_CHAT_MSG_REQ,data);
}

void ChatDialog::slot_text_chat_msg(std::shared_ptr<ChatMsg> msg)
{
    if (!msg) return;

    auto from_uid = msg->from_uid;
    UserMgr::getInstance()->AppendHistoryMsg(from_uid, *msg);

    auto iter = _chat_items_added.find(from_uid);
    ChatUserWid* chat_user_wid = nullptr;
    QListWidgetItem* item = nullptr;

    if (iter == _chat_items_added.end()) {
        qDebug() << "Friend message, add new chat item, id is " << from_uid;
        auto f = UserMgr::getInstance()->getFriend(from_uid);
        if (!f) {
            qDebug() << "Cannot find friend info for uid:" << from_uid;
            return;
        }

        auto chat_data = std::make_shared<ChatData>(f->_uid, f->_name, f->_icon);
        chat_user_wid = new ChatUserWid(); // 👈 修正：直接为外部指针赋值，避免变量遮蔽

        std::shared_ptr<UserInfo> f_user=std::make_shared<UserInfo>(f);
        chat_user_wid->SetInfo(f_user);
        chat_user_wid->SetChatData(chat_data);

        item = new QListWidgetItem();
        item->setSizeHint(chat_user_wid->sizeHint());
        ui->chat_user_list->insertItem(0, item); // 新来消息的好友条目建议直接插入最顶端 (Row 0)
        ui->chat_user_list->setItemWidget(item, chat_user_wid);
        _chat_items_added[from_uid] = item;
    } else {
        item = iter.value();
        QWidget* widget = ui->chat_user_list->itemWidget(item);
        chat_user_wid = qobject_cast<ChatUserWid*>(widget);
    }

    if (chat_user_wid) {
        QString previewText=(msg->type=="file")?QString("File %1").arg(msg->filename.isEmpty()?msg->content:msg->filename):msg->content;
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
        if(msg->type=="file"){
            QString sizeStr = msg->fileszStr;
            if (sizeStr.isEmpty()) {
                double sizeMb = msg->filesz / (1024.0 * 1024.0);
                sizeStr = (sizeMb >= 1.0)
                              ? QString::number(sizeMb, 'f', 2) + " MB"
                              : QString::number(msg->filesz / 1024.0, 'f', 1) + " KB";
            }

            QString filename=msg->filename.isEmpty()?msg->content:msg->filename;
            //emit _bridge->sig_new_file_arrive(sender_name,filename,sizeStr,"",msg->timeStr);
            emit _bridge->sig_new_file_arrive(
                sender_name,
                filename,
                sizeStr,
                "",          // 本地路径为空
                msg->timeStr,
                msg->token   // 👈 把消息里解析出的 msg->token 传给 QML！
                );
        }
        else{
            emit _bridge->sig_new_message_received(sender_name, msg->content);
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
    if(!fri_ptr)return;

    double sizeMb=token->filesz/(1024.0*1024.0);
    QString sizeStr = (sizeMb >= 1.0)
                          ? QString::number(sizeMb, 'f', 2) + " MB"
                          : QString::number(token->filesz / 1024.0, 'f', 1) + " KB";

    int uuid = static_cast<int>(QDateTime::currentMSecsSinceEpoch() & 0x7FFFFFFF);
    ChatMsg sent_msg;
    sent_msg.msg_id   = uuid;
    sent_msg.from_uid = UserMgr::getInstance()->GetUid();
    sent_msg.to_uid   = token->to_uid;
    sent_msg.type     = "file";
    sent_msg.content  = QString("[File] %1").arg(token->filename);
    sent_msg.timeStr  = QTime::currentTime().toString("hh:mm AP");
    UserMgr::getInstance()->AppendHistoryMsg(token->to_uid,sent_msg);

    auto iter=_chat_items_added.find(token->to_uid);
    if (iter != _chat_items_added.end()) {
        auto wid = qobject_cast<ChatUserWid*>(ui->chat_user_list->itemWidget(iter.value()));
        if (wid) wid->updateLastMsg(sent_msg.content);
    }

    emit _bridge->sig_new_file_arrive(
        fri_ptr->_name,
        token->filename,
        sizeStr,
        _pending_upload_path,
        sent_msg.timeStr,
        token->token
        );


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
            auto iter=_chat_items_added.find(f->_uid);
            if(iter!=_chat_items_added.end()){
                return;
            }
            auto* chat_user_wid=new ChatUserWid();
            std::shared_ptr<UserInfo> f_ele=std::make_shared<UserInfo>(f);
            chat_user_wid->SetInfo(f_ele);
            auto *item=new QListWidgetItem();
            item->setSizeHint(chat_user_wid->sizeHint());
            ui->chat_user_list->addItem(item);
            ui->chat_user_list->setItemWidget(item,chat_user_wid);
            _chat_items_added[f->_uid]=item;
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
    LoadingDialog* loading=new LoadingDialog(this);
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
    auto iter = _chat_items_added.find(auth_info->_uid);
    if (iter != _chat_items_added.end()) {
        return; // 已经存在于左侧会话列表，直接返回
    }
    // 在 groupitem 之后插入新项
    int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
    int str_i = randomValue%strs.size();
    int head_i = randomValue%heads.size();
    int name_i=randomValue%names.size();

    auto *chat_user_wid = new ChatUserWid();
    auto user_info=std::make_shared<UserInfo>(auth_info);
    chat_user_wid->SetInfo(user_info);
    QListWidgetItem *item = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item->setSizeHint(chat_user_wid->sizeHint());

    ui->chat_user_list->insertItem(0,item);
    ui->chat_user_list->setItemWidget(item,chat_user_wid);
    _chat_items_added.insert(auth_info->_uid,item);
}

void ChatDialog::slot_jump_chat_item(std::shared_ptr<SearchInfo> si)
{
    qDebug()<<"Slot jump chat item success";
    auto iter=_chat_items_added.find(si->_uid);
    if(iter!=_chat_items_added.end()){
        qDebug()<<"Jump to chat dialog id is "<<si->_uid;
        ui->chat_user_list->scrollToItem(iter.value());
        ui->side_chat_lb->SetSelected(true);
        SetSelectedChatItem(si->_uid);
        SetSelectedChatPage(si->_uid);
        slot_side_chat();
        return;
    }

    auto *chat_user_wid=new ChatUserWid();
    auto info=std::make_shared<UserInfo>(si);
    chat_user_wid->SetInfo(info);
    QListWidgetItem *item=new QListWidgetItem;
    item->setSizeHint(chat_user_wid->sizeHint());
    ui->chat_user_list->insertItem(0,item);
    ui->chat_user_list->setItemWidget(item,chat_user_wid);

    _chat_items_added[si->_uid]=item;
    ui->side_chat_lb->SetSelected(true);
    SetSelectedChatItem(si->_uid);
    SetSelectedChatPage(si->_uid);
    slot_side_chat();

}

void ChatDialog::slot_auth_rsp(std::shared_ptr<AuthRsp>auth_rsp)
{
    bool isFriend = UserMgr::getInstance()->CheckFriendById(auth_rsp->_uid);
    if(isFriend){
        return;
    }
    // 在 groupitem 之后插入新项
    int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
    int str_i = randomValue%strs.size();
    int head_i = randomValue%heads.size();
    int name_i=randomValue%names.size();

    auto *chat_user_wid = new ChatUserWid();
    auto user_info=std::make_shared<UserInfo>(auth_rsp);
    chat_user_wid->SetInfo(user_info);
    QListWidgetItem *item = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item->setSizeHint(chat_user_wid->sizeHint());

    ui->chat_user_list->insertItem(0,item);
    ui->chat_user_list->setItemWidget(item,chat_user_wid);
    _chat_items_added.insert(auth_rsp->_uid,item);
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