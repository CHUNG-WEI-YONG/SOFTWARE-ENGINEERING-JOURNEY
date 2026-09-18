#ifndef CHATBRIDGE_H
#define CHATBRIDGE_H
#include <QObject>
#include <QVariantMap>
#include <QVariantList>
#include <QFileInfo>
class ChatBridge:public QObject
{
    Q_OBJECT
public:
    ChatBridge(QObject *parent=nullptr):QObject(parent){}
    Q_INVOKABLE void sendMessageFromQml(const QString& targetUser, const QString& text);
    Q_INVOKABLE void loadMoreHistoryFromQML(const QString& target);
    Q_INVOKABLE void uploadFileFromQml(const QString& target,const QString& file_path);
    Q_INVOKABLE void downloadFileFromQml(const QString& fileUrl,const QString& saveFileName);

signals:
    void sig_user_switched(QString name, bool isOnline, QString lastTime,QString logoPath, QVariantList history);

    // 当底层 TCP 收到好友发来的实时新消息时，轰鸣触发这个信号
    void sig_new_message_received(QString sender, QString message);
    void sig_append_history_batch(QVariantList oldHistory);
    void sig_req_more_history(const QString& target);
    void sig_send_msg(QString target,QString text);
    void sig_file_upload_progress(const QString& friendName,int percent);
    void sig_file_upload_complete(const QString& friendName,bool success,const QString& fileUrl);
    void sig_req_upload_file(const QString& friendName,const QString& filePath);
    void sig_req_download_file(const QString& fileToken, const QString& fileName);
    void sig_new_file_arrive(const QString& friendName, const QString& fileName,
                             const QString& fileSizeStr, const QString& filePath,
                             const QString& timeStr, const QString& fileToken);


};


#endif // CHATBRIDGE_H
