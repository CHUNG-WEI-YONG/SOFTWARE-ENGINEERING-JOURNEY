#include "mainwindow.h"
#include <QFile>
#include <QDebug>
#include <QApplication>
#include "global.h"


int main(int argc, char *argv[])
{
    qputenv("QT_QUICK_CONTROLS_STYLE", "Basic");
    QApplication a(argc, argv);
    QFile qss(":/style/stylesheet.qss");
    if(qss.open(QFile::ReadOnly)){
        qDebug("open success");
        QString style=QLatin1String(qss.readAll());
        //qDebug()<<style;
        a.setStyleSheet(style);
        qss.close();
    }
    else{
        qDebug("open failed");

    }
    QString filename="config.ini";
    QString filepath=QCoreApplication::applicationDirPath();
    QString config_path = QDir::toNativeSeparators(filepath +QDir::separator() + filename);
    QSettings settings(config_path, QSettings::IniFormat);
    QString gate_host = settings.value("GateServer/host").toString();
    QString gate_port = settings.value("GateServer/port").toString();
    gate_url_prefix = "http://"+gate_host+":"+gate_port;
    qDebug() << "Connecting to gate URL:" << gate_url_prefix;

    MainWindow w;
    qDebug() << QFile::exists(":/rc/loginlogo.png");
    w.show();
    return QApplication::exec();
}
