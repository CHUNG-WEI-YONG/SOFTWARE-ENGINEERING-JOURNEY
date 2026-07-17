#include "clickedbutton.h"
#include "global.h"

ClickedButton::ClickedButton(QWidget* parent):QPushButton(parent) {
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::NoFocus);
}

ClickedButton::~ClickedButton(){}

void ClickedButton::SetState(QString normal, QString hover, QString press)
{
    _normal=normal;
    _hover=hover;
    _press=press;
    setProperty("state",_normal);
    repolish(this);
    update();
}

void ClickedButton::enterEvent(QEnterEvent *event)
{
    setProperty("state",_hover);
    repolish(this);
    update();
    QPushButton::enterEvent(event);
}

void ClickedButton::leaveEvent(QEvent *event)
{
    setProperty("state",_normal);
    repolish(this);
    update();
    QPushButton::leaveEvent(event);
}

void ClickedButton::mousePressEvent(QMouseEvent *event)
{
    setProperty("state",_press);
    repolish(this);
    update();
    QPushButton::mousePressEvent(event);
}

void ClickedButton::mouseReleaseEvent(QMouseEvent *event)
{
    setProperty("state",_hover);
    repolish(this);
    update();
    QPushButton::mouseReleaseEvent(event);
}

