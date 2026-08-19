#include "applyfrienditem.h"
#include "ui_applyfrienditem.h"
#include <QStyleOption>
#include <QPainter>

ApplyFriendItem::ApplyFriendItem(QWidget *parent) :
    ListItemBase(parent), _added(false),
    ui(new Ui::ApplyFriendItem)
{
    ui->setupUi(this);
    SetItem(ListItemType::Apply_Friend_Item);
    ui->addBtn->SetState("normal","hover", "press");
    ui->addBtn->hide();
    connect(ui->addBtn, &ClickedButton::clicked,  [this](){
        qDebug()<<"Apply friend item has apply_info is "<<_apply_info->_uid;
        emit this->sig_auth_friend(_apply_info);
    });
    //connect
}

ApplyFriendItem::~ApplyFriendItem()
{
    delete ui;
}

void ApplyFriendItem::SetInfo(std::shared_ptr<ApplyInfo> apply_info)
{
    _apply_info=apply_info;
    QPixmap pix(_apply_info->_icon);
    ui->icon_lb->setPixmap(pix.scaled(ui->icon_lb->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);

    ui->user_name_lb->setText(_apply_info->_name);
    ui->user_chat_lb->setText(_apply_info->_desc);
}

void ApplyFriendItem::ShowAddBtn(bool bshow)
{
    if(bshow){
        ui->addBtn->show();
        _added=false;
        ui->already_add_lb->hide();
    }
    else{
        ui->addBtn->hide();
        _added=true;
        ui->already_add_lb->show();
    }
}

int ApplyFriendItem::GetUid()
{
    return _apply_info->_uid;
}

void ApplyFriendItem::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    // 必须要显式调用基类的绘制，确保继承链完整
    ListItemBase::paintEvent(event);
}

void ApplyFriendItem::slot_apply_sure()
{
    ShowAddBtn(false);
}
