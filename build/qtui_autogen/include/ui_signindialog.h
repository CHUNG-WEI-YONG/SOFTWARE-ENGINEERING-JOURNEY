/********************************************************************************
** Form generated from reading UI file 'signindialog.ui'
**
** Created by: Qt User Interface Compiler version 6.11.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SIGNINDIALOG_H
#define UI_SIGNINDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SigninDialog
{
public:
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout;
    QStackedWidget *stackedWidget;
    QWidget *page;
    QVBoxLayout *verticalLayout_3;
    QSpacerItem *verticalSpacer_2;
    QWidget *widget;
    QHBoxLayout *horizontalLayout_7;
    QLabel *error_label;
    QHBoxLayout *horizontalLayout;
    QLabel *user_label;
    QLineEdit *user_edit;
    QHBoxLayout *horizontalLayout_2;
    QLabel *email_label;
    QLineEdit *email_edit;
    QHBoxLayout *horizontalLayout_3;
    QLabel *pass_label;
    QLineEdit *pass_edit;
    QHBoxLayout *horizontalLayout_4;
    QLabel *repass_label;
    QLineEdit *repass_edit;
    QHBoxLayout *horizontalLayout_5;
    QLabel *code_label;
    QLineEdit *code_edit;
    QPushButton *get_Code;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout_6;
    QSpacerItem *horizontalSpacer;
    QPushButton *confirm_btn;
    QPushButton *cancel_btn;
    QSpacerItem *horizontalSpacer_2;
    QSpacerItem *verticalSpacer_3;
    QWidget *page_2;

    void setupUi(QDialog *SigninDialog)
    {
        if (SigninDialog->objectName().isEmpty())
            SigninDialog->setObjectName("SigninDialog");
        SigninDialog->resize(300, 500);
        SigninDialog->setMinimumSize(QSize(300, 500));
        SigninDialog->setMaximumSize(QSize(300, 500));
        verticalLayout_2 = new QVBoxLayout(SigninDialog);
        verticalLayout_2->setObjectName("verticalLayout_2");
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName("verticalLayout");
        stackedWidget = new QStackedWidget(SigninDialog);
        stackedWidget->setObjectName("stackedWidget");
        page = new QWidget();
        page->setObjectName("page");
        verticalLayout_3 = new QVBoxLayout(page);
        verticalLayout_3->setObjectName("verticalLayout_3");
        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout_3->addItem(verticalSpacer_2);

        widget = new QWidget(page);
        widget->setObjectName("widget");
        horizontalLayout_7 = new QHBoxLayout(widget);
        horizontalLayout_7->setObjectName("horizontalLayout_7");
        error_label = new QLabel(widget);
        error_label->setObjectName("error_label");
        error_label->setAlignment(Qt::AlignmentFlag::AlignCenter);

        horizontalLayout_7->addWidget(error_label);


        verticalLayout_3->addWidget(widget);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        user_label = new QLabel(page);
        user_label->setObjectName("user_label");
        user_label->setMinimumSize(QSize(0, 25));
        user_label->setMaximumSize(QSize(16777215, 25));

        horizontalLayout->addWidget(user_label);

        user_edit = new QLineEdit(page);
        user_edit->setObjectName("user_edit");
        user_edit->setMinimumSize(QSize(0, 25));
        user_edit->setMaximumSize(QSize(16777215, 25));

        horizontalLayout->addWidget(user_edit);


        verticalLayout_3->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        email_label = new QLabel(page);
        email_label->setObjectName("email_label");
        email_label->setMinimumSize(QSize(0, 25));
        email_label->setMaximumSize(QSize(16777215, 25));

        horizontalLayout_2->addWidget(email_label);

        email_edit = new QLineEdit(page);
        email_edit->setObjectName("email_edit");
        email_edit->setMinimumSize(QSize(0, 25));
        email_edit->setMaximumSize(QSize(16777215, 25));

        horizontalLayout_2->addWidget(email_edit);


        verticalLayout_3->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        pass_label = new QLabel(page);
        pass_label->setObjectName("pass_label");
        pass_label->setMinimumSize(QSize(0, 25));
        pass_label->setMaximumSize(QSize(16777215, 25));

        horizontalLayout_3->addWidget(pass_label);

        pass_edit = new QLineEdit(page);
        pass_edit->setObjectName("pass_edit");
        pass_edit->setMinimumSize(QSize(0, 25));
        pass_edit->setMaximumSize(QSize(16777215, 25));
        pass_edit->setEchoMode(QLineEdit::EchoMode::Password);

        horizontalLayout_3->addWidget(pass_edit);


        verticalLayout_3->addLayout(horizontalLayout_3);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName("horizontalLayout_4");
        repass_label = new QLabel(page);
        repass_label->setObjectName("repass_label");
        repass_label->setMinimumSize(QSize(0, 25));
        repass_label->setMaximumSize(QSize(16777215, 25));

        horizontalLayout_4->addWidget(repass_label);

        repass_edit = new QLineEdit(page);
        repass_edit->setObjectName("repass_edit");
        repass_edit->setMinimumSize(QSize(0, 25));
        repass_edit->setMaximumSize(QSize(16777215, 25));
        repass_edit->setEchoMode(QLineEdit::EchoMode::Password);

        horizontalLayout_4->addWidget(repass_edit);


        verticalLayout_3->addLayout(horizontalLayout_4);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName("horizontalLayout_5");
        code_label = new QLabel(page);
        code_label->setObjectName("code_label");
        code_label->setMinimumSize(QSize(0, 25));
        code_label->setMaximumSize(QSize(16777215, 25));

        horizontalLayout_5->addWidget(code_label);

        code_edit = new QLineEdit(page);
        code_edit->setObjectName("code_edit");
        code_edit->setMinimumSize(QSize(0, 25));
        code_edit->setMaximumSize(QSize(16777215, 25));
        code_edit->setEchoMode(QLineEdit::EchoMode::Password);

        horizontalLayout_5->addWidget(code_edit);

        get_Code = new QPushButton(page);
        get_Code->setObjectName("get_Code");
        get_Code->setMinimumSize(QSize(0, 25));
        get_Code->setMaximumSize(QSize(16777215, 25));

        horizontalLayout_5->addWidget(get_Code);


        verticalLayout_3->addLayout(horizontalLayout_5);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout_3->addItem(verticalSpacer);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName("horizontalLayout_6");
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer);

        confirm_btn = new QPushButton(page);
        confirm_btn->setObjectName("confirm_btn");
        confirm_btn->setMinimumSize(QSize(0, 25));
        confirm_btn->setMaximumSize(QSize(16777215, 25));

        horizontalLayout_6->addWidget(confirm_btn);

        cancel_btn = new QPushButton(page);
        cancel_btn->setObjectName("cancel_btn");
        cancel_btn->setMinimumSize(QSize(0, 25));
        cancel_btn->setMaximumSize(QSize(16777215, 25));

        horizontalLayout_6->addWidget(cancel_btn);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_2);


        verticalLayout_3->addLayout(horizontalLayout_6);

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout_3->addItem(verticalSpacer_3);

        stackedWidget->addWidget(page);
        page_2 = new QWidget();
        page_2->setObjectName("page_2");
        stackedWidget->addWidget(page_2);

        verticalLayout->addWidget(stackedWidget);


        verticalLayout_2->addLayout(verticalLayout);


        retranslateUi(SigninDialog);

        stackedWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(SigninDialog);
    } // setupUi

    void retranslateUi(QDialog *SigninDialog)
    {
        SigninDialog->setWindowTitle(QCoreApplication::translate("SigninDialog", "Dialog", nullptr));
        error_label->setText(QCoreApplication::translate("SigninDialog", "Error Message", nullptr));
        user_label->setText(QCoreApplication::translate("SigninDialog", "User: ", nullptr));
        email_label->setText(QCoreApplication::translate("SigninDialog", "Email: ", nullptr));
        pass_label->setText(QCoreApplication::translate("SigninDialog", "Password: ", nullptr));
        repass_label->setText(QCoreApplication::translate("SigninDialog", "Reenter: ", nullptr));
        code_label->setText(QCoreApplication::translate("SigninDialog", "Verify Code :", nullptr));
        get_Code->setText(QCoreApplication::translate("SigninDialog", "GET", nullptr));
        confirm_btn->setText(QCoreApplication::translate("SigninDialog", "Confirm", nullptr));
        cancel_btn->setText(QCoreApplication::translate("SigninDialog", "Cancel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SigninDialog: public Ui_SigninDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SIGNINDIALOG_H
