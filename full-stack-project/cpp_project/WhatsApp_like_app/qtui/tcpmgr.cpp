#include "tcpmgr.h"
#include <QAbstractSocket>
#include <QJsonDocument>


TcpMgr::TcpMgr() {
    connect(&_socket,&QTcpSocket::connected,this,&TcpMgr::sig_con_success);
    connect(&_socket,&QTcpSocket::readyRead,[&](){
        _buffer.append(_socket.readAll());
        QDataStream stream(&_buffer,QIODevice::ReadOnly);
        stream.setVersion(QDataStream::Qt_5_0);
        forever{
            if(!_b_recv_pending){
                if(_buffer.size()<static_cast<int>(sizeof(quint16)*2)){
                    return;
                }
                stream>>_message_id>>_message_len;
                _buffer=_buffer.mid(static_cast<int>(sizeof(quint16)*2));
                qDebug()<<"Message id is "<<_message_id<<" .Length is "<<_message_len;
            }

            if(_buffer.size()<_message_len){
                _b_recv_pending=true;
                return;
            }
            _b_recv_pending=false;
            QByteArray message=_buffer.mid(0,_message_len);
            qDebug()<<"Receive message body is "<<message;
            _buffer=_buffer.mid(_message_len);
            handle_Message(ReqId(_message_id),_message_len,message);
        }
    });

    QObject::connect(&_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred), [&](QAbstractSocket::SocketError socketError) {
           Q_UNUSED(socketError)
          qDebug() << "Error:" << _socket.errorString();
        });

    connect(&_socket , &QTcpSocket::disconnected,[this](){
        qDebug()<<"Socket disconnected";
    });
    connect(this,&TcpMgr::sig_send_data,this,&TcpMgr::slot_send_data);
    initHandlers();

}

void TcpMgr::initHandlers()
{
    _handlers.insert(ReqId::ID_CHAT_LOGIN,[this](ReqId id,quint16 len,QByteArray message){
        Q_UNUSED(len);
        qDebug()<<"Handle id is "<<static_cast<int>(id)<<" and data is "<<message;
        QJsonDocument jsonDoc=QJsonDocument::fromJson(message);
        QJsonObject jsonObj=jsonDoc.object();
        if(jsonDoc.isNull()){
            qDebug()<<"Error in Reading Messsage";
            //emit sig_login_failed(static_cast<ErrorCode>(error));
            return;
        }


        if(!jsonObj.contains("Error")){
            ErrorCode error=ErrorCode::Err_JSON;
            qDebug()<<"Login failed. Error is "<<static_cast<int>(error);
            emit sig_login_failed(error);
            return ;
        }
        int error=jsonObj["Error"].toInt();
        if(static_cast<ErrorCode>(error)!=ErrorCode::SUCCESS){
            qDebug()<<"Error message Get";
            emit sig_login_failed(static_cast<ErrorCode>(error));
            return;
        }
        qDebug()<<"Log in successful";
        emit sig_switch_chat_dlg();
    });
}

void TcpMgr::HandleMessage(ReqId id, int len, QByteArray data)
{
    auto find_iter=_handlers.find(id);
    if(find_iter==_handlers.end()){
        qDebug()<<"Error in finding handlers id "<<static_cast<int>(id);
        return;
    }
    find_iter.value()(id,len,data);

}

void TcpMgr::slot_tcp_connect(Serverinfo si)
{
    _host=si.host;
    _port=static_cast<uint16_t>(si.port.toUInt());
    _socket.connectToHost(si.host,_port);
}

void TcpMgr::slot_send_data(ReqId Reqid, QString data)
{
    quint16 id = static_cast<quint16>(Reqid);
    QByteArray  databytes=data.toUtf8();

    quint16 len=static_cast<quint16>(databytes.size());

    QByteArray block;
    QDataStream out(&block,QIODevice::WriteOnly);

    out.setByteOrder((QDataStream::BigEndian));

    out<<id<<len;
    block.append(databytes);
    _socket.write(block);
}
