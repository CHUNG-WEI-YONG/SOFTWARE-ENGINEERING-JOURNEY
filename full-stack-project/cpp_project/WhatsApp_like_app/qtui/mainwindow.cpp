#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "logindialog.h"
#include "signindialog.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    _login_dlg = new LoginDialog(this);
    setCentralWidget(_login_dlg);
    _login_dlg->show();

    connect(_login_dlg,&LoginDialog::switchRegister,this,&MainWindow::SlotSwitchRegister);
    _sigin_dlg = new SigninDialog(this);

    _login_dlg->setWindowFlags((Qt::CustomizeWindowHint|Qt::FramelessWindowHint));
    _sigin_dlg->setWindowFlags((Qt::CustomizeWindowHint|Qt::FramelessWindowHint));

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
    setCentralWidget(_sigin_dlg);
    _login_dlg->hide();
    _sigin_dlg->show();
}