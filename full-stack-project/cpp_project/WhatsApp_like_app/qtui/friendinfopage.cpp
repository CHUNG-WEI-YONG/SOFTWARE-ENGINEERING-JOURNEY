#include "friendinfopage.h"
#include "ui_friendinfopage.h"

FriendInfoPage::FriendInfoPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FriendInfoPage)
{
    this->setObjectName("friend_info_page");
    ui->setupUi(this);
    ui->phone_call->SetState("normal","hover","press");
    ui->video_call->SetState("normal","hover","press");
    ui->msg_chat->SetState("normal","hover","press");
}

FriendInfoPage::~FriendInfoPage()
{
    delete ui;
}

void FriendInfoPage::SetInfo(std::shared_ptr<UserInfo> si)
{
    _user_info=si;
    QPixmap pixmap(si->_icon);
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);

    ui->name_tip->setText(si->_name);
    ui->back_tip->setText(si->_desc);
    ui->nick_tip->setText(si->_nick);
}

void FriendInfoPage::on_msg_chat_clicked()
{
    emit sig_jump_chat_item(_user_info);
}

