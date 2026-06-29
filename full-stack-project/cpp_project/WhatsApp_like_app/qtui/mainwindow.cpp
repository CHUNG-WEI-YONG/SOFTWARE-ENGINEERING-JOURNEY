#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "logindialog.h"
#include "signindialog.h"
#include "resetdialog.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    _login_dlg = new LoginDialog(this);
    _login_dlg->setWindowFlags((Qt::CustomizeWindowHint|Qt::FramelessWindowHint));
    setCentralWidget(_login_dlg);
    _login_dlg->show();

    connect(_login_dlg,&LoginDialog::switchRegister,this,&MainWindow::SlotSwitchRegister);
    // _sigin_dlg = new SigninDialog(this);

    // _login_dlg->setWindowFlags((Qt::CustomizeWindowHint|Qt::FramelessWindowHint));
    // _sigin_dlg->setWindowFlags((Qt::CustomizeWindowHint|Qt::FramelessWindowHint));
    connect(_login_dlg,&LoginDialog::switchReset,this,&MainWindow::SlotSwitchReset);
    //return 0;
    //connect(_reset_dlg,&ResetDialog::switchLogin,this,&MainWindow::SlotResetSwitchLogin);
}




MainWindow::~MainWindow()
{
    delete ui;
    // if(_login_dlg){
    //     delete _login_dlg;
    //     _login_dlg=nullptr;
    // }
    // if(_sigin_dlg){
    //     delete _sigin_dlg;
    //     _sigin_dlg=nullptr;
    // }
}

void MainWindow::SlotSwitchRegister(){
    // if(_login_dlg) {
    //     _login_dlg->disconnect(); // 解绑所有信号
    //     _login_dlg->deleteLater(); // 让 Qt 在事件循环中安全释放它
    //     _login_dlg = nullptr;
    // }
    _sigin_dlg = new SigninDialog(this);

    _sigin_dlg->setWindowFlags((Qt::CustomizeWindowHint|Qt::FramelessWindowHint));
    _sigin_dlg->hide();

    connect(_sigin_dlg,&SigninDialog::sigSwitchLogin,this,&MainWindow::SlotSwitchLogin);
    setCentralWidget(_sigin_dlg);
    _login_dlg->hide();
    _sigin_dlg->show();
}

void MainWindow::SlotSwitchLogin(){
    // if(_sigin_dlg) {
    //     _sigin_dlg->disconnect();
    //     _sigin_dlg->deleteLater();
    //     _sigin_dlg = nullptr;
    // }
    _login_dlg=new LoginDialog(this);
    _login_dlg->setWindowFlags((Qt::CustomizeWindowHint|Qt::FramelessWindowHint));
    setCentralWidget(_login_dlg);

    _sigin_dlg->hide();
    _login_dlg->show();

    connect(_login_dlg,&LoginDialog::switchRegister,this , &MainWindow::SlotSwitchRegister);
    connect(_login_dlg,&LoginDialog::switchReset,this,&MainWindow::SlotSwitchReset);
}

void MainWindow::SlotSwitchReset()
{
    _reset_dlg=new ResetDialog(this);
    _reset_dlg->setWindowFlags((Qt::CustomizeWindowHint|Qt::FramelessWindowHint));
    setCentralWidget(_reset_dlg);

    _login_dlg->hide();
    _reset_dlg->show();

    connect(_reset_dlg,&ResetDialog::switchLogin,this,&MainWindow::SlotResetSwitchLogin);

}

void MainWindow::SlotResetSwitchLogin()
{
    _login_dlg=new LoginDialog(this);
    _login_dlg->setWindowFlags((Qt::CustomizeWindowHint|Qt::FramelessWindowHint));
    setCentralWidget(_login_dlg);

    _reset_dlg->hide();
    _login_dlg->show();

    connect(_login_dlg,&LoginDialog::switchReset,this,&MainWindow::SlotSwitchReset);
    connect(_login_dlg,&LoginDialog::switchRegister,this,&MainWindow::SlotSwitchRegister);
}