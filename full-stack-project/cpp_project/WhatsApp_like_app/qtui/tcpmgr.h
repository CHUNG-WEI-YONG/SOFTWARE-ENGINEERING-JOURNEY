#ifndef TCPMGR_H
#define TCPMGR_H
#include "global.h"
#include <QTcpSocket>
#include "singleton.h"
#include <functional>
#include <QObject>
#include "userdata.h"
class TcpMgr:public QObject , public Singleton<TcpMgr>, public std::enable_shared_from_this<TcpMgr>
{
    Q_OBJECT
public:
    ~TcpMgr();
private:
    TcpMgr();
    friend Singleton<TcpMgr>;
    void initHandlers();
    void HandleMessage(ReqId d,int len,QByteArray data);
    QTcpSocket _socket;
    QString _host;
    uint16_t _port;
    QByteArray _buffer;
    bool _b_recv_pending;
    quint16 _message_id;
    quint16 _message_len;
    QMap<ReqId,std::function<void(ReqId d,int len,QByteArray data)>>_handlers;

public slots:
    void slot_tcp_connect(Serverinfo);
    void slot_send_data(ReqId id, QString data);

signals:
    void sig_con_success(bool bsuccess=true);
    void sig_send_data(ReqId id ,QString data);
    void sig_switch_chat_dlg();
    void login_failed(int);
    void sig_login_failed(ErrorCode err);
    void sig_user_search(std::shared_ptr<SearchInfo> );

};

#endif // TCPMGR_H
