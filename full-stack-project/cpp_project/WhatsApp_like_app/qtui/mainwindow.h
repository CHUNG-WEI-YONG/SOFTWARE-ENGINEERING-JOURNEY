#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "logindialog.h"
#include "signindialog.h"
#include "resetdialog.h"
#include "chatdialog.h"
/*************************************************************************
 * @file:     mainwindow.h
 * @brief:
 * @author:   Chung Wei Yong
 * @date:     2026
 * @history:
 *************************************************************************/
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
public slots:
    void SlotSwitchRegister();
    void SlotSwitchLogin();
    void SlotSwitchReset();
    void SlotResetSwitchLogin();
    void SlotSwitchChat();
private:
    Ui::MainWindow *ui;
    LoginDialog * _login_dlg;
    SigninDialog * _sigin_dlg;
    ResetDialog *_reset_dlg;
    ChatDialog *_chat_dlg;
};

#endif // MAINWINDOW_H


