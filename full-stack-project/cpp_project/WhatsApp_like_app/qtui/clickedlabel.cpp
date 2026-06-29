#include "clickedlabel.h"
#include <QMouseEvent>
ClickedLabel::ClickedLabel(QWidget *parent):QLabel(parent),_curstate(ClickLbState::Normal)
{
this->setCursor(Qt::PointingHandCursor);
}

void ClickedLabel::enterEvent(QEnterEvent *e)
{
    if(_curstate==ClickLbState::Normal){
        qDebug()<<"User hover , change to hover";
        setProperty("state",_normal_hover);
        repolish(this);
        update();

    }
    else if(_curstate==ClickLbState::Selected){
        qDebug()<<"Enter m change to selected_hover";
        setProperty("state",_selected_hover);
        repolish(this);
        update();
    }
    QLabel::enterEvent(e);
}

void ClickedLabel::leaveEvent(QEvent *event)
{
    if(_curstate==ClickLbState::Normal){
        qDebug()<<"leave  , change to normal";
        setProperty("state",_normal);
        repolish(this);
        update();

    }
    else if(_curstate==ClickLbState::Selected){
        qDebug()<<"Enter , change to selected";
        setProperty("state",_selected);
        repolish(this);
        update();
    }
    QLabel::leaveEvent(event);
}

void ClickedLabel::mousePressEvent(QMouseEvent *e)
{
    if(e->button()==Qt::LeftButton){
        if(_curstate==ClickLbState::Normal){
            _curstate=ClickLbState::Selected;
            qDebug()<<"Normal seen change to selected seen";
            setProperty("state",_selected_hover);
            repolish(this);
            update();
        }
        else{
            _curstate=ClickLbState::Normal;
            qDebug()<<"clicked change to normal hover";
            setProperty("state",_normal_hover);
            repolish(this);
            update();
        }
        emit clicked();

    }
    QLabel::mousePressEvent(e);
}

void ClickedLabel::setState(QString normal, QString hover, QString press, QString select, QString select_hover, QString select_press)
{
    _normal=normal;
    _normal_hover=hover;
    _normal_press = press;

    _selected = select;
    _selected_hover = select_hover;
    _selected_press = select_press;

    //this->setPixmap(QPixmap(":/rc/hidden.png"));

    setProperty("state", _normal);
    repolish(this);
    update();
    
}

ClickLbState ClickedLabel::GetCurState()
{
    return _curstate;
};


