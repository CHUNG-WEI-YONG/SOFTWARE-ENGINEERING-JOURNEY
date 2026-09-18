#include "fileuploader.h"
#include "userdata.h"
#include <QTcpSocket>
#include <QThread>
#include <QObject>


FileUploader::FileUploader(const FileToken &tokenInfo, const QString &localPath, QObject *parent):QObject(parent)
    , _tokenInfo(tokenInfo)
    , _filePath(localPath)
{

}

FileUploader::~FileUploader()
{
    cleanup();
}

void FileUploader::startUpload()
{
    qDebug()<<"Current run on thread of "<<QThread::currentThread();
    _file.setFileName((_filePath));
    if(!_file.open(QIODevice::ReadOnly)){
        return sig_finished(false,QString("Error in opening : %1").arg(_filePath));
    }

    _total_bytes=_file.size();

    _socket=new QTcpSocket(this);
    connect(_socket, &QTcpSocket::connected, this, &FileUploader::onConnected);
    connect(_socket, &QTcpSocket::bytesWritten, this, &FileUploader::onBytesWritten);
    connect(_socket, &QTcpSocket::errorOccurred, this, &FileUploader::onError);
    _socket->connectToHost(_tokenInfo.host,_tokenInfo.port);



}

void FileUploader::onConnected()
{
    qDebug()<<"Connected to the host";
    sendHandshake();
}

void FileUploader::onBytesWritten(qint64 bytes)
{
    if(_handshake_sent>0){
        if(bytes<=_handshake_len){
            _handshake_len-=bytes;
            bytes=0;
        }
        else{
            bytes-=_handshake_len;
            _handshake_len=0;
        }
    }
    //emit sig_progress(_file_sent_bytes, _total_bytes);

    if (bytes > 0) {
        _file_sent_bytes += bytes;
        emit sig_progress(_file_sent_bytes, _total_bytes);
    }

    if(_file_sent_bytes>=_total_bytes){
        qDebug()<<"File send success";
        _file.close();
        emit sig_finished(true,"Success");
        _socket->disconnectFromHost();
    }
    else{
        sendNextChunk();
    }


}

void FileUploader::onError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    QString errStr = _socket ? _socket->errorString() : "Unknown socket error";
    qWarning() << "[FileUploader] Socket error:" << errStr;

    cleanup();
    emit sig_finished(false, errStr);
}

void FileUploader::sendHandshake()
{
    QByteArray data=_tokenInfo.toJsonString().toUtf8();
    quint16 id=ID_FILE_UPLOAD_REQ;
    quint16 len=static_cast<quint16>(data.size());

    QByteArray block;
    QDataStream out(&block,QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::BigEndian);

    out<<id<<len;
    block.append(data);

    _handshake_len=block.size();
    _socket->write(block);
    _handshake_sent=true;
    sendNextChunk();

}

void FileUploader::sendNextChunk()
{
    if(!_file.isOpen()||!_file.atEnd()||!_file_sent_bytes>=_total_bytes){
        return;
    }
    QByteArray chunk=_file.read(CHUNK_SIZE);
    if(!chunk.isEmpty()){
        _socket->write(chunk);
        _file_sent_bytes+=CHUNK_SIZE;
    }
}

void FileUploader::cleanup()
{
    if(_file.isOpen()){
        _file.close();
    }
    if(_socket){
        _socket->abort();
        _socket->deleteLater();
        _socket=nullptr;
    }


}



