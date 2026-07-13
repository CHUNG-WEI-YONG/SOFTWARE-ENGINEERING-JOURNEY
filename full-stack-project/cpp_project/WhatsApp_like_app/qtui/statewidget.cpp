#include "statewidget.h"
#include <QStyleOption>
#include <QPainter>
#include <QLabel>
#include <QVBoxLayout>
#include <QMouseEvent>


StateWidget::StateWidget(QWidget *parent):QWidget(parent),_curstate(ClickLbState::Normal)
{
    setCursor(Qt::PointingHandCursor);
    AddRedPoint();
}

void StateWidget::AddRedPoint(){
    _red_point=new QLabel();
    _red_point->setObjectName("red_point");
    QVBoxLayout *layout2=new QVBoxLayout();
    _red_point->setAlignment(Qt::AlignCenter);
    layout2->addWidget(_red_point);
    layout2->setContentsMargins(0,0,0,0);
    this->setLayout(layout2);
    _red_point->setVisible(false);

}

void StateWidget::ShowRedPoint(bool show)
{
    _red_point->setVisible(show);
}

void StateWidget::SetState(QString normal, QString hover, QString press, QString select, QString select_hover, QString select_press)
{
    _normal = normal;
    _normal_hover = hover;
    _normal_press = press;

    _selected = select;
    _selected_hover = select_hover;
    _selected_press = select_press;

    setProperty("state",normal);
    repolish(this);
}

ClickLbState StateWidget::GetCurState()
{
    return _curstate;
}

void StateWidget::ClearState()
{
    _curstate = ClickLbState::Normal;
    setProperty("state",_normal);
    repolish(this);
    update();
}

void StateWidget::SetSelected(bool selected)
{
    if(selected){
        _curstate=ClickLbState::Selected;
        setProperty("state",_selected);
        repolish(this);
        update();
    }
    else{
        _curstate=ClickLbState::Normal;
        setProperty("state",_normal);
        repolish(this);
        update();
    }
}

void StateWidget::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this); // 👈 Qt 6 钦定平替 init(this)，更加安全稳固

    // 2. 唤醒物理画布
    QPainter p(this);

    // 3. 强力驱动 QStyle 引擎，将 QSS 中配置的 state 皮肤背景硬核绘制上屏
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    // 4. ──► 🎯 核心修正：顺手调用基类的 paintEvent，确保事件总线闭环运转，并删除冗余 return ◄──
    QWidget::paintEvent(event);
}


void StateWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if(_curstate == ClickLbState::Selected){
            qDebug()<<"PressEvent , already to selected press: "<< _selected_press;
            //emit clicked();
            // 调用基类的mousePressEvent以保证正常的事件处理
            QWidget::mousePressEvent(event);
            return;
        }

        if(_curstate == ClickLbState::Normal){
            qDebug()<<"PressEvent , change to selected press: "<< _selected_press;
            _curstate = ClickLbState::Selected;
            setProperty("state",_selected_press);
            repolish(this);
            update();
        }

        return;
    }
    // 调用基类的mousePressEvent以保证正常的事件处理
    QWidget::mousePressEvent(event);
}

void StateWidget::mouseReleaseEvent(QMouseEvent *event){
    if(event->button()==Qt::LeftButton){
        if(_curstate==ClickLbState::Normal){
            setProperty("state",_normal_hover);
            repolish(this);
            update();
        }
        else{
            setProperty("state",_selected_hover);
            repolish(this);
            update();
        }
        emit clicked();
        return ;
    }
    QWidget::mouseReleaseEvent(event);
}

void StateWidget::enterEvent(QEnterEvent *event) // 👈 核心修正 1：平替为 Qt 6 钦定的 QEnterEvent
{
    // 处理鼠标悬停进入的逻辑
    if (_curstate == ClickLbState::Normal) {
        // ──► 🎯 核心修正 2：如果需要，可以在这里同步更新你的内部状态枚举 ◄──
        // _curstate = ClickLbState::Hover; // 如果你定义了 Hover 状态的话

        setProperty("state", _normal_hover);
        repolish(this);
        update();
    } else if (_curstate == ClickLbState::Selected) { // 👈 建议用显式 else if 防御非正常状态
        setProperty("state", _selected_hover);
        repolish(this);
        update();
    }

    // 消费事件流，确保基类总线闭环
    QWidget::enterEvent(event);
}
void StateWidget::leaveEvent(QEvent *event)
{
    // 鼠标划走时，必须让换装归位
    if (_curstate == ClickLbState::Normal) {
        setProperty("state", _normal); // 退回初始正常状态
    } else {
        setProperty("state", _selected); // 退回被选中的状态
    }
    repolish(this);
    update();

    QWidget::leaveEvent(event);
}