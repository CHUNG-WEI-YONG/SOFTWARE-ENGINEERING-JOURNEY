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
    auto httpmgr_instance=Httpmgr::getInstance();
    connect(httpmgr_instance.get(),&Httpmgr::sig_reg_mod_finish,this,&SigninDialog::sig_reg_mod_finish);
    initHttpHandlers();
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
        QJsonObject json_obj;
        json_obj["email"]=email;
        json_obj["key"] = "2026_safe_handshake";
        Httpmgr::getInstance()->PostHttp(QUrl(gate_url_prefix+"/get_verifycode"),json_obj,ReqId ::ID_GET_VERIFY_CODE,Modules::REGISTERMOD);
        showTip(tr("Code send to your email.Please check"),true);

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

    jsonDoc.object();
    if(!_handlers.contains(id)){
        qWarning() << "警告：收到了未注册的请求 ID 回执，调度雷达拒绝分发！ID code:" << static_cast<int>(id);
        return;
    }
    _handlers[id](jsonDoc.object());
    return;
}

void SigninDialog::initHttpHandlers()
{
//register
    _handlers.insert(ReqId::ID_GET_VERIFY_CODE,[this](const QJsonObject& JsonObj){
    int error=JsonObj["error"].toInt();
    if(static_cast<ErrorCode>(error)!=ErrorCode::SUCCESS){
        showTip(tr("Error in argument"),false);
        return;
    }
    auto email=JsonObj["email"].toString();
    showTip(tr("Code sent to email"),true);
    qDebug()<<"email is"<<email;
});
}

