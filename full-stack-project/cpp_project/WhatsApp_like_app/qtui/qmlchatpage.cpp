#include "qmlchatpage.h"
#include "ui_qmlchatpage.h"

QMLChatPage::QMLChatPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::QMLChatPage)
{
    ui->setupUi(this);
}

QMLChatPage::~QMLChatPage()
{
    delete ui;
}
