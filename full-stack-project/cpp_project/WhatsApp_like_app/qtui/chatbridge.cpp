#include "chatbridge.h"
#include "tcpmgr.h"

void ChatBridge::sendMessageFromQml(const QString& targetUser, const QString& text) {
    qDebug() << "🛰️ [C++ 后台] 收到发给" << targetUser << "的消息:" << text;
    // 在这里调用你的网络总线发送：
    emit sig_send_msg(targetUser,text);
    //TcpMgr::getInstance()->sendMsg(targetUser, text);
}

void ChatBridge::loadMoreHistoryFromQML(const QString &target)
{
    qDebug()<<"Until top, starting to get older message";
    emit sig_req_more_history(target);
}

void ChatBridge::uploadFileFromQml(const QString& friendName, const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        qWarning() << "File does not exist:" << filePath;
        emit sig_file_upload_complete(friendName, false, "");
        return;
    }

    emit sig_req_upload_file(friendName,filePath);

    // Step A: Request upload token from ChatServer (via your existing TCP/gRPC client)
    // Step B: Direct connection to FileServer in a background thread/QNetworkAccessManager
    // Step C: On progress callbacks -> emit sig_file_upload_progress(...)
    // Step D: On finished -> notify ChatServer and emit sig_file_upload_complete(...)
}

void ChatBridge::downloadFileFromQml(const QString& fileUrl, const QString& saveFileName)
{
    qDebug() << "Downloading file from:" << fileUrl << "to save as:" << saveFileName;
    emit sig_req_download_file(fileUrl,saveFileName);
    // Direct HTTP GET / TCP download from File;Server
}