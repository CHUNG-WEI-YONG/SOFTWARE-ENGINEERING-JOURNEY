#include "contactuserlist.h"
#include "listitembase.h"
#include "grouptipitem.h"
#include "conuseritem.h"
#include <QRandomGenerator>
#include "tcpmgr.h"


ContactUserList::ContactUserList(QWidget *parent):QListWidget(parent)
{
    //Q_UNUSED(parent);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 安装事件过滤器
    this->viewport()->installEventFilter(this);
    addContectUserList();
    connect(this, &QListWidget::itemClicked, this, &ContactUserList::slot_item_click);
    // connect(TcpMgr::getInstance.get(),&TcpMgr::sig_add_auth_friend,this,&ContactUserList::slot_add_auth_friend);
    // connect(TcpMgr::getInstance.get(),&TcpMgr::sig_auth_rsp,this,&ContactUserList::slot_auth_rsp);

}

void ContactUserList::ShowRedPoint(bool bshow)
{
    _add_friend_item->showRedPoint(bshow);
}

bool ContactUserList::eventFilter(QObject *watched, QEvent *event)
{
    if(watched==this->viewport()){
        if(event->type()==QEvent::Enter){
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        }
        else if(event->type()==QEvent::Leave){
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }
    }
    if (watched == this->viewport() && event->type() == QEvent::Wheel) {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
        int numDegrees = wheelEvent->angleDelta().y() / 8;
        int numSteps = numDegrees / 15; // 计算滚动步数

        // 设置滚动幅度
        this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() - numSteps);

        // 检查是否滚动到底部
        QScrollBar *scrollBar = this->verticalScrollBar();
        int maxScrollValue = scrollBar->maximum();
        int currentValue = scrollBar->value();
        //int pageSize = 10; // 每页加载的联系人数量

        if (maxScrollValue - currentValue <= 0) {
            // 滚动到底部，加载新的联系人
            qDebug()<<"load more contact user";
            //发送信号通知聊天界面加载更多聊天内容
            emit sig_loading_contact_user();
        }

        return true; // 停止事件传递
    }
    return QListWidget::eventFilter(watched , event);

}

void ContactUserList::addContectUserList()
{
    auto grouptip=new GroupTipItem();
    QListWidgetItem *item=new QListWidgetItem;
    item->setSizeHint(grouptip->sizeHint());
    this->addItem(item);
    this->setItemWidget(item,grouptip);
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);

    _add_friend_item=new ConUserItem();
    QListWidgetItem *con_item=new QListWidgetItem;
    _add_friend_item->setObjectName("new_Friend_item");
    _add_friend_item->setInfo(0,tr("New Friends "),":/rc/chat_picture/add_friend.png");
    _add_friend_item->SetItem(ListItemType::Apply_Friend_Item);
    con_item->setSizeHint(_add_friend_item->sizeHint());
    this->addItem(con_item);
    this->setItemWidget(con_item,_add_friend_item);

    this->setCurrentItem(con_item);

    auto groupcon=new GroupTipItem();
    groupcon->SetGroupTip("Your Contacts:");
    _group_item=new QListWidgetItem;
    _group_item->setSizeHint(grouptip->sizeHint());
    this->addItem(_group_item);
    this->setItemWidget(_group_item,groupcon);
    _group_item->setFlags(_group_item->flags() & ~Qt::ItemIsSelectable);

    for(int i = 0; i < 13; i++){
        int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
        int str_i = randomValue%strs.size();
        int head_i = randomValue%heads.size();
        int name_i = randomValue%names.size();

        auto *con_user_wid = new ConUserItem();
        con_user_wid->setInfo(0,names[name_i], heads[head_i]);
        QListWidgetItem *item = new QListWidgetItem;
        //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
        item->setSizeHint(con_user_wid->sizeHint());
        this->addItem(item);
        this->setItemWidget(item, con_user_wid);
    }
}

void ContactUserList::slot_item_click(QListWidgetItem *item)
{
    QWidget *wid=this->itemWidget(item);
    if(!wid){
        qDebug()<< "slot item clicked widget is nullptr";
        return;
    }
    ListItemBase* list=qobject_cast<ListItemBase*>(wid);
    if(!list){
        qDebug()<< "slot item clicked widget is nullptr";
        return;
    }
    auto type=list->getItem();
    if(type==ListItemType::InvalidItem||type==ListItemType::Group_Tip_Item){
        qDebug()<<"Invalid item clicked";
        return;
    }
    else if(type==ListItemType::Apply_Friend_Item){
        qDebug()<<"Apply friend item clicked";
        emit sig_switch_apply_friend_page();
        return;
    }
    else if(type==ListItemType::ContactUserItem){
        qDebug()<< "contact user item clicked ";
        //跳转到好友申请界面
        emit sig_switch_friend_info_page();
        return;
    }
}
