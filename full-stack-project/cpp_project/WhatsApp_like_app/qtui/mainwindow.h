#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "logindialog.h"
#include "signindialog.h"
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

private:
    Ui::MainWindow *ui;
    LoginDialog * _login_dlg;
    SigninDialog * _sigin_dlg;
};
#endif // MAINWINDOW_H


