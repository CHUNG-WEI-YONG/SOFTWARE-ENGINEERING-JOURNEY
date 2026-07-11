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

    QAction* clearaction = new QAction(ui->search_edit);
    clearaction->setIcon(QIcon(":/rc/chat_picture/close_transparent.png"));
    ui->search_edit->addAction(clearaction, QLineEdit::TrailingPosition);

    connect(ui->search_edit, &QLineEdit::textChanged, this, [clearaction](const QString &text){
        if(text.isEmpty()){
            clearaction->setIcon(QIcon(":/rc/chat_picture/close_transparent.png"));
        } else {
            clearaction->setIcon(QIcon(":/rc/chat_picture/close_search.png"));
        }
    });

    // 4. 点击清除按钮重置搜索框
    connect(clearaction, &QAction::triggered, this, [this, clearaction](){
        ui->search_edit->clear();
        clearaction->setIcon(QIcon(":/rc/chat_picture/close_transparent.png"));
        ui->search_edit->clearFocus();
        ShowSearch(false);
    });

    // 5. 实例化 QML 通信桥梁与上下文环境
    ChatBridge* bridge = new ChatBridge(this);
    QQmlContext* context = ui->chat_quickwid->rootContext();

    // 从单例中获取当前登录用户的资产信息
    QString currentUserName = UserMgr::getInstance()->returnName();
    QString myOwnLogoPath = "qrc:/rc/chat_picture/head_5.jpg";

    // 6. 核心注入：必须在 setSource 之前配置全局变量，防止 QML 第一帧爆红闪退
    context->setContextProperty("cppBridge", bridge);
    context->setContextProperty("currentChatUserName", "Select a friend...");
    context->setContextProperty("myOwnAvatar", myOwnLogoPath);
    context->setContextProperty("currentChatUserName", currentUserName);

    // 7. 加载 QML 视图画布并设置尺寸自适应
    ui->chat_quickwid->setSource(QUrl(QStringLiteral("qrc:/style/ChatPage.qml")));
    ui->chat_quickwid->setResizeMode(QQuickWidget::SizeRootObjectToView);
    ui->stackedWidget->setCurrentIndex(0);

    // 8. 核心事件总线：监听左侧好友列表点击，分发历史记录给 QML
    connect(ui->chat_user_list, &QListWidget::itemClicked, this, [this, bridge](QListWidgetItem *item){
        if (!item) return;

        // 提取挂载在该行的自定义用户小项 (ChatUserWid)
        QWidget* widget = ui->chat_user_list->itemWidget(item);
        if (!widget) return;

        ChatUserWid* userWid = qobject_cast<ChatUserWid*>(widget);

        // 提取目标联系人的姓名
        QString clickedName = userWid ? userWid->GetUserName() : item->data(Qt::DisplayRole).toString();
        if (clickedName.isEmpty()) clickedName = "Unknown User";

        // 优先从隐藏打包的 UserRole 盲包里打捞我们在 AddUserlist 存入的原始路径
        QString clickedIcon = item->data(Qt::UserRole).toString();
        if (clickedIcon.isEmpty() && userWid) {
            clickedIcon = userWid->GetUserIcon();
        }

        // ──► 🎯 终极全量路径清洗总线：彻底消灭双斜杠与裂开隐患 ◄──
        // 1. 如果格式已经变成了扭曲的双斜杠 "qrc://rc/"，强行收缩为标准的单斜杠 "qrc:/rc/"
        if (clickedIcon.contains(QLatin1String("qrc://rc/"))) {
            clickedIcon.replace(QLatin1String("qrc://rc/"), QLatin1String("qrc:/rc/"));
        }
        // 2. 如果是以传统的冒号开头 ":/rc/"，精准平替为 QML 的 "qrc:/rc/"
        else if (clickedIcon.startsWith(QLatin1String(":/rc/"))) {
            clickedIcon.replace(QLatin1String(":/rc/"), QLatin1String("qrc:/rc/"));
        }
        // 3. 兜底防御：如果是常规冒号 ":/"，进行安全前缀重组并剔除可能产生的双斜杠
        else if (clickedIcon.startsWith(QLatin1String(":/"))) {
            clickedIcon.replace(0, 1, QLatin1String("qrc:/"));
            if (clickedIcon.startsWith(QLatin1String("qrc://"))) {
                clickedIcon.replace(QLatin1String("qrc://"), QLatin1String("qrc:/"));
            }
        }

        // 终极锁死防线：必须确保以标准的 qrc:/rc/ 开头
        if (clickedIcon.isEmpty() || !clickedIcon.startsWith(QLatin1String("qrc:/rc/"))) {
            clickedIcon = QStringLiteral("qrc:/rc/chat_picture/search.png");
        }

        // 模拟在线/离线双态驱动 (偶数在线，奇数离线)
        bool fakeOnline = (ui->chat_user_list->currentRow() % 2 == 0);

        // 组装符合 QML 渲染引擎蓝图的模拟历史消息盲包
        QVariantList fakeHistory;
        QVariantMap msg1, msg2;

        msg1["sender"] = "other";
        msg1["type"] = "text";
        msg1["content"] = QString("Hello! I am %1. Welcome to QML world!").arg(clickedName);
        msg1["timeStr"] = "10:00 AM";
        fakeHistory.append(msg1);

        msg2["sender"] = "me";
        msg2["type"] = "text";
        msg2["content"] = "Hi, glad to see you too.";
        msg2["timeStr"] = "10:01 AM";
        fakeHistory.append(msg2);

        qDebug() << "🎛️ [C++ 联动器] 5个参数对齐发射. 最终清洗完成路径:" << clickedIcon;

        // 发射完美对齐的信号
        emit bridge->sig_user_switched(clickedName, fakeOnline, QStringLiteral("10 mins ago"), clickedIcon, fakeHistory);
    });

    // 9. 触发列表初次装载与滚动条触底懒加载监听
    ShowSearch(false);
    connect(ui->chat_user_list, &ChatUserList::sig_loading_user, this, &ChatDialog::slot_loading_user);
    AddUserlist();
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

void ChatDialog::ShowSearch(bool b_search)
{
    if(b_search){
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
