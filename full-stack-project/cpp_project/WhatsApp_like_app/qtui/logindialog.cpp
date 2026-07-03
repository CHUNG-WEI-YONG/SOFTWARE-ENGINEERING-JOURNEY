#include "logindialog.h"
#include "ui_logindialog.h"
#include "httpmgr.h"
#include <QMap>
#include "tcpmgr.h"

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    connect(ui->signin_btn,&QPushButton::clicked,this,&LoginDialog::switchRegister);
    ui->forgot_label->setState("normal","normal_hover","","selected","selected_hover","");
    connect(ui->forgot_label,&ClickedLabel::clicked,this,&LoginDialog::slot_forget_pwd);
    initHttpHandlers();
    connect(Httpmgr::getInstance().get(),&Httpmgr::sig_login_mod_finish,this,&LoginDialog::slot_login_mod_finish);

    connect(this,&LoginDialog::sig_connect_Tcp,TcpMgr::getInstance().get(),&TcpMgr::slot_tcp_connect);
    connect(TcpMgr::getInstance().get(),&TcpMgr::sig_con_success,this,&LoginDialog::slot_conn_success);
    connect(TcpMgr::getInstance().get(),&TcpMgr::sig_login_failed,this,&LoginDialog::slot_conn_failed);

}

LoginDialog::~LoginDialog()
{
    delete ui;
}

bool LoginDialog::CheckUserValid()
{
    auto email=ui->email_edit->text();
    if(email.isEmpty()){
        qDebug()<<"User enter empty email";
        AddTipErr(TipErr::TIP_EMAIL_ERR,tr("Email cannot be empty"));
        return false;
    }
    DelTipErr(TipErr::TIP_EMAIL_ERR);
    return true;
}

bool LoginDialog::CheckPasswdValid()
{
    std::string er;
    std::string passwd=ui->pass_edit->text().toStdString();
    bool result=ValidatePasswordStyle(passwd,er);
    if(!result){
        qDebug()<<"Error password";
        AddTipErr(TipErr::TIP_PWD_ERR,tr("Password style not correct"));
        return false;
    }
    DelTipErr(TipErr::TIP_PWD_ERR);
    return true;
}

void LoginDialog::slot_forget_pwd()
{
    qDebug()<<"slot forget password";
    emit switchReset();
}

void LoginDialog::on_login_btn_clicked()
{
    qDebug()<<"Log in button clicked";
    if(CheckUserValid()==false){
        return;
    }
    if(CheckPasswdValid()==false){
        return;
    }
    enableBtn(false);
    auto email=ui->email_edit->text();
    auto passwd=ui->pass_edit->text();
    QJsonObject root;
    root["email"]=email;
    root["passwd"]=xorString (passwd);
    Httpmgr::getInstance()->PostHttp(QUrl(gate_url_prefix+"/user_login"),root,ReqId::ID_LOGIN_USER,Modules::LOGINMOD);
}

void LoginDialog::slot_login_mod_finish(ReqId id, QString res, ErrorCode err)
{
    if(err!=ErrorCode::SUCCESS){
        showTip(tr("Connection Error"),false);
        return;
    }
    QJsonDocument jsonDoc=QJsonDocument::fromJson(res.toUtf8());
    if(jsonDoc.isNull()){
        showTip(tr("JSON Failed"),false);
        return;
    }

    if(!jsonDoc.isObject()){
        showTip(tr("JSON Failed"),false);
        return;
    }
    if(!_handlers.contains(id)){
        qWarning() << "💥 严重警告：登录调度雷达收到未注册的请求 ID 回执，拒绝分发！ID code:" << static_cast<int>(id);
        enableBtn(true);
        return;
    }
    _handlers[id](jsonDoc.object());
}

void LoginDialog::slot_conn_success(bool success)
{
    if(success){
        showTip(tr("Login Successful. Waiting to connect to server"),true);
        QJsonObject obj;
        obj["uid"]=_uid;
        obj["token"]=_token;
        QJsonDocument doc(obj);
        QString jsonString=doc.toJson(QJsonDocument::Indented);
        TcpMgr::getInstance()->sig_send_data(ReqId::ID_CHAT_LOGIN,jsonString);
    }
    else{
        showTip(tr("Wifi Error"),false);
        enableBtn(true);
    }
}

void LoginDialog::slot_conn_failed(int err)
{
    QString result=QString("Login failde , Error as %1").arg(err);
    showTip(result,false);
    enableBtn(true);
}

void LoginDialog::showTip(QString str,bool b_ok){
    if(b_ok){
        ui->error_label->setProperty("state","normal");
    }
    else{
        ui->error_label->setProperty("state","err");
    }
    ui->error_label->setText(str);
    repolish(ui->error_label);
}

void LoginDialog::enableBtn(bool enable)
{
    ui->login_btn->setEnabled(enable);
    ui->signin_btn->setEnabled(enable);
    return ;
}

void LoginDialog::initHttpHandlers()
{
    _handlers.insert(ReqId::ID_LOGIN_USER,[this](const QJsonObject& jsonObj){
        int error=jsonObj["error"].toInt();
        if(static_cast<ErrorCode>(error)!=ErrorCode::SUCCESS){
            qDebug()<<"JSON Parse error in Login.";
            showTip(tr("Something went wrong"),false);
            enableBtn(true);
            return ;
        }

        auto email=jsonObj["email"];
        Serverinfo si;
        si.uid=jsonObj["uid"].toVariant().toInt();
        si.host=jsonObj["host"].toString();
        si.port=jsonObj["port"].toString();
        si.token=jsonObj["token"].toString();

        _uid=si.uid;
        _token=si.token;
        qDebug()<<"Log in success.User uid is "<<_uid<<" .Token is "<<_token;
        emit sig_connect_Tcp(si);

    });
}

void LoginDialog::AddTipErr(TipErr te,QString tips){
    _tips_errors[te]=tips;
    showTip(tips,false);
}

void LoginDialog::DelTipErr(TipErr te){
    _tips_errors.remove(te);
    if(_tips_errors.empty()){
        ui->error_label->clear();
        return ;
    }
    showTip(_tips_errors.first(),false);

}


