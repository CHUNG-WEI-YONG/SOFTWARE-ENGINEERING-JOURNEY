#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "global.h"
#include <QDebug>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

private:
    Ui::LoginDialog *ui;
    bool CheckUserValid();
    bool CheckPasswdValid();
    void AddTipErr(TipErr te,QString tips);
    void DelTipErr(TipErr te);
    void showTip(QString str,bool b_ok);
    void enableBtn(bool result);
    void initHttpHandlers();
    QMap<ReqId , std::function<void(const QJsonObject&)>>_handlers;
    QMap<TipErr , QString> _tips_errors;
    QString _token;
    int _uid;

public slots:
    void slot_forget_pwd();
signals:
    void switchRegister();
    void switchReset();
    void sig_connect_Tcp(Serverinfo);
private slots:
    void on_login_btn_clicked();
    void slot_login_mod_finish(ReqId id,QString res,ErrorCode err);
    void slot_conn_success(bool success);
    void slot_conn_failed(ErrorCode err);
};

#endif // LOGINDIALOG_H
