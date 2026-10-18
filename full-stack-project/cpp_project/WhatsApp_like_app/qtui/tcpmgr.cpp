#include "tcpmgr.h"
#include "usermgr.h"
#include <QAbstractSocket>
#include <QJsonDocument>
#include "userdata.h"
#include <QJsonArray>


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
          qDebug() << "Socket Error:" << _socket.errorString();
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
        auto uid = jsonObj["uid"].toInt();
        auto name = jsonObj["name"].toString();
        auto nick = jsonObj["nick"].toString();
        auto icon = jsonObj["icon"].toString();
        auto sex = jsonObj["sex"].toInt();
        auto desc = jsonObj["desc"].toString();
        auto user_info = std::make_shared<UserInfo>(uid, name, nick, desc, sex,"","");
        UserMgr::getInstance()->SetUserInfo(user_info);
        UserMgr::getInstance()->SetToken(jsonObj["token"].toString());

        if(jsonObj.contains("applylist")){
            UserMgr::getInstance()->AppendApplyList(jsonObj["applylist"].toArray());
        }

        if(jsonObj.contains("friend_list")){
            UserMgr::getInstance()->AppendFriendList(jsonObj["friend_list"].toArray());
        }

        emit sig_switch_chat_dlg();
    });

    _handlers.insert(ReqId::ID_NOTIFY_OFF_LINE_REQ,[this](ReqId id,quint16 len,QByteArray message){
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
            qDebug()<<"Nofity message failed. Error is "<<static_cast<int>(error);
            emit sig_login_failed(error);
            return ;
        }
        int error=jsonObj["error"].toInt();
        if(static_cast<ErrorCode>(error)!=ErrorCode::SUCCESS){
            qDebug()<<"Error message Get";
            emit sig_login_failed(static_cast<ErrorCode>(error));
            return;
        }


        auto uid=jsonObj["uid"];
        auto content=jsonObj["content"];
        QString reason = jsonObj.value("reason").toString(
            tr("Your account was logged in from another device.")
            );
        emit sig_offline(reason);
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
        qDebug() << "🔔 Friend Apply Received from UID:" << req_user->_from_uid << ", Name:" << name;
        //UserMgr::getInstance()->AddFriend(req_user);
        // 3. 发射信号通知 UI 层
        emit sig_friend_apply(req_user);

        qDebug() << "✅ Notify Add friend req success";

    });

    _handlers.insert(ReqId::ID_NOTIFY_AUTH_FRIEND_REQ,[this](ReqId id,quint16 len,QByteArray message){
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
            qDebug()<<"Notify auth friend failed. Error is "<<static_cast<int>(error);
            emit sig_login_failed(error);
            return ;
        }
        int error=jsonObj["error"].toInt();
        if(static_cast<ErrorCode>(error)!=ErrorCode::SUCCESS){
            qDebug()<<"Error message Get";
            emit sig_login_failed(static_cast<ErrorCode>(error));
            return;
        }
        qDebug()<<"Authen friend successful";
        auto from_uid = jsonObj["fromuid"].toInt();
        auto name = jsonObj["name"].toString();
        auto nick = jsonObj["nick"].toString();
        auto icon = jsonObj["icon"].toString();
        auto sex = jsonObj["sex"].toInt();
        auto thread_id=jsonObj["thread_id"].toInt();
        auto authInfo=std::make_shared<AuthInfo>(from_uid,name,nick,icon,sex,thread_id);

        if (jsonObj.contains("chat_datas") && jsonObj["chat_datas"].isArray()) {
            QJsonArray chatArray = jsonObj["chat_datas"].toArray();
            for (const auto& itemVal : chatArray) {
                QJsonObject chatObj = itemVal.toObject();
                int msg_id      = chatObj["msg_id"].toInt();
                int sender_id   = chatObj["sender"].toInt();
                QString content = chatObj["msg_content"].toString();
                int status      = chatObj.contains("status") ? chatObj["status"].toInt() : 1;
                QString timeStr = QTime::currentTime().toString("hh:mm AP");

                auto msg = std::make_shared<TextChatData>(
                    msg_id,
                    thread_id,
                    ChatFormType::PRIVATE,
                    content,
                    sender_id,
                    status,
                    timeStr
                    );
                authInfo->_chat_datas.push_back(msg);
            }
        }

        UserMgr::getInstance()->AddFriend(authInfo);
        emit sig_add_auth_friend(authInfo);
        qDebug()<<"Auth info for user friend id:"<<authInfo->_uid<<"received a reply\n";
    });

    _handlers.insert(ReqId::ID_AUTH_FRIEND_RSP,[this](ReqId id,quint16 len,QByteArray message){
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
            qDebug()<<"Auth friend failed. Error is "<<static_cast<int>(error);
            emit sig_login_failed(error);
            return ;
        }
        int error=jsonObj["error"].toInt();
        if(static_cast<ErrorCode>(error)!=ErrorCode::SUCCESS){
            qDebug()<<"Error message Get";
            emit sig_login_failed(static_cast<ErrorCode>(error));
            return;
        }
        qDebug()<<"Authen friend successful";
        auto uid = jsonObj["uid"].toInt();
        auto name = jsonObj["name"].toString();
        auto nick = jsonObj["nickname"].toString();
        auto icon = jsonObj["icon"].toString();
        auto sex = jsonObj["sex"].toInt();
        auto thread_id=jsonObj["thread_id"].toInt();
        auto authInfo=std::make_shared<AuthRsp>(uid,name,nick,icon,sex,thread_id);

        if (jsonObj.contains("chat_datas") && jsonObj["chat_datas"].isArray()) {
            QJsonArray chatArray = jsonObj["chat_datas"].toArray();
            for (const auto& itemVal : chatArray) {
                QJsonObject chatObj = itemVal.toObject();
                int msg_id      = chatObj["msg_id"].toInt();
                int sender_id   = chatObj["sender"].toInt();
                QString content = chatObj["msg_content"].toString();
                int status      = chatObj.contains("status") ? chatObj["status"].toInt() : 1;

                QString timeStr = QTime::currentTime().toString("hh:mm AP");

                // 构造多态消息对象存入 authInfo->_chat_datas
                auto msg = std::make_shared<TextChatData>(
                    msg_id,
                    thread_id,
                    ChatFormType::PRIVATE,
                    content,
                    sender_id,
                    status,
                    timeStr
                    );
                authInfo->_chat_datas.push_back(msg);
            }
        }

        UserMgr::getInstance()->AddFriend(authInfo);
        emit sig_auth_rsp(authInfo);

        qDebug()<<"Auth friend success";
    });

    _handlers.insert(ReqId::ID_LOAD_CHAT_MSG_RSP,[this](ReqId id,quint16 len,QByteArray message){
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
            qDebug()<<"Load chat msg failed. Error is "<<static_cast<int>(error);
            emit sig_login_failed(error);
            return ;
        }
        int error=jsonObj["error"].toInt();
        if(static_cast<ErrorCode>(error)!=ErrorCode::SUCCESS){
            qDebug()<<"Error message Get";
            emit sig_login_failed(static_cast<ErrorCode>(error));
            return;
        }

        int to_uid = jsonObj["to_uid"].toInt(); // 对应聊天好友的 UID
        int thread_id = jsonObj.contains("thread_id")
                            ? jsonObj["thread_id"].toInt()
                            : UserMgr::getInstance()->GetFriendThreadId(to_uid);
        int next_last_msg_id = jsonObj["last_msg_id"].toInt(); // 服务器返回下一次拉取的起始 msg_id
        QJsonArray msgArray = jsonObj["chat_datas"].toArray();
        std::vector<std::shared_ptr<ChatDataBase>> historyList;
        historyList.reserve(msgArray.size());
        for(const auto& item:msgArray){
            QJsonObject mObj=item.toObject();
            int msg_id      = mObj["msg_id"].toInt();
            int sender_id   = mObj.contains("sender") ? mObj["sender"].toInt() : mObj["from_uid"].toInt();
            int type    = mObj["type"].toInt();
            QString content = mObj["msg_content"].toString();
            QString created_at = mObj["created_at"].toString();
            QString updated_at=mObj["updated_at"].toString();
            int status=mObj["status"].toInt();
            if (created_at.isEmpty()) {
                created_at = QTime::currentTime().toString("hh:mm AP");
            }

            if(type==static_cast<int>(ChatMsgType::FILE)){
                QString filename = mObj["filename"].toString();
                qint64 filesz    = mObj["filesz"].toVariant().toLongLong();
                QString token    = mObj["token"].toString();
                QString md5      = mObj["md5"].toString();

                if (filename.isEmpty() && content.startsWith('{')) {
                    QJsonDocument subDoc = QJsonDocument::fromJson(content.toUtf8());
                    if (!subDoc.isNull() && subDoc.isObject()) {
                        QJsonObject subObj = subDoc.object();
                        filename = subObj["filename"].toString();
                        filesz   = subObj["filesz"].toVariant().toLongLong();
                        token    = subObj["token"].toString();
                        md5      = subObj["md5"].toString();
                    }
                }

                if(filename.isEmpty())filename=content;
                auto filedata=std::make_shared<FileChatData>(msg_id,thread_id,ChatFormType::PRIVATE
                                                               ,filename, filesz, token, md5, sender_id, 1, created_at);
                historyList.push_back(filedata);
            }
            else{
                if(created_at!=updated_at && !updated_at.isEmpty()){
                    created_at=updated_at;
                }
                auto text_data=std::make_shared<TextChatData>(msg_id, thread_id, ChatFormType::PRIVATE, content, sender_id, 1, created_at);
                historyList.push_back(text_data);
            }
        }

        emit sig_load_history_finish(thread_id, historyList, next_last_msg_id);


    });

    _handlers.insert(ReqId::ID_TEXT_CHAT_MSG_RSP,[this](ReqId id,quint16 len,QByteArray message){
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
            qDebug()<<"Chat msg failed. Error is "<<static_cast<int>(error);
            emit sig_login_failed(error);
            return ;
        }
        int error=jsonObj["error"].toInt();
        if(static_cast<ErrorCode>(error)!=ErrorCode::SUCCESS){
            qDebug()<<"Error message Get";
            emit sig_login_failed(static_cast<ErrorCode>(error));
            return;
        }

        qDebug()<<"Chat message sent successfully";


    });

    _handlers.insert(ReqId::ID_NOTIFY_TEXT_CHAT_MSG_REQ,[this](ReqId id,quint16 len,QByteArray message){
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
            qDebug()<<"Nofity message failed. Error is "<<static_cast<int>(error);
            emit sig_login_failed(error);
            return ;
        }
        int error=jsonObj["error"].toInt();
        if(static_cast<ErrorCode>(error)!=ErrorCode::SUCCESS){
            qDebug()<<"Error message Get";
            emit sig_login_failed(static_cast<ErrorCode>(error));
            return;
        }

        int from_uid = jsonObj["from_uid"].toInt();
        int to_uid   = jsonObj["to_uid"].toInt();

        // 仅接收目标是当前登录用户的消息（群聊时可扩展忽略此检查）
        if (UserMgr::getInstance()->GetUid() != to_uid) {
            return;
        }

        int thread_id   = jsonObj.contains("thread_id")
                            ? jsonObj["thread_id"].toInt()
                            : UserMgr::getInstance()->GetFriendThreadId(from_uid);
        QString defaultTime=jsonObj["time"].toString();
        if (defaultTime.isEmpty()) {
            defaultTime = QTime::currentTime().toString("hh:mm AP");
        }

        QString type    = jsonObj["type"].toString("text");
        QString content = jsonObj["content"].toString();
        QString timeStr = jsonObj["time"].toString();
        if (timeStr.isEmpty()) {
            timeStr = QTime::currentTime().toString("hh:mm AP");
        }


        QJsonArray msgArray;
        if (jsonObj.contains("text_array") && jsonObj["text_array"].isArray()) {
            msgArray = jsonObj["text_array"].toArray();
        } else if (jsonObj.contains("text") && jsonObj["text"].isArray()) {
            msgArray = jsonObj["text"].toArray();
        } else {
            // 兼容单条扁平包
            msgArray.append(jsonObj);
        }

        for (const auto& itemVal : msgArray) {
            QJsonObject itemObj = itemVal.toObject();
            int msg_id = itemObj["msg_id"].toInt();
            QString type = itemObj["type"].toString("text");
            QString content = itemObj["content"].toString();
            QString timeStr = itemObj.contains("time") ? itemObj["time"].toString() : defaultTime;

            std::shared_ptr<ChatDataBase> chat_msg = nullptr;

            if (type == "file") {
                QString filename = itemObj["filename"].toString();
                qint64 filesz    = itemObj["filesz"].toVariant().toLongLong();
                QString token    = itemObj["token"].toString();
                QString md5      = itemObj["md5"].toString();

                // 若外层没有独立字段，解析 content 内包含的 JSON 字符串
                if (filename.isEmpty() && content.startsWith('{')) {
                    QJsonDocument subDoc = QJsonDocument::fromJson(content.toUtf8());
                    if (!subDoc.isNull() && subDoc.isObject()) {
                        QJsonObject subObj = subDoc.object();
                        filename = subObj["filename"].toString();
                        filesz   = subObj["filesz"].toVariant().toLongLong();
                        token    = subObj["token"].toString();
                        md5      = subObj["md5"].toString();
                    }
                }
                if (filename.isEmpty()) filename = content;

                chat_msg = std::make_shared<FileChatData>(
                    msg_id, thread_id, ChatFormType::PRIVATE, filename, filesz, token, md5, from_uid, 1, timeStr
                    );
            } else {
                chat_msg = std::make_shared<TextChatData>(
                    msg_id, thread_id, ChatFormType::PRIVATE, content, from_uid, 1, timeStr
                    );
            }

            // 以基类多态指针发射信号
            if (chat_msg) {
                emit sig_text_chat_msg(chat_msg);
            }
        }
    });

    _handlers.insert(ReqId::ID_SEND_FILE_RSP,[this](ReqId id,quint16 len,QByteArray message){
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
            qDebug()<<"Nofity message failed. Error is "<<static_cast<int>(error);
            emit sig_login_failed(error);
            return ;
        }
        int error=jsonObj["error"].toInt();
        if(static_cast<ErrorCode>(error)!=ErrorCode::SUCCESS){
            qDebug()<<"Error message Get";
            emit sig_login_failed(static_cast<ErrorCode>(error));
            return;
        }

        int from_uid=jsonObj["from_uid"].toInt();
        int to_uid=jsonObj["to_uid"].toInt();
        auto file=FileToken::fromJson(jsonObj);
        auto filemsg=std::make_shared<FileToken>(file);

        emit sig_upload_file(filemsg);
    });

    _handlers.insert(ReqId::ID_DOWNLOAD_FILE_RSP,[this](ReqId id,quint16 len,QByteArray message){
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
            qDebug()<<"Nofity message failed. Error is "<<static_cast<int>(error);
            emit sig_login_failed(error);
            return ;
        }
        int error=jsonObj["error"].toInt();
        if(static_cast<ErrorCode>(error)!=ErrorCode::SUCCESS){
            qDebug()<<"Error message Get";
            emit sig_login_failed(static_cast<ErrorCode>(error));
            return;
        }


        auto file=FileToken::fromJson(jsonObj);
        auto filemsg=std::make_shared<FileToken>(file);

        emit sig_download_file_rsp(filemsg);
    });

    _handlers.insert(ReqId::ID_HEARTBEAT_RSP,[this](ReqId id,quint16 len,QByteArray message){
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
            qDebug()<<"Error message Get In HeartBeat";
            return;
        }



        cout<<"Receive heartbeat message response";
    });

    _handlers.insert(ReqId::ID_LOAD_CHAT_THREAD_RSP,[this](ReqId id,quint16 len,QByteArray message){
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
            qDebug()<<"Nofity message failed. Error is "<<static_cast<int>(error);
            emit sig_login_failed(error);
            return ;
        }
        int error=jsonObj["error"].toInt();
        if(static_cast<ErrorCode>(error)!=ErrorCode::SUCCESS){
            qDebug()<<"Error message Get";
            emit sig_login_failed(static_cast<ErrorCode>(error));
            return;
        }


        auto threads=jsonObj["threads"].toArray();
        vector<std::shared_ptr<ChatThreadInfo>> chat;
        for(const QJsonValue& t:threads){
            auto thread=std::make_shared<ChatThreadInfo>();
            thread->_thread_id=t["thread_id"].toInt();
            thread->_type=t["type"].toString();
            thread->_user1_id=t["uid_1"].toInt();
            thread->_user2_id=t["uid_2"].toInt();
            chat.push_back(thread);
        }

        bool next_page=jsonObj["load_more"].toBool();
        int next_id=jsonObj["last_thread_id"].toInt();

        emit sig_load_chat_thread(next_page,next_id,chat);
    });

    _handlers.insert(ReqId::ID_CREATE_PRIVATE_CHAT_RSP,[this](ReqId id,quint16 len,QByteArray message){
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
            qDebug()<<"Nofity message failed. Error is "<<static_cast<int>(error);
            emit sig_login_failed(error);
            return ;
        }
        int error=jsonObj["error"].toInt();
        if(static_cast<ErrorCode>(error)!=ErrorCode::SUCCESS){
            qDebug()<<"Error message Get";
            emit sig_login_failed(static_cast<ErrorCode>(error));
            return;
        }

        int user1_id=jsonObj["uid"].toInt();
        int user2_id=jsonObj["other_uid"].toInt();
        int thread_id=jsonObj["thread_id"].toInt();

        emit sig_create_private_chat(user1_id,user2_id,thread_id);
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

void TcpMgr::CloseConnection()
{
    _socket.disconnectFromHost();

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


