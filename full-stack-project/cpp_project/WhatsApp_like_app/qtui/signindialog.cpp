#include "signindialog.h"
#include "ui_signindialog.h"
#include "global.h"
#include "httpmgr.h"

SigninDialog::SigninDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SigninDialog)
    ,_countdown(5)
{
    ui->setupUi(this);
    ui->error_label->setProperty("state","normal");
    repolish(ui->error_label);
    auto httpmgr_instance=Httpmgr::getInstance();
    connect(httpmgr_instance.get(),&Httpmgr::sig_reg_mod_finish,this,&SigninDialog::sig_reg_mod_finish);
    initHttpHandlers();
    ui->error_label->clear();

    // connect(ui->user_edit,&QLineEdit::editingFinished(),this,[this](){
    //     checkUserValid();
    // });
    connect(ui->user_edit,&QLineEdit::editingFinished,this,[this](){
        checkUserValid();
    });
    connect(ui->email_edit, &QLineEdit::editingFinished, this, [this](){
        //on_get_Code_clicked();
        checkEmailValid();
    });
    connect(ui->pass_edit, &QLineEdit::editingFinished, this, [this](){
        checkPassValid();
    });
    connect(ui->repass_edit, &QLineEdit::editingFinished, this, [this](){
        checkPassValid();
    });
    connect(ui->code_edit, &QLineEdit::editingFinished, this, [this](){
        checkVarifyValid();
    });
    ui->pass_visible->setCursor(Qt::PointingHandCursor);
    ui->repass_visible->setCursor(Qt::PointingHandCursor);
    ui->pass_visible->setState("unvisible","unvisible_hover","","visible",
                               "visible_hover","");
    ui->repass_visible->setState("unvisible","unvisible_hover","","visible",
                                  "visible_hover","");

    connect(ui->pass_visible,&ClickedLabel::clicked,this,[this](){
        auto state=ui->pass_visible->GetCurState();
        if(state==ClickLbState::Normal){
            ui->pass_edit->setEchoMode(QLineEdit::Password);
        }
        else{
            ui->pass_edit->setEchoMode(QLineEdit::Normal);
        }
    });

    connect(ui->repass_visible,&ClickedLabel::clicked,this,[this](){
        auto state=ui->repass_visible->GetCurState();
        if(state==ClickLbState::Normal){
            ui->repass_edit->setEchoMode(QLineEdit::Password);
        }
        else{
            ui->repass_edit->setEchoMode(QLineEdit::Normal);
        }
    });

    _countdown_timer=new QTimer(this);
    connect(_countdown_timer,&QTimer::timeout,[this](){
        if(_countdown==0){
            _countdown_timer->stop();
            emit sigSwitchLogin();
            return ;
        }
        else{
            _countdown--;
            auto str=QString("Sign in Successful. Return to login page in %1 s").arg(_countdown);
            ui->tip2_lb->setText(str);
        }
    });
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
        DelTipErr(TipErr::TIP_EMAIL_ERR);

    }
    else{
        AddTipErr(TipErr::TIP_EMAIL_ERR,"Wrong Email!");
        //showTip(tr("Wrong Email!"),true);
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
    qDebug() << "response id =" << int(id);
    qDebug() << "_handlers contains =" << _handlers.contains(id);
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

    _handlers.insert(ReqId::ID_REG_USER,[this](const QJsonObject& JsonObj){
        int error = JsonObj["error"].toInt();
        if(error != int(ErrorCode::SUCCESS)){
            showTip(tr("Error in register user"),false);
            return;
        }
        auto email = JsonObj["email"].toString();
        showTip(tr("User registered successfully"), true);
        qDebug()<<"user uid is "<<JsonObj["uid"].toString();
        qDebug()<< "email is " << email ;
        changeTipPage();
    });
}


