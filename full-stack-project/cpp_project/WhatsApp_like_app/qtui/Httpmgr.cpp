#include "httpmgr.h"

Httpmgr::~Httpmgr(){}

Httpmgr::Httpmgr() {
    connect(this,&Httpmgr::sig_http_finish,this,&Httpmgr::slot_http_finish);

}

void Httpmgr::PostHttp(QUrl url, QJsonObject json, ReqId id, Modules mod) {
    QByteArray data = QJsonDocument(json).toJson(QJsonDocument::Compact);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(data.length()));

    QNetworkReply* reply = _manager.post(request, data);

    // 监听网络底层异常，方便定位是否为对端切断
    QObject::connect(reply, &QNetworkReply::errorOccurred, this, [](QNetworkReply::NetworkError err) {
        qDebug() << "⚠️ [Qt 网络错误发生]:" << err;
    });

    QObject::connect(reply, &QNetworkReply::finished, [this, reply, id, mod]() {
        // 保证在任何 return 路径下，reply 都会被安全销毁
        auto cleanup = [reply]() {
            reply->deleteLater();
        };

        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "❌ [Http 请求失败]:" << reply->errorString();
            emit this->sig_http_finish(id, "", ErrorCode::Err_NETWORK, mod);
            cleanup();
            return;
        }

        QByteArray res = reply->readAll();
        qDebug() << "✅ [Http 收到回包]:" << res;

        QJsonParseError parseError;
        QJsonDocument resDoc = QJsonDocument::fromJson(res, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            qDebug() << "❌ [JSON 解析错误]:" << parseError.errorString();
            emit sig_http_finish(id, "", ErrorCode::Err_JSON, mod);
            cleanup(); // ✅ 确保释放
            return;
        }

        emit this->sig_http_finish(id, res, ErrorCode::SUCCESS, mod);
        cleanup();
    });
}

void Httpmgr::slot_http_finish(ReqId id,QString res,ErrorCode error,Modules mod){
    if(mod==Modules::REGISTERMOD){
        emit sig_reg_mod_finish(id,res,error);
    }

    if(mod==Modules::RESETMOD){
        emit sig_reset_mod_finish(id,res,error);
    }

    if(mod==Modules::LOGINMOD){
        emit sig_login_mod_finish(id,res,error);
    }








}