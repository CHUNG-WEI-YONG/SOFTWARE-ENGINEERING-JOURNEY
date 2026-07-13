#include "searchlist.h"
#include "tcpmgr.h"
#include "customizeedit.h"
#include "logindialog.h"
#include <QScrollBar>
#include "adduseritem.h"

SearchList::SearchList(QWidget *parent):QListWidget(parent),_find_dlg(nullptr),_search_edit(nullptr),_search_pending(false){
    Q_UNUSED(parent);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->viewport()->installEventFilter(this);
    connect(this,&QListWidget::itemClicked,this,&SearchList::slot_item_clicked);
    AddTipItem();
    connect(TcpMgr::getInstance().get(),&TcpMgr::sig_user_search,this,&SearchList::slot_user_search);
}

void SearchList::CloseSearch()
{

}

void SearchList::SetSearchEdit(QWidget *edit)
{

}

void SearchList::waitPending(bool pending)
{

}

void SearchList::AddTipItem()
{
    auto * invalid_item=new QWidget();
    QListWidgetItem *item_temp=new QListWidgetItem();
    item_temp->setSizeHint(QSize(250,10));
    this->addItem(item_temp);
    invalid_item->setObjectName("invalid_item");
    this->setItemWidget(item_temp, invalid_item);
    item_temp->setFlags(item_temp->flags() & ~Qt::ItemIsSelectable);

    auto *new_user_item=new AddUserItem();
    QListWidgetItem *list=new QListWidgetItem();
    list->setSizeHint(new_user_item->sizeHint());
    this->addItem(list);
    this->setItemWidget(list,new_user_item);



}

void SearchList::slot_item_clicked(QListWidgetItem *item)
{

}

void SearchList::slot_user_search(std::shared_ptr<SearchInfo> si)
{

}
