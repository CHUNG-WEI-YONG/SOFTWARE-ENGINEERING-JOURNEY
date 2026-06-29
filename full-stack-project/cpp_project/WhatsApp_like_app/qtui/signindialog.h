#ifndef SIGNINDIALOG_H
#define SIGNINDIALOG_H

#include <QDialog>
#include "global.h"
#include <QDebug>
namespace Ui {
    class SigninDialog;
}

class SigninDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SigninDialog(QWidget *parent = nullptr);
    ~SigninDialog();

private slots:
    void on_get_Code_clicked();
    void showTip(QString str,bool b_ok);
    void sig_reg_mod_finish(ReqId id,QString res,ErrorCode error);

    void on_sure_btn_clicked();

    void on_returnbtn_clicked();

    void on_cancel_btn_clicked();

private:
    void initHttpHandlers();
    Ui::SigninDialog *ui;
    QMap<ReqId , std::function<void(const QJsonObject& )>>_handlers;
    bool checkUserValid();
    bool checkEmailValid();
    bool checkPassValid();
    //bool checkConfirmValid();
    bool checkVarifyValid();
    void AddTipErr(TipErr te,QString tips);
    void DelTipErr(TipErr te);
    void changeTipPage();
    QMap<TipErr , QString> _tips_errors;
    QTimer *_countdown_timer;
    int _countdown;


signals:
    void sigSwitchLogin();


};

#endif // SIGNINDIALOG_H
