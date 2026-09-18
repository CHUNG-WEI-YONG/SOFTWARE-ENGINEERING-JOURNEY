#include "filedownloader.h"
#include <QTcpSocket>
#include <QDataStream>
#include <QtEndian>
#include "global.h"

FileDownloader::FileDownloader(const QString &host, quint16 port,
                               const QString &token, const quint64 totalBytes, const QString &saveFilePath, QObject *parent)
    : QObject(parent)
    , _host(host)
    , port(port)
    , _token(token)
    , total_bytes(totalBytes)
    , _save_path(_save_path)
{

}

FileDownloader::~FileDownloader()
{
    if (_file.isOpen()) {
        _file.close();
    }
}

void FileDownloader::StartDownloaded()
{
    _file.setFileName(_save_path);
    if(!_file.open(QIODevice::WriteOnly)){
        qDebug()<<"Error in downloadin file";
        emit sig_finished(false,"Error in Downloading File");
    }

    _socket=new QTcpSocket(this);

    connect(_socket,&QTcpSocket::connected,this,&FileDownloader::onConnected);
    connect(_socket, &QTcpSocket::readyRead, this, &FileDownloader::onReadyRead);
    connect(_socket, &QTcpSocket::errorOccurred, this, &FileDownloader::onErrorOccurred);

    qDebug() << "[Downloader] Connecting to FileServer:" << _host << ":" << port;
    _socket->connectToHost(_host, port);

}

void FileDownloader::onConnected(){
    qDebug() << "[Downloader] Socket connected, sending handshake token...";
    QByteArray tokenByte=_token.toUtf8();
    quint16 tokenLen=static_cast<quint16>(tokenByte.size());

    QByteArray header;
    header.resize(4);
    quint16 msgId = static_cast<quint16>(ReqId::ID_DOWNLOAD_FILE_REQ);
    qToBigEndian<quint16>(msgId, reinterpret_cast<uchar*>(header.data()));
    qToBigEndian<quint16>(tokenLen, reinterpret_cast<uchar*>(header.data() + 2));

    _socket->write(header);
    _socket->write(tokenByte);
}

void FileDownloader::onReadyRead()
{
    if(!_socket||!_file.isOpen()){
        return;
    }
    while(_socket->bytesAvailable()>0){
        QByteArray chunk=_socket->readAll();
        qint16 written=_file.write(chunk);
        if(written<=0){
            emit sig_finished(false,"Error in saving file");
            qDebug()<<"Cannot save";
            _socket->disconnectFromHost();
        }
        recv_bytes+=written;
        emit sig_progress(recv_bytes,total_bytes);

        if(recv_bytes>=total_bytes){
            _file.flush();
            _file.close();
            qDebug()<<"File success download";
            emit sig_finished(true,"Success Download");
            _socket->disconnectFromHost();
            return;
        }
    }

}

void FileDownloader::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    if(recv_bytes<total_bytes){
        emit sig_finished(false,"Connection error");
    }
}







