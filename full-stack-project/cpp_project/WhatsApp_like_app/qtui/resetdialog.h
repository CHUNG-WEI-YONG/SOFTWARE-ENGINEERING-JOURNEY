#ifndef RESETDIALOG_H
#define RESETDIALOG_H

#include <QDialog>
#include "global.h"

namespace Ui {
class ResetDialog;
}

class ResetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ResetDialog(QWidget *parent = nullptr);
    ~ResetDialog();

private:
    Ui::ResetDialog *ui;
    QMap<TipErr , QString> _tip_errs;
    void initHttpHandlers();
    QMap<ReqId , std::function<void(const QJsonObject& )>>_handlers;
    bool checkUserValid();
    bool checkEmailValid();
    bool checkPassValid();
    //bool checkConfirmValid();
    bool checkVarifyValid();
    void AddTipErr(TipErr te,QString tips);
    void DelTipErr(TipErr te);
    void changeTipPage();
    void showTip(QString ,bool);
    void initHandlers();

signals:
    void switchLogin();

public slots:
    void slot_reset_mod_finish(ReqId id,QString res ,ErrorCode er);
private slots:
    void on_cancel_btn_clicked();
    void on_confirm_btn_clicked();
    void on_get_btn_clicked();
};

#endif // RESETDIALOG_H
