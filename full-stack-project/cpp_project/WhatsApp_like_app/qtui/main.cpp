#include "mainwindow.h"
#include <QFile>
#include <QDebug>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFile qss(":/style/stylesheet.qss");
    if(qss.open(QFile::ReadOnly)){
        qDebug("open success");
        QString style=QLatin1String(qss.readAll());
        qDebug()<<style;
        a.setStyleSheet(style);
        qss.close();
    }
    else{
        qDebug("open failed");

    }

    MainWindow w;
    qDebug() << QFile::exists(":/rc/loginlogo.png");
    w.show();
    return QApplication::exec();
}
