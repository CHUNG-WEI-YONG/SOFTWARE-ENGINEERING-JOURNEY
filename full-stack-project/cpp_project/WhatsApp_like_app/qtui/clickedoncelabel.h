#ifndef CLICKEDONCELABEL_H
#define CLICKEDONCELABEL_H

#include <QLabel>
#include <QMouseEvent>

class ClickedOnceLabel : public QLabel
{
    Q_OBJECT;
public:

    ClickedOnceLabel(QWidget *parent=nullptr);
    virtual void mouseReleaseEvent(QMouseEvent *event)override;

signals:
    void Clicked(QString);

};

#endif // CLICKEDONCELABEL_H
