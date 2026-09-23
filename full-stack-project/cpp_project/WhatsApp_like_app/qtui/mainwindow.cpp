#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "logindialog.h"
#include "signindialog.h"
#include "resetdialog.h"
#include "chatdialog.h"
#include "tcpmgr.h"
#include <QMessageBox>
#include "usermgr.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    _login_dlg = new LoginDialog(this);
    _login_dlg->setWindowFlags((Qt::CustomizeWindowHint|Qt::FramelessWindowHint));
    setCentralWidget(_login_dlg);
    _login_dlg->show();
    _ui_status=UIStatus::LOGIN_UI;

    connect(_login_dlg,&LoginDialog::switchRegister,this,&MainWindow::SlotSwitchRegister);

    connect(_login_dlg,&LoginDialog::switchReset,this,&MainWindow::SlotSwitchReset);
    //return 0;
    //connect(_reset_dlg,&ResetDialog::switchLogin,this,&MainWindow::SlotResetSwitchLogin);
    connect(TcpMgr::getInstance().get(),&TcpMgr::sig_switch_chat_dlg,this,&MainWindow::SlotSwitchChat);
    connect(TcpMgr::getInstance().get(),&TcpMgr::sig_offline,this,&MainWindow::slot_offline);

    //emit TcpMgr::getInstance()->sig_switch_chat_dlg();
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

void MainWindow::offlinelogin()
{

    if(_ui_status==UIStatus::LOGIN_UI)return;
    QMessageBox::warning(this,tr("Log Out "),"This account has been log in at another device");
    UserMgr::getInstance()->Reset();

    _login_dlg=new LoginDialog();
    _login_dlg->setAttribute(Qt::WA_DeleteOnClose);
    _ui_status=UIStatus::LOGIN_UI;
    _login_dlg->show();
    _chat_dlg->hide();
    this->deleteLater();

}

void MainWindow::SlotSwitchRegister(){

    _sigin_dlg = new SigninDialog(this);


    _sigin_dlg->setWindowFlags((Qt::CustomizeWindowHint|Qt::FramelessWindowHint));
    _sigin_dlg->hide();

    connect(_sigin_dlg,&SigninDialog::sigSwitchLogin,this,&MainWindow::SlotSwitchLogin);
    setCentralWidget(_sigin_dlg);
    _login_dlg->hide();
    _sigin_dlg->show();
    _ui_status=UIStatus::REGISTER_UI;
}

void MainWindow::SlotSwitchLogin(){

    _login_dlg=new LoginDialog(this);
    _login_dlg->setWindowFlags((Qt::CustomizeWindowHint|Qt::FramelessWindowHint));
    setCentralWidget(_login_dlg);

    _sigin_dlg->hide();
    _login_dlg->show();
    _ui_status=UIStatus::LOGIN_UI;
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
    _ui_status=UIStatus::RESET_UI;

    connect(_reset_dlg,&ResetDialog::switchLogin,this,&MainWindow::SlotResetSwitchLogin);

}

void MainWindow::SlotResetSwitchLogin()
{
    if(_ui_status==UIStatus::LOGIN_UI)return;
    _ui_status=UIStatus::LOGIN_UI;
    _login_dlg=new LoginDialog(this);
    _login_dlg->setWindowFlags((Qt::CustomizeWindowHint|Qt::FramelessWindowHint));
    setCentralWidget(_login_dlg);

    _reset_dlg->hide();
    _login_dlg->show();

    connect(_login_dlg,&LoginDialog::switchReset,this,&MainWindow::SlotSwitchReset);
    connect(_login_dlg,&LoginDialog::switchRegister,this,&MainWindow::SlotSwitchRegister);
}

void MainWindow::SlotSwitchChat()
{
    _ui_status=UIStatus::CHAT_UI;
    _chat_dlg=new ChatDialog(this);
    _chat_dlg->setWindowFlags((Qt::CustomizeWindowHint|Qt::FramelessWindowHint));
    setCentralWidget(_chat_dlg);
    _login_dlg->hide();
    _chat_dlg->show();
    this->setMaximumSize(16777215, 16777215); // 允许无限放大

    // 方案 A：让软件启动时直接【撑满全屏】（工业级 IM 软件首选）
    //this->showMaximized();
    this->resize(1000, 750);
}

void MainWindow::slot_offline()
{
    QMessageBox::information(this,"Offline Notice","Same Account log in in another place");
    TcpMgr::getInstance()->CloseConnection();
    offlinelogin();
}

void MainWindow::slot_excepCon_Offline()
{
    QMessageBox::information(this,"Offline Notice","Too long in Connecting Server");
    TcpMgr::getInstance()->CloseConnection();
    offlinelogin();
}