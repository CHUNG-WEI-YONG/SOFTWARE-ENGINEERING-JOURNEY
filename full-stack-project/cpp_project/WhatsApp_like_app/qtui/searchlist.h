#ifndef SEARCHLIST_H
#define SEARCHLIST_H
#include <QListWidget>
#include <QWheelEvent>
#include <QScrollBar>
#include <QEvent>
#include <QDebug>
#include <QDialog>
#include <memory>
#include "loadingdialog.h"
#include "userdata.h"
#include "findsuccessdialog.h"
class SearchList:public QListWidget
{
public:
    SearchList(QWidget *parent=nullptr);
    void CloseSearch();
    void SetSearchEdit(QWidget *edit);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override {
        // 检查事件是否是鼠标悬浮进入或离开
        if (watched == this->viewport()) {
            if (event->type() == QEvent::Enter) {
                // 鼠标悬浮，显示滚动条
                this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            } else if (event->type() == QEvent::Leave) {
                // 鼠标离开，隐藏滚动条
                this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            }
        }

        // 检查事件是否是鼠标滚轮事件
        if (watched == this->viewport() && event->type() == QEvent::Wheel) {
            QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
            int numDegrees = wheelEvent->angleDelta().y() / 8;
            int numSteps = numDegrees / 15; // 计算滚动步数

            // 设置滚动幅度
            this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() - numSteps);

            return true; // 停止事件传递
        }

        return QListWidget::eventFilter(watched, event);
    }

private:
    void waitPending(bool pending=true);
    void CloseFindDialog();
    bool _search_pending;
    void AddTipItem();
    std::shared_ptr<QDialog> _find_dlg = nullptr;
    QWidget* _search_edit;
    LoadingDialog* _loadingDialog;
private slots:
    void slot_item_clicked(QListWidgetItem *item);
    void slot_user_search(std::shared_ptr<SearchInfo> si);
signals:
};

#endif // SEARCHLIST_H
