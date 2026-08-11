#include "tcpmgr.h"
#include "usermgr.h"
#include <QAbstractSocket>
#include <QJsonDocument>


TcpMgr::TcpMgr() {
    connect(&_socket,&QTcpSocket::connected,this,[this](){
        emit this->sig_con_success(true);
    });
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
            HandleMessage(ReqId(_message_id),_message_len,message);
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
    _handlers.insert(ReqId::ID_CHAT_LOGIN_RSP,[this](ReqId id,quint16 len,QByteArray message){
        Q_UNUSED(len);
        qDebug()<<"Handle id is "<<static_cast<int>(id)<<" and data is "<<message;
        QJsonDocument jsonDoc=QJsonDocument::fromJson(message);
        QJsonObject jsonObj=jsonDoc.object();
        if(jsonDoc.isNull()){
            qDebug()<<"Error in Reading Messsage";
            emit sig_login_failed(ErrorCode::Err_JSON);
            return;
        }


        if(!jsonObj.contains("error")){
            ErrorCode error=ErrorCode::Err_JSON;
            qDebug()<<"Login failed. Error is "<<static_cast<int>(error);
            emit sig_login_failed(error);
            return ;
        }
        int error=jsonObj["error"].toInt();
        if(static_cast<ErrorCode>(error)!=ErrorCode::SUCCESS){
            qDebug()<<"Error message Get";
            emit sig_login_failed(static_cast<ErrorCode>(error));
            return;
        }
        qDebug()<<"Log in successful";
        UserMgr::getInstance()->SetUid(jsonObj["uid"].toInt());
        UserMgr::getInstance()->SetName(jsonObj["name"].toString());
        UserMgr::getInstance()->SetToken(jsonObj["token"].toString());

        emit sig_switch_chat_dlg();
    });

    _handlers.insert(ReqId::ID_SEARCH_USER_RSP, [this](ReqId id, quint16 len, QByteArray message) {
        Q_UNUSED(len);
        QJsonDocument jsonDoc = QJsonDocument::fromJson(message);
        if (jsonDoc.isNull() || !jsonDoc.isObject()) {
            emit sig_user_search(nullptr);
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();
        if (jsonObj["error"].toInt() != static_cast<int>(ErrorCode::SUCCESS)) {
            emit sig_user_search(nullptr);
            return;
        }

        // 明确提取为强类型变量，防止 ASCII 转换
        int uid = jsonObj["uid"].toVariant().toInt();
        QString name = jsonObj["name"].toString();
        QString nick = jsonObj["nick"].toString();
        QString desc = jsonObj["desc"].toString();
        int sex = jsonObj["sex"].toVariant().toInt();
        QString icon = jsonObj["icon"].toString();

        // 按统一顺序传入：(uid, name, nick, desc, sex, icon)
        auto search_info = std::make_shared<SearchInfo>(uid, name, nick, desc, sex, icon);
        qDebug() << "After create SearchInfo UID:"
                 << search_info->_uid;
        emit sig_user_search(search_info);
    });

    _handlers.insert(ReqId::ID_ADD_FRIEND_RSP,[this](ReqId id,quint16 len,QByteArray message){
        Q_UNUSED(len);
        qDebug()<<"Handle id is "<<static_cast<int>(id)<<" and data is "<<message;
        QJsonDocument jsonDoc=QJsonDocument::fromJson(message);
        QJsonObject jsonObj=jsonDoc.object();
        if(jsonDoc.isNull()){
            qDebug()<<"Json object is null , return error";
            return;
        }
        if(!jsonObj.contains("error")){
            ErrorCode error=ErrorCode::Err_JSON;
            qDebug()<<"Add Friend failed. Error is "<<static_cast<int>(error);
            //emit sig_login_failed(error);
            return ;
        }
        int error=jsonObj["error"].toInt();
        if(static_cast<ErrorCode>(error)!=ErrorCode::SUCCESS){
            qDebug()<<"Error message Get";
            //emit sig_login_failed(static_cast<ErrorCode>(error));
            return;
        }

        qDebug()<<"Add friend req success";

    });

    _handlers.insert(ReqId::ID_NOTIFY_ADD_FRIEND_REQ,[this](ReqId id,quint16 len,QByteArray message){
        Q_UNUSED(len);
        qDebug()<<"Handle id is "<<static_cast<int>(id)<<" and data is "<<message;
        QJsonDocument jsonDoc=QJsonDocument::fromJson(message);
        QJsonObject jsonObj=jsonDoc.object();
        if(jsonDoc.isNull()){
            qDebug()<<"Json object is null , return error";
            return;
        }
        if(!jsonObj.contains("error")){
            ErrorCode error=ErrorCode::Err_JSON;
            qDebug()<<"Notify add Friend failed. Error is "<<static_cast<int>(error);
            //emit sig_login_failed(error);
            return ;
        }
        int error=jsonObj["error"].toInt();
        if(static_cast<ErrorCode>(error)!=ErrorCode::SUCCESS){
            qDebug()<<"Error message Get";
            //emit sig_login_failed(static_cast<ErrorCode>(error));
            return;
        }

        QString name=jsonObj["name"].toString();
        int uid=jsonObj["applyuid"].toInt();
        QString icon=jsonObj["icon"].toString();
        QString desc=jsonObj["desc"].toString();
        int sex=jsonObj["sex"].toInt();
        QString nick=jsonObj["nick"].toString();
        std::shared_ptr<AddFriendApply> req_user=std::make_shared<AddFriendApply>(uid,name,desc,icon,nick,sex);
        emit sig_friend_apply(req_user);
        qDebug()<<"Notify Add friend req success";

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

std::vector<std::shared_ptr<ApplyInfo> > TcpMgr::GetApplyList()
{
    return _apply_list;
}

void TcpMgr::slot_tcp_connect(Serverinfo si)
{
    _host=si.host;
    _port=static_cast<uint16_t>(si.port.toUInt());
    _socket.connectToHost(si.host,_port);
}


void TcpMgr::slot_send_data(ReqId Reqid, QByteArray databytes)
{
    quint16 id = static_cast<quint16>(Reqid);

    quint16 len=static_cast<quint16>(databytes.size());

    QByteArray block;
    QDataStream out(&block,QIODevice::WriteOnly);

    out.setByteOrder((QDataStream::BigEndian));

    out<<id<<len;
    block.append(databytes);
    qDebug()<<"slot send data done";
    _socket.write(block);
}

TcpMgr::~TcpMgr(){
    qDebug()<<"TCPMGR destruct";
}


