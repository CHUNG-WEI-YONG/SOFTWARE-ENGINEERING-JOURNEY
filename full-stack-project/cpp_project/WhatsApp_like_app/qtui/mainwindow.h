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

enum UIStatus{
    LOGIN_UI,
    REGISTER_UI,
    RESET_UI,
    CHAT_UI
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
    void offlinelogin();
public slots:
    void SlotSwitchRegister();
    void SlotSwitchLogin();
    void SlotSwitchReset();
    void SlotResetSwitchLogin();
    void SlotSwitchChat();
    void slot_offline();
    void slot_excepCon_Offline();
private:
    Ui::MainWindow *ui;
    LoginDialog * _login_dlg;
    SigninDialog * _sigin_dlg;
    ResetDialog *_reset_dlg;
    ChatDialog *_chat_dlg;
    UIStatus _ui_status;
};

#endif // MAINWINDOW_H


