#include "searchlist.h"
#include "tcpmgr.h"
#include "customizeedit.h"
#include "logindialog.h"
#include <QScrollBar>
#include "adduseritem.h"
#include "findsuccessdialog.h"

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
        qDebug() << "🚀 [SearchList] 激活网络加人弹窗流...";

        // ──► 🎯 核心修正 3：如果之前有残留窗口，先安全蒸发它，释放模态死锁 ◄──
        if (_find_dlg) {
            _find_dlg->deleteLater();
            _find_dlg = nullptr;
        }

        // 原生 new 分配内存，绑定 this 实现对象树联动
        _find_dlg = new FindSuccessDialog(this);

        // ──► 🎯 核心修正 4：绝杀黑魔法！命令窗口关闭时主动自我解构 ◄──
        _find_dlg->setAttribute(Qt::WA_DeleteOnClose);

        // 组装局部 Demo 回执资产
        auto si = std::make_shared<SearchInfo>(0, "llfc", "llfc", "hello , my friend!", 0);

        // ──► 🎯 核心修正 5：裸指针天然拥有具体子类视野，直接调用，告别繁琐的 dynamic_pointer_cast！ ◄──
        _find_dlg->SetSearchInfo(si);

        // 唤醒模态框呈现！
        _find_dlg->show();
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

}
