#include "loadingdialog.h"
#include "ui_loadingdialog.h"
#include <QMovie>

LoadingDialog::LoadingDialog(QWidget *parent,QString tip)
    : QDialog(parent)
    , ui(new Ui::LoadingDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog|Qt::FramelessWindowHint|Qt::WindowSystemMenuHint|Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    if (parent) {
        setFixedSize(parent->size());
    }

    QMovie *movie=new QMovie(":/rc/chat_picture/loading.gif");
    ui->loading_lb->setMovie(movie);
    movie->start();

    movie->setScaledSize(ui->loading_lb->size());
    ui->status_lb->setText(tip);
}

LoadingDialog::~LoadingDialog()
{
    delete ui;
}
