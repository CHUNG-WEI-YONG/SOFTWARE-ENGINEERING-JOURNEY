#include "chatuserlist.h"
#include <QListWidget>
#include <QWheelEvent>
#include <QEvent>
#include <QScrollBar>
#include <QDebug>
#include "usermgr.h"
#include <QTimer>

ChatUserList::ChatUserList(QWidget *parent):_load_pending(false)
{
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->viewport()->installEventFilter(this);
}

bool ChatUserList::eventFilter(QObject *watched, QEvent *event)
{
    if(watched==this->viewport()){
        if(event->type()==QEvent::Enter){
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        }
        else if(event->type()==QEvent::Leave){
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }
    }

    if(watched==this->viewport() && event->type()==QEvent::Wheel){
        QWheelEvent* wheelEvent=static_cast<QWheelEvent*>(event);
        int numDegrees = wheelEvent->angleDelta().y() / 8;
        int numSteps = numDegrees / 15; // 计算滚动步数

        // 设置滚动幅度
        this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() - numSteps);

        QScrollBar *scrollBar = this->verticalScrollBar();
        int maxScrollValue = scrollBar->maximum();
        int currentValue = scrollBar->value();
        //int pageSize = 10; // 每页加载的联系人数量

        if (maxScrollValue - currentValue <= 2) {
            // 滚动到底部，加载新的联系人
            bool all=UserMgr::getInstance()->isLoadChatFin();
            if(all){
                return true;
            }
            if(_load_pending){
                return true;
            }
            _load_pending=true;
            QTimer::singleShot(100,[this](){
                _load_pending=false;
                QCoreApplication::quit();
            });
            qDebug()<<"load more chat user";
            //发送信号通知聊天界面加载更多聊天内容
            emit sig_loading_user();
        }

        //return true;

    }

    return QListWidget::eventFilter(watched, event);
}
