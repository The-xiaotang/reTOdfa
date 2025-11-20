/********************************************************************************
** Form generated from reading UI file 'widget.ui'
**
** Created by: Qt User Interface Compiler version 6.10.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Widget
{
public:
    QPlainTextEdit *input;
    QLabel *label;
    QTabWidget *tabWidget;
    QWidget *displayNFA;
    QTableWidget *NFAtable;
    QWidget *displayDFA;
    QTableWidget *DFAtable;
    QWidget *displayMIN_DFA;
    QTableWidget *MIN_DFAtable;
    QPushButton *run;
    QPushButton *upload;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName("Widget");
        Widget->resize(1010, 857);
        input = new QPlainTextEdit(Widget);
        input->setObjectName("input");
        input->setGeometry(QRect(190, 30, 341, 81));
        label = new QLabel(Widget);
        label->setObjectName("label");
        label->setGeometry(QRect(30, 30, 291, 31));
        QFont font;
        font.setPointSize(20);
        label->setFont(font);
        tabWidget = new QTabWidget(Widget);
        tabWidget->setObjectName("tabWidget");
        tabWidget->setGeometry(QRect(30, 130, 981, 581));
        displayNFA = new QWidget();
        displayNFA->setObjectName("displayNFA");
        NFAtable = new QTableWidget(displayNFA);
        NFAtable->setObjectName("NFAtable");
        NFAtable->setGeometry(QRect(5, 1, 971, 551));
        tabWidget->addTab(displayNFA, QString());
        displayDFA = new QWidget();
        displayDFA->setObjectName("displayDFA");
        DFAtable = new QTableWidget(displayDFA);
        DFAtable->setObjectName("DFAtable");
        DFAtable->setGeometry(QRect(5, 1, 741, 421));
        tabWidget->addTab(displayDFA, QString());
        displayMIN_DFA = new QWidget();
        displayMIN_DFA->setObjectName("displayMIN_DFA");
        MIN_DFAtable = new QTableWidget(displayMIN_DFA);
        MIN_DFAtable->setObjectName("MIN_DFAtable");
        MIN_DFAtable->setGeometry(QRect(5, 1, 741, 421));
        tabWidget->addTab(displayMIN_DFA, QString());
        run = new QPushButton(Widget);
        run->setObjectName("run");
        run->setGeometry(QRect(550, 30, 101, 81));
        upload = new QPushButton(Widget);
        upload->setObjectName("upload");
        upload->setGeometry(QRect(30, 80, 141, 31));

        retranslateUi(Widget);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QCoreApplication::translate("Widget", "Widget", nullptr));
        label->setText(QCoreApplication::translate("Widget", "\346\255\243\345\210\231\350\241\250\350\276\276\345\274\217", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(displayNFA), QCoreApplication::translate("Widget", "NFA", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(displayDFA), QCoreApplication::translate("Widget", "DFA", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(displayMIN_DFA), QCoreApplication::translate("Widget", "MIN_DFA", nullptr));
        run->setText(QCoreApplication::translate("Widget", "\350\277\220\350\241\214", nullptr));
        upload->setText(QCoreApplication::translate("Widget", "\344\270\212\344\274\240\346\226\207\344\273\266", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H
