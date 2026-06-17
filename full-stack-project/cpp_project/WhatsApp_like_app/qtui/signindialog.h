#ifndef SIGNINDIALOG_H
#define SIGNINDIALOG_H

#include <QDialog>
#include "global.h"
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

private:
    void initHttpHandlers();
    Ui::SigninDialog *ui;
    QMap<ReqId , std::function<void(const QJsonObject& )>>_handlers;
};

#endif // SIGNINDIALOG_H