void SigninDialog::on_sure_btn_clicked()
{
    if(ui->user_edit->text() == ""){
        showTip(tr("用户名不能为空"), false);
        return;
    }
    if(ui->email_edit->text() == ""){
        showTip(tr("邮箱不能为空"), false);
        return;
    }
    if(ui->pass_edit->text() == ""){
        showTip(tr("密码不能为空"), false);
        return;
    }
    std::string ec;
    if(!ValidatePasswordStyle(ui->pass_edit->text().toStdString(),ec)){
        showTip(QString::fromStdString(ec),false);
        return;
    }
    if(ui->repass_edit->text() == ""){
        showTip(tr("确认密码不能为空"), false);
        return;
    }
    if(ui->repass_edit->text() != ui->pass_edit->text()){
        showTip(tr("密码和确认密码不匹配"), false);
        return;
    }
    if(ui->code_edit->text() == ""){
        showTip(tr("验证码不能为空"), false);
        return;
    }
    //day11 发送http请求注册用户
    QJsonObject json_obj;
    json_obj["user"] = ui->user_edit->text();
    json_obj["email"] = ui->email_edit->text();
    json_obj["passwd"] = xorString(ui->pass_edit->text());
    json_obj["confirm"] = xorString(ui->repass_edit->text());
    json_obj["varifycode"] = ui->code_edit->text();
    Httpmgr::getInstance()->PostHttp(QUrl(gate_url_prefix+"/register_user"),
                                    json_obj, ReqId::ID_REG_USER,Modules::REGISTERMOD);

}

void SigninDialog::AddTipErr(TipErr te,QString tips){
    _tips_errors[te]=tips;
    showTip(tips,false);
}

void SigninDialog::DelTipErr(TipErr te){
    _tips_errors.remove(te);
    if(_tips_errors.empty()){
        ui->error_label->clear();
        return ;
    }
    showTip(_tips_errors.first(),false);

}

void SigninDialog::changeTipPage()
{
    _countdown_timer->stop();
    ui->stackedWidget->setCurrentWidget(ui->page_2);
    _countdown_timer->start(1000);
}


bool SigninDialog::checkUserValid(){
    if(ui->user_edit->text()==""){
        AddTipErr(TipErr::TIP_USER_ERR,tr("Username cannot be empty"));
        return false;
    }
    DelTipErr(TipErr::TIP_USER_ERR);
    return true;

}

bool SigninDialog::checkPassValid(){
    std::string error;
    bool result=ValidatePasswordStyle((ui->pass_edit->text().toStdString()),error);
    if(!result){
        AddTipErr(TipErr::TIP_PWD_ERR,QString :: fromStdString(error));
        return false;
    }
    DelTipErr(TipErr::TIP_PWD_ERR);
    if(ui->pass_edit->text()!=ui->repass_edit->text()){
        AddTipErr(TipErr::TIP_CONFIRM_ERR,tr("Password and the reenter password are not the same"));
        return false;
    }
    DelTipErr(TipErr::TIP_CONFIRM_ERR);
    return result;
}


bool SigninDialog::checkVarifyValid(){
    if(ui->code_edit->text()==""){
        AddTipErr(TipErr::TIP_VARIFY_ERR,tr("Varify Code cannot be empty"));
        return false;
    }
    DelTipErr(TipErr::TIP_VARIFY_ERR);
    return true;

}

bool SigninDialog::checkEmailValid(){
    auto email=ui->email_edit->text();
    QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
    bool match=regex.match(email).hasMatch();
    if(!match){
        AddTipErr(TipErr::TIP_EMAIL_ERR,tr("Wrong email format"));
        return false;
    }
    DelTipErr(TipErr::TIP_EMAIL_ERR);
    return true;
}
void SigninDialog::on_returnbtn_clicked()
{
    _countdown_timer->stop();
    emit sigSwitchLogin();
}


void SigninDialog::on_cancel_btn_clicked()
{
    _countdown_timer->stop();
    emit sigSwitchLogin();
}

