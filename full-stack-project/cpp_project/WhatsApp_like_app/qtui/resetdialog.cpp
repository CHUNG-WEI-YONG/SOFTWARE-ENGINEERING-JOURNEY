#include "resetdialog.h"
#include "ui_resetdialog.h"
#include "httpmgr.h"
#include "global.h"
ResetDialog::ResetDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ResetDialog)
{
    ui->setupUi(this);
    connect(ui->user_edit,&QLineEdit::editingFinished,this,[this](){
        checkUserValid();
    });
    connect(ui->email_edit, &QLineEdit::editingFinished, this, [this](){
        checkEmailValid();
    });
    connect(ui->pass_edit, &QLineEdit::editingFinished, this, [this](){
        checkPassValid();
    });
    connect(ui->code_edit, &QLineEdit::editingFinished, this, [this](){
        checkVarifyValid();
    });
    ui->pass_visible->setState("unvisible","unvisible_hover","","visible",
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

    //连接reset相关信号和注册处理回调
    initHandlers();
    connect(Httpmgr::getInstance().get(), &Httpmgr::sig_reset_mod_finish, this,
            &ResetDialog::slot_reset_mod_finish);
}

ResetDialog::~ResetDialog()
{
    delete ui;
}

bool ResetDialog::checkUserValid()
{
    if(ui->user_edit->text() == ""){
        AddTipErr(TipErr::TIP_USER_ERR, tr("用户名不能为空"));
        return false;
    }
    DelTipErr(TipErr::TIP_USER_ERR);
    return true;
}
bool ResetDialog::checkPassValid()
{
    auto pass = ui->pass_edit->text();
    if(pass.length() < 6 || pass.length()>15){
        //提示长度不准确
        AddTipErr(TipErr::TIP_PWD_ERR, tr("密码长度应为6~15"));
        return false;
    }
    // 创建一个正则表达式对象，按照上述密码要求
    // 这个正则表达式解释：
    // ^[a-zA-Z0-9!@#$%^&*]{6,15}$ 密码长度至少6，可以是字母、数字和特定的特殊字符
    QRegularExpression regExp("^[a-zA-Z0-9!@#$%^&*]{6,15}$");
    bool match = regExp.match(pass).hasMatch();
    if(!match){
        //提示字符非法
        AddTipErr(TipErr::TIP_PWD_ERR, tr("不能包含非法字符"));
        return false;;
    }
    DelTipErr(TipErr::TIP_PWD_ERR);
    return true;
}
bool ResetDialog::checkEmailValid()
{
    //验证邮箱的地址正则表达式
    auto email = ui->email_edit->text();
    // 邮箱地址的正则表达式
    QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
    bool match = regex.match(email).hasMatch(); // 执行正则表达式匹配
    if(!match){
        //提示邮箱不正确
        AddTipErr(TipErr::TIP_EMAIL_ERR, tr("邮箱地址不正确"));
        return false;
    }
    DelTipErr(TipErr::TIP_EMAIL_ERR);
    return true;
}
bool ResetDialog::checkVarifyValid()
{
    auto pass = ui->code_edit->text();
    if(pass.isEmpty()){
        AddTipErr(TipErr::TIP_VARIFY_ERR, tr("验证码不能为空"));
        return false;
    }
    DelTipErr(TipErr::TIP_VARIFY_ERR);
    return true;
}
void ResetDialog::AddTipErr(TipErr te, QString tips)
{
    _tip_errs[te] = tips;
    showTip(tips, false);
}
void ResetDialog::DelTipErr(TipErr te)
{
    _tip_errs.remove(te);
    if(_tip_errs.empty()){
        ui->error_label->clear();
        return;
    }
    showTip(_tip_errs.first(), false);
}

void ResetDialog::showTip(QString str, bool b_ok)
{
    if(b_ok){
        ui->error_label->setProperty("state","normal");
    }else{
        ui->error_label->setProperty("state","err");
    }
    ui->error_label->setText(str);
    repolish(ui->error_label);
}

void ResetDialog::initHandlers(){
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

    _handlers.insert(ReqId::ID_RESET_PWD,[this](QJsonObject jsonObj){
        int error=jsonObj["error"].toInt();
        if(static_cast<ErrorCode>(error)!=ErrorCode::SUCCESS){
            showTip(tr("Error in resetting password"),false);
            return;
        }
        auto email=jsonObj["email"].toString();
        showTip(tr("Reset Password Successfully"),true);
        qDebug()<<"Reset password user: "<<email;
        qDebug()<<"User uid is "<<jsonObj["uid"].toString();

    });
}

void ResetDialog::slot_reset_mod_finish(ReqId id,QString res ,ErrorCode error)
{
    qDebug() << "response id =" << int(id);
    qDebug() << "_handlers contains = " << _handlers.contains(id);
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
void ResetDialog::on_cancel_btn_clicked()
{
    emit switchLogin();
}


void ResetDialog::on_confirm_btn_clicked()
{
    bool valid = checkUserValid();
    if(!valid){
        return;
    }
    valid = checkEmailValid();
    if(!valid){
        return;
    }
    valid = checkPassValid();
    if(!valid){
        return;
    }
    valid = checkVarifyValid();
    if(!valid){
        return;
    }
    //发送http重置用户请求
    QJsonObject json_obj;
    json_obj["user"] = ui->user_edit->text();
    json_obj["email"] = ui->email_edit->text();
    json_obj["passwd"] = xorString(ui->pass_edit->text());
    json_obj["varifycode"] = ui->code_edit->text();
    Httpmgr::getInstance()->PostHttp(QUrl(gate_url_prefix+"/reset_pwd"),
                                        json_obj, ReqId::ID_RESET_PWD,Modules::RESETMOD);
    //emit switchLogin();
}


void ResetDialog::on_get_btn_clicked()
{
    qDebug()<<"receive varify btn clicked ";
    auto email = ui->email_edit->text();
    auto bcheck = checkEmailValid();
    if(!bcheck){
        return;
    }
    //发送http请求获取验证码
    QJsonObject json_obj;
    json_obj["email"] = email;
    Httpmgr::getInstance()->PostHttp(QUrl(gate_url_prefix+"/get_verifycode"),
                                     json_obj, ReqId::ID_GET_VERIFY_CODE,Modules::RESETMOD);
}

