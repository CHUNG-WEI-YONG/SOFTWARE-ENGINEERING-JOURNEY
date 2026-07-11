#include "chatdialog.h"
#include "ui_chatdialog.h"
#include <QAction>
#include <QRandomGenerator>
#include <chatuserwid.h>
#include "loadingdialog.h"
#include <QWidget>
#include <QQmlContext>
#include "chatbridge.h"
ChatDialog::ChatDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ChatDialog),_mode(ChatUIMode::ChatMode),_state(ChatUIMode::ChatMode)
{
    ui->setupUi(this);
    ui->add_btn->SetState("normal","hover","press");
    ui->add_btn->setProperty("state","normal");
    ui->search_edit->SetMax(25);

    QAction* searchaction=new QAction(ui->search_edit);
    searchaction->setIcon(QIcon(":/rc/chat_picture/search.png"));
    ui->search_edit->addAction(searchaction,QLineEdit::LeadingPosition);
    ui->search_edit->setPlaceholderText(QStringLiteral("Search list"));

    QAction* clearaction=new QAction(ui->search_edit);
    clearaction->setIcon(QIcon(":/rc/chat_picture/close_transparent.png"));
    ui->search_edit->addAction(clearaction,QLineEdit::TrailingPosition);

    connect(ui->search_edit,&QLineEdit::textChanged,this,[clearaction](const QString &text){
        if(text.isEmpty()){
            clearaction->setIcon(QIcon(":/rc/chat_picture/close_transparent.png"));

        }
        else{
            clearaction->setIcon(QIcon(":/rc/chat_picture/close_search.png"));
        }
    });

    connect(clearaction,&QAction::triggered,this,[this,clearaction](){
        ui->search_edit->clear();
        clearaction->setIcon(QIcon(":/rc/chat_picture/close_transparent.png"));
        ui->search_edit->clearFocus();
        ShowSearch(false);
    });

    ChatBridge* bridge = new ChatBridge(this);
    ui->chat_quickwid->rootContext()->setContextProperty("cppBridge", bridge);
    ui->chat_quickwid->rootContext()->setContextProperty("currentChatUserName", "Select a friend...");
    //ChatPage* chat_page = new ChatPage(this);

    // // ui->widget_3 对应你在设计器里留出的右侧空白自适应底座
    // ui->chat_page->layout()->addWidget(chat_page);
    //int chatPageIndex = ui->chat_page->addWidget(chat_page);

    // 2. 物理激活，让栈容器立刻切到这一页展现出来（通常刚加进去的是第 0 页）
    //ui->chat_page->setCurrentIndex(chatPageIndex);

    QQmlContext* context = ui->chat_quickwid->rootContext();

    // 假设你当前点击的用户叫 "Chung Wei"（未来你可以把它换成从列表里动态获取的真实变量）
    QString currentUserName = "Chung Wei";

    // 🚀 将 C++ 的 QString 变量映射为 QML 里面可以直接认出的全局名字 "currentChatUserName"
    context->setContextProperty("currentChatUserName", currentUserName);

    // ⚠️ 注入完数据后，再加载 QML 文件！否则 QML 启动时找不到变量会爆红闪退
    ui->chat_quickwid->setSource(QUrl(QStringLiteral("qrc:/style/ChatPage.qml")));
    ui->chat_quickwid->setResizeMode(QQuickWidget::SizeRootObjectToView);
    // ui->chat_quickwid->setSource(QUrl(QStringLiteral("qrc:/style/ChatPage.qml")));



    // // 🛡️ 极其重要：让内部的 QML 画布自动跟随外层的 QStackedWidget 容器拉伸放大，锁死自适应
    // ui->chat_quickwid->setResizeMode(QQuickWidget::SizeRootObjectToView);
    ui->stackedWidget->setCurrentIndex(0);

    connect(ui->chat_user_list, &QListWidget::itemClicked, this, [this, bridge](QListWidgetItem *item){
        if (!item) return;

        // 直接抓取左侧被点击行的纯文本名字
        QString clickedName = item->text();
        if(clickedName.isEmpty()) {
            clickedName = item->data(Qt::DisplayRole).toString();
        }

        // 伪造在线状态（偶数行在线绿点，奇数行离线灰点）
        bool fakeOnline = (ui->chat_user_list->currentRow() % 2 == 0);

        // ──► 🎯 核心补全：在 C++ 临时捏造一个专属的历史记录数据盲包 ◄──
        QVariantList fakeHistory;

        QVariantMap msg1, msg2;
        msg1["sender"] = "other";
        msg1["message"] = QString("Hello! I am %1. Welcome to QML world!").arg(clickedName);
        fakeHistory.append(msg1);

        msg2["sender"] = "me";
        msg2["message"] = "Hi, glad to see you too.";
        fakeHistory.append(msg2);

        qDebug() << "🎛️ [C++ 联动器] 4个参数完美对齐，发射 sig_user_switched 信号给 QML！";

        // 🚀 跨越时空阻碍，轰鸣发射！参数数量、类型 100% 严丝合缝对齐！
        emit bridge->sig_user_switched(clickedName, fakeOnline, "10 mins ago", fakeHistory);
    });

    ShowSearch(false);

    connect(ui->chat_user_list,&ChatUserList::sig_loading_user,this,&ChatDialog::slot_loading_user);
    AddUserlist();

}

ChatDialog::~ChatDialog()
{
    delete ui;
}

void ChatDialog::AddUserlist()
{
    std::vector<QString>  strs ={"hello world !",
                                 "nice to meet u",
                                 "New year，new life",
                                 "You have to love yourself",
                                 "My love is written in the wind ever since the whole world is you"};

    std::vector<QString> heads = {
        ":/rc/chat_picture/head_1.jpg",
        ":/rc/chat_picture/head_2.jpg",
        ":/rc/chat_picture/head_3.jpg",
        ":/rc/chat_picture/head_4.jpg",
        ":/rc/chat_picture/head_5.jpg"
    };

    std::vector<QString> names = {
        "chung",
        "zack",
        "golang",
        "cpp",
        "java",
        "nodejs",
        "python",
        "rust"
    };

    for(int i = 0; i < 13; i++){
        int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
        int str_i = randomValue%strs.size();
        int head_i = randomValue%heads.size();
        int name_i = randomValue%names.size();

        auto *chat_user_wid = new ChatUserWid();
        chat_user_wid->SetInfo(names[name_i], heads[head_i], strs[str_i]);
        QListWidgetItem *item = new QListWidgetItem;
        //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
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
