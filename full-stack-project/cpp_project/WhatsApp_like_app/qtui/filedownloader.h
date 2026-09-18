#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include <QObject>
#include <QTcpSocket>
#include <QFile>
#include <QString>

class FileDownloader:public QObject
{
    Q_OBJECT;
public:
    explicit FileDownloader(const QString& host,quint16 port,const QString& token,const quint64 totalBytes,const QString& saveFilePath, QObject *parent = nullptr);
    ~FileDownloader();
public slots:
    void StartDownloaded();

signals:
    void sig_progress(qint64 receivedBytes,qint64 totalBytes);
    void sig_finished(bool success ,QString reason);

private slots:
    void onConnected();
    void onReadyRead();
    void onErrorOccurred(QAbstractSocket::SocketError socketError);

private:
    QString _host;
    quint16 port;
    QString _token;
    qint64 total_bytes{0};
    qint64 recv_bytes{0};
    QString _save_path;

    QTcpSocket* _socket{nullptr};
    QFile _file;


};

#endif // FILEDOWNLOADER_H
