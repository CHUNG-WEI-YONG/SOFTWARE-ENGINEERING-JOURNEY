#ifndef QMLCHATPAGE_H
#define QMLCHATPAGE_H

#include <QWidget>

namespace Ui {
class QMLChatPage;
}

class QMLChatPage : public QWidget
{
    Q_OBJECT

public:
    explicit QMLChatPage(QWidget *parent = nullptr);
    ~QMLChatPage();

private:
    Ui::QMLChatPage *ui;
};

#endif // QMLCHATPAGE_H
