#ifndef HTTPMGR_H
#define HTTPMGR_H
#include "singleton.h"
#include <QString>
#include <QUrl>
#include <QObject>
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QJsonDocument>

class Httpmgr:public QObject,public Singleton<Httpmgr>,public std::enable_shared_from_this<Httpmgr>
{
    Q_OBJECT;
public:
    ~Httpmgr();
private:
    friend class Singleton<Httpmgr>;
    Httpmgr();
    QNetworkAccessManager _manager;
    void PostHttp(QUrl url,QJsonObject json,ReqId req_id,Modules mod);

private slots:
    void slot_http_finish(ReqId id,QString res,ErrorCode error,Modules mod);


signals:
    void sig_http_finish(ReqId id,QString res,ErrorCode error,Modules mod);
    void sig_reg_mod_finish(ReqId id,QString res,ErrorCode error);
};

#endif // HTTPMGR_H
