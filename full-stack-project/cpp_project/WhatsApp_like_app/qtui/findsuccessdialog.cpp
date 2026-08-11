#include "findsuccessdialog.h"
#include "ui_findsuccessdialog.h"
#include <QDir>
#include "applyfriend.h"

FindSuccessDialog::FindSuccessDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FindSuccessDialog),
    _parent(parent)
{
    ui->setupUi(this);
    setWindowTitle("Add");
    setWindowFlags(windowFlags()|Qt::FramelessWindowHint);
    QString app_path=QCoreApplication::applicationDirPath();
    QString pix_path=QDir::toNativeSeparators(app_path+QDir::separator()+"static"+QDir::separator()+"head_4.jpg");
    QPixmap head_pix(pix_path);
    head_pix = head_pix.scaled(ui->head_lb->size(),
                               Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->head_lb->setPixmap(head_pix);
    ui->add_new_friend_btn->SetState("normal","hover","press");
    this->setModal(true);
}

FindSuccessDialog::~FindSuccessDialog()
{
    qDebug()<<"FindSuccessDialog deleted";
    delete ui;
}

void FindSuccessDialog::SetSearchInfo(std::shared_ptr<SearchInfo> si)
{
    ui->name_lb->setText(si->_name);
    _si = si;
}

void FindSuccessDialog::on_add_new_friend_btn_clicked()
{
    this->hide();
    auto friend_pg=new ApplyFriend(_parent);
    friend_pg->SetSearchInfo(_si);
    qDebug() << "FindSuccess Dlg received target UID:" << _si->_uid;
    friend_pg->setModal(true);
    friend_pg->show();
}

