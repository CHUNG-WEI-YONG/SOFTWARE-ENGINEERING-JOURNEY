#include "searchlist.h"
#include "tcpmgr.h"
#include "customizeedit.h"
#include "logindialog.h"
#include <QScrollBar>
#include "adduseritem.h"
#include "findsuccessdialog.h"
#include "findfaileddialog.h"

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
    if(pending){
        _loadingDialog=new LoadingDialog;
        _loadingDialog->setModal(true);
        _loadingDialog->show();
        _search_pending=pending;
    }
    else{
        _loadingDialog->hide();
        _loadingDialog->deleteLater();
        _search_pending=pending;
    }
}

void SearchList::CloseFindDialog()
{
    // ──► 🎯 核心修正 2：用 deleteLater 彻底释放老窗口的操作系统句柄和模态锁 ◄──
    if (_find_dlg) {
        qDebug() << "📢 [SearchList] 正在调用 deleteLater() 异步安全销毁弹窗内存";
        _find_dlg->deleteLater();
        _find_dlg = nullptr; // 斩断野指针
    }
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
    if (!item) return;

    QWidget* widget = this->itemWidget(item);
    if (!widget) return;

    ListItemBase *custom_item = qobject_cast<ListItemBase*>(widget);
    if (!custom_item) return;

    auto itemtype = custom_item->getItem();
    if (itemtype == ListItemType::InvalidItem) return;

    if (itemtype == ListItemType::Add_User_Tip_Item) {
        if(_search_pending){
            return;
        }
        qDebug() << "🚀 [SearchList] 激活网络加人弹窗流...";

        // ──► 🎯 核心修正 3：如果之前有残留窗口，先安全蒸发它，释放模态死锁 ◄──
        if (_find_dlg) {
            _find_dlg->deleteLater();
            _find_dlg = nullptr;
        }

        auto search_edit=static_cast<CustomizeEdit*> (_search_edit);
        auto uid_Str=search_edit->text();
        QJsonObject obj;
        obj["uid"]=uid_Str;
        QJsonDocument doc(obj);
        QByteArray jsonData=doc.toJson(QJsonDocument::Compact);
        TcpMgr::getInstance()->sig_send_data(ReqId::ID_ADD_FRIEND_REQ,jsonData);


        return;
    }

    // 如果点击了其它非功能行，平滑关闭老弹窗
    CloseFindDialog();
}

// void SearchList::slot_item_clicked(QListWidgetItem *item)
// {
//     QWidget* widget=this->itemWidget(item);
//     if(!widget){
//         qDebug()<<"Widget is wrong pointer";
//         return;
//     }

//     ListItemBase *custom_item=qobject_cast<ListItemBase*>(widget);
//     if(!widget){
//         qDebug()<<"Widget is wrong pointer";
//         return;
//     }
//     auto itemtype=custom_item->getItem();
//     if(itemtype==ListItemType::InvalidItem){
//         qDebug()<<"Invalid item clicked";
//     }
//     if(itemtype==ListItemType::Add_User_Tip_Item){
//         // if(_search_pending){
//         //     return;
//         // }
//         // waitPending(true);
//         // auto search_edit=dynamic_cast<CustomizeEdit*>(_search_edit);
//         // auto uid_str=search_edit->text();

//         // QJsonObject obj;
//         // obj["uid"]=uid_str;
//         _find_dlg=std::make_shared<FindSuccessDialog>(this);
//         auto si = std::make_shared<SearchInfo>(0,"llfc","llfc","hello , my friend!",0);
//         (std::dynamic_pointer_cast<FindSuccessDialog>(_find_dlg))->SetSearchInfo(si);
//         _find_dlg->show();
//         return;

//     }

// }

void SearchList::slot_user_search(std::shared_ptr<SearchInfo> si)
{
    waitPending(false);
    if(si==nullptr){
        _find_dlg=std::make_shared<FindFailedDialog>(this);

    }
    else{
        //is friend
        //add friend
        //todo: search is friend or not
        _find_dlg=std::make_shared<FindSuccessDialog>(this);
        dynamic_pointer_cast<FindSuccessDialog>(_find_dlg)->SetSearchInfo(si);
    }
    _find_dlg->show();
}
