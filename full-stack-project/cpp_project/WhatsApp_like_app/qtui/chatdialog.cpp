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

ChatDialog::ChatDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ChatDialog)
    , _mode(ChatUIMode::ChatMode)
    , _state(ChatUIMode::ChatMode)
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


    connect(ui->con_user_list,&ContactUserList::sig_switch_apply_friend_page,this,&ChatDialog::slot_text_changed);
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
    QQmlContext* context = ui->chat_quickwid->rootContext();
    QString currentUserName = UserMgr::getInstance()->returnName();
    QString myOwnLogoPath = "qrc:/rc/chat_picture/head_5.jpg";

    context->setContextProperty("cppBridge", bridge);
    //context->setContextProperty("currentChatUserName", "Select a friend...");
    context->setContextProperty("myOwnAvatar", myOwnLogoPath);
    context->setContextProperty("currentChatUserName", currentUserName);

    ui->chat_quickwid->setSource(QUrl(QStringLiteral("qrc:/style/ChatPage.qml")));
    ui->chat_quickwid->setResizeMode(QQuickWidget::SizeRootObjectToView);
    ui->stackedWidget->setCurrentIndex(0);

    connect(ui->chat_user_list, &QListWidget::itemClicked, this, [this, bridge](QListWidgetItem *item){
        if (!item) return;
        QWidget* widget = ui->chat_user_list->itemWidget(item);
        if (!widget) return;
        ChatUserWid* userWid = qobject_cast<ChatUserWid*>(widget);
        QString clickedName = userWid ? userWid->GetUserName() : item->data(Qt::DisplayRole).toString();
        if (clickedName.isEmpty()) clickedName = "Unknown User";

        QString clickedIcon = item->data(Qt::UserRole).toString();
        if (clickedIcon.isEmpty() && userWid) { clickedIcon = userWid->GetUserIcon(); }

        if (clickedIcon.contains(QLatin1String("qrc://rc/"))) {
            clickedIcon.replace(QLatin1String("qrc://rc/"), QLatin1String("qrc:/rc/"));
        } else if (clickedIcon.startsWith(QLatin1String(":/rc/"))) {
            clickedIcon.replace(QLatin1String(":/rc/"), QLatin1String("qrc:/rc/"));
        }
        if (clickedIcon.isEmpty() || !clickedIcon.startsWith(QLatin1String("qrc:/rc/"))) {
            clickedIcon = QStringLiteral("qrc:/rc/chat_picture/search.png");
        }

        bool fakeOnline = (ui->chat_user_list->currentRow() % 2 == 0);
        QVariantList fakeHistory; QVariantMap msg1, msg2;
        msg1["sender"] = "other"; msg1["type"] = "text";
        msg1["content"] = QString("Hello! I am %1. Welcome to QML world!").arg(clickedName);
        msg1["timeStr"] = "10:00 AM"; fakeHistory.append(msg1);
        msg2["sender"] = "me"; msg2["type"] = "text";
        msg2["content"] = "Hi, glad to see you too.";
        msg2["timeStr"] = "10:01 AM"; fakeHistory.append(msg2);

        emit bridge->sig_user_switched(clickedName, fakeOnline, QStringLiteral("10 mins ago"), clickedIcon, fakeHistory);
    });

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

    AddLBGroup(ui->side_chat_lb);
    AddLBGroup(ui->side_contact_lb);

    connect(ui->side_chat_lb,&StateWidget::clicked,this,&ChatDialog::slot_side_chat);
    connect(ui->side_contact_lb,&StateWidget::clicked,this,&ChatDialog::slot_side_contact);
    ShowSearch(false);

    QTimer::singleShot(0, this, [this](){
        ui->search_edit->clearFocus();
        this->setFocus();
    });
    this->installEventFilter(this);

    ui->search_user_list->SetSearchEdit(ui->search_edit);
    connect(TcpMgr::getInstance().get(),&TcpMgr::sig_friend_apply,this,&ChatDialog::slot_friend_apply);

}

ChatDialog::~ChatDialog()
{
    delete ui;
}

void ChatDialog::AddUserlist()
{
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
        chat_user_wid->SetInfo(names[name_i], baseHeadPath, strs[str_i]);

        // 💡 盲包也存冒号路径，保持实体属性的 100% 纯正统一
        QListWidgetItem *item = new QListWidgetItem;
        item->setData(Qt::UserRole, baseHeadPath);

        item->setSizeHint(chat_user_wid->sizeHint());
        ui->chat_user_list->addItem(item);
        ui->chat_user_list->setItemWidget(item, chat_user_wid);
    }
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
    this->AddUserlist();
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
    }
    else{
        UserMgr::getInstance()->add_apply(std::make_shared<ApplyInfo>(add));
    }
    ui->side_bar->show();
    ui->side_contact_lb->ShowRedPoint(true);
    ui->con_user_list->ShowRedPoint();
    ui->friend_apply_page->AddNewFriendApply(add);

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