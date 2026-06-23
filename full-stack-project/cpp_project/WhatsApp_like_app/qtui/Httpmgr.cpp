#include "httpmgr.h"

Httpmgr::~Httpmgr(){}

Httpmgr::Httpmgr() {
    connect(this,&Httpmgr::sig_http_finish,this,&Httpmgr::slot_http_finish);

}

void Httpmgr::PostHttp(QUrl url,QJsonObject json ,ReqId id,Modules mod ){
    QByteArray data=QJsonDocument(json).toJson();
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    request.setHeader(QNetworkRequest::ContentLengthHeader,QByteArray::number(data.length()));
    //auto self=shared_from_this();
    QNetworkReply * reply=_manager.post(request,data);
    QObject::connect(reply,&QNetworkReply::finished,[this,reply,id,mod](){
        if(reply->error()!=QNetworkReply::NoError){
            qDebug()<<reply->errorString();
            emit this->sig_http_finish(id,"",ErrorCode::Err_NETWORK,mod);
            reply->deleteLater();
            return;
        }
        //no error
        QByteArray res=reply->readAll();
        qDebug()<<"response: "<<res;
        QJsonParseError parseError;
        QJsonDocument resDoc = QJsonDocument::fromJson(res, &parseError);
        QJsonObject resObj = resDoc.object();
        if (parseError.error != QJsonParseError::NoError) {
            qDebug() << "JSON Parse Error:" << parseError.errorString();
            emit sig_http_finish(id, "", ErrorCode::Err_JSON, mod);
            return;
        }
        emit this->sig_http_finish(id,res,ErrorCode::SUCCESS,mod);
        reply->deleteLater();
        return;
    });
}

void Httpmgr::slot_http_finish(ReqId id,QString res,ErrorCode error,Modules mod){
    if(mod==Modules::REGISTERMOD){
        emit sig_reg_mod_finish(id,res,error);
    }








}