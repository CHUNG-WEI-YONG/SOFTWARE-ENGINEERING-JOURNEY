#include "clickedlabel.h"
#include <QMouseEvent>
ClickedLabel::ClickedLabel(QWidget *parent):QLabel(parent),_curstate(ClickLbState::Normal)
{
this->setCursor(Qt::PointingHandCursor);
}

void ClickedLabel::enterEvent(QEnterEvent *e)
{
    if(_curstate==ClickLbState::Normal){
        //qDebug()<<"User hover , change to hover";
        setProperty("state",_normal_hover);
        repolish(this);
        update();

    }
    else if(_curstate==ClickLbState::Selected){
        //qDebug()<<"Enter m change to selected_hover";
        setProperty("state",_selected_hover);
        repolish(this);
        update();
    }
    QLabel::enterEvent(e);
}

void ClickedLabel::leaveEvent(QEvent *event)
{
    if(_curstate==ClickLbState::Normal){
        //qDebug()<<"leave  , change to normal";
        setProperty("state",_normal);
        repolish(this);
        update();

    }
    else if(_curstate==ClickLbState::Selected){
        //qDebug()<<"Enter , change to selected";
        setProperty("state",_selected);
        repolish(this);
        update();
    }
    QLabel::leaveEvent(event);
}

// void ClickedLabel::mousePressEvent(QMouseEvent *e)
// {
//     if(e->button()==Qt::LeftButton){
//         if(_curstate==ClickLbState::Normal){
//             _curstate=ClickLbState::Selected;
//             qDebug()<<"Normal seen change to selected seen";
//             setProperty("state",_selected_hover);
//             repolish(this);
//             update();
//         }
//         else{
//             _curstate=ClickLbState::Normal;
//             qDebug()<<"clicked change to normal hover";
//             setProperty("state",_normal_hover);
//             repolish(this);
//             update();
//         }
//         emit clicked();

//     }
//     QLabel::mousePressEvent(e);
// }

void ClickedLabel::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton){
        if(_curstate == ClickLbState::Normal){
            // ──► 🎯 核心修正：锁定普通按下态 ◄──
            setProperty("state", _normal_press);
        }
        else{
            // ──► 🎯 核心修正：锁定选中状态下的按下态 ◄──
            setProperty("state", _selected_press);
        }
        repolish(this);
        update();
    }
    QLabel::mousePressEvent(e);
}

// 🚀 2. 鼠标抬起瞬间：真正开始结算状态翻转，展示【悬停态】并触发点击信号
void ClickedLabel::mouseReleaseEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton){
        // 判定用户是在当前控件内部释放鼠标的（防止用户点着点着划走了，提升交互严密性）
        if(this->rect().contains(e->pos())){
            if(_curstate == ClickLbState::Normal){
                // 彻底蜕变为选中态
                _curstate = ClickLbState::Selected;
                //qDebug() << "🎯 [状态机] 鼠标释放：从 Normal 正式蜕变为 Selected_Hover";
                setProperty("state", _selected_hover);
            }
            else{
                // 回归普通态
                _curstate = ClickLbState::Normal;
                //qDebug() << "🎯 [状态机] 鼠标释放：从 Selected 正式翻转回 Normal_Hover";
                setProperty("state", _normal_hover);
            }
            repolish(this);
            update();

            // 判定成功，正式引爆点击业务总线信号
            emit clicked(this->text(),_curstate);
        }
    }
    QLabel::mouseReleaseEvent(e);
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
}

bool ClickedLabel::SetCurState(ClickLbState lb)
{
    _curstate=lb;
    if(_curstate==ClickLbState::Normal){
        setProperty("state",_normal);
        repolish(this);
    }
    else if(_curstate==ClickLbState::Selected){
        setProperty("state",_selected);
        repolish(this);
    }
    return true;
}

void ClickedLabel::ResetNormalState()
{
    _curstate=ClickLbState::Normal;
    setProperty("state",_normal);
    repolish(this);
};


