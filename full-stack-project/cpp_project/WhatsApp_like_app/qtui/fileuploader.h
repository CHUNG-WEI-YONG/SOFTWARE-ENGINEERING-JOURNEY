#ifndef FILEUPLOADER_H
#define FILEUPLOADER_H
#include <QObject>
#include <QTcpSocket>
#include <QFile>
#include <QString>
#include "UserMgr.h"

class FileUploader:public QObject
{
    Q_OBJECT
public:
    FileUploader(const FileToken& tokenInfo,const QString& localPath,QObject *parent=nullptr);
    ~FileUploader();

public slots:
    // 连接到 QThread::started 的主入口
    void startUpload();
signals:
    // 跨线程通知主界面的进度与状态
    void sig_progress(qint64 sentBytes, qint64 totalBytes);
    void sig_finished(bool success, const QString& reason);

private slots:
    void onConnected();
    void onBytesWritten(qint64 bytes);
    void onError(QAbstractSocket::SocketError socketError);

private:
    void sendHandshake();
    void sendNextChunk();
    void cleanup();

    FileToken _tokenInfo;
    QString _filePath;

    // 严禁在构造函数中 new，必须在 startUpload() 中延迟创建
    QTcpSocket* _socket{ nullptr };
    QFile _file;

    qint64 _total_bytes{ 0 };
    qint64 _file_sent_bytes{ 0 };
    qint64 _handshake_len{ 0 };
    bool _handshake_sent{ false };

    static constexpr quint16 ID_FILE_UPLOAD_REQ = 1001; // 与服务器约定的请求 MsgID
    static constexpr qint64 CHUNK_SIZE = 65536;        // 64 KB 分块
};

#endif // FILEUPLOADER_H
