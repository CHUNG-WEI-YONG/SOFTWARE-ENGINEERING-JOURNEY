#include "conuseritem.h"
#include "ui_conuseritem.h"

ConUserItem::ConUserItem(QWidget *parent)
    : ListItemBase(parent)
    , ui(new Ui::ConUserItem)
{
    ui->setupUi(this);
    SetItem(ListItemType::ContactUserItem);
    ui->red_point_lb->raise();
    showRedPoint(false);

}

ConUserItem::~ConUserItem()
{
    delete ui;
}

void ConUserItem::setInfo(std::shared_ptr<AuthInfo> auth)
{
    _info=std::make_shared<UserInfo>(auth);
    QPixmap pix(_info->_icon);
    ui->icon_lb->setPixmap(pix.scaled(ui->icon_lb->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);
    ui->user_name_lb->setText(_info->_name);
}

void ConUserItem::setInfo(std::shared_ptr<AuthRsp> auth_rsp)
{
    _info=std::make_shared<UserInfo>(auth_rsp);
    QPixmap pix(_info->_icon);
    ui->icon_lb->setPixmap(pix.scaled(ui->icon_lb->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);
    ui->user_name_lb->setText(_info->_name);
}

void ConUserItem::setInfo(int uid, QString name, QString icon)
{
    _info = std::make_shared<UserInfo>(uid,name, name, icon, 0);

    // 加载图片
    QPixmap pixmap(_info->_icon);

    // 设置图片自动缩放
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);

    ui->user_name_lb->setText(_info->_name);
}

void ConUserItem::showRedPoint(bool bshow)
{
    if(bshow){
        ui->red_point_lb->show();
    }
    else{
        ui->red_point_lb->hide();
    }
}


QSize ConUserItem::sizeHint() const
{
    return QSize(250, 70); // 返回自定义的尺寸
}
