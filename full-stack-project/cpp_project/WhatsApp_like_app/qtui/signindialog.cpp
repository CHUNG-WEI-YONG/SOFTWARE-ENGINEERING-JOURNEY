#include "signindialog.h"
#include "ui_signindialog.h"
#include "global.h"
#include "httpmgr.h"

SigninDialog::SigninDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SigninDialog)
{
    ui->setupUi(this);
    ui->error_label->setProperty("state","normal");
    repolish(ui->error_label);
    connect(Httpmgr::getInstance(),&Httpmgr::sig_reg_mod_finish,this,&SigninDialog::sig_reg_mod_finish);
}

SigninDialog::~SigninDialog()
{
    delete ui;
}


void SigninDialog::on_get_Code_clicked()
{
    auto email=ui->email_edit->text();
    QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
    bool match=regex.match(email).hasMatch();
    if(match){

    }
    else{
        showTip(tr("Wrong Email!"),true);
    }
}


void SigninDialog::showTip(QString str,bool b_ok){
    if(b_ok){
        ui->error_label->setProperty("state","normal");
    }
    else{
        ui->error_label->setProperty("state","err");
    }
    ui->error_label->setText(str);
    repolish(ui->error_label);
}

void SigninDialog::sig_reg_mod_finish(ReqId id, QString res, ErrorCode error)
{
    if(error!=ErrorCode::SUCCESS){
        showTip(tr("Request Error"),false);
        return;
    }
    QJsonDocument jsonDoc=QJsonDocument::fromJson(res.toUtf8());
    if(jsonDoc.isNull()){
        showTip(tr("Error in reading file"),false);
        return;
    }

}

