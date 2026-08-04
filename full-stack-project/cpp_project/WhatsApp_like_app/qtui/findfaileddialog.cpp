#include "findfaileddialog.h"
#include "ui_findfaileddialog.h"
#include <QDebug>
FindFailedDialog::FindFailedDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FindFailedDialog)
{
    ui->setupUi(this);
    setWindowTitle("Apply");
    setWindowFlags(windowFlags()|Qt::FramelessWindowHint);
    this->setObjectName("FindFailedDialog");
    ui->fail_sure_button->SetState("normal","hover","press");
    this->setModal(true);
}

FindFailedDialog::~FindFailedDialog()
{
    qDebug()<<"FindFailedDlg deleted";
    delete ui;
}

void FindFailedDialog::on_fail_sure_button_clicked()
{
    this->hide();
}

