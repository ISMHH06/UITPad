/********************************************************************************
** Form generated from reading UI file 'UITPad.ui'
**
** Created by: Qt User Interface Compiler version 6.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_UITPAD_H
#define UI_UITPAD_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_UITPadClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *UITPadClass)
    {
        if (UITPadClass->objectName().isEmpty())
            UITPadClass->setObjectName("UITPadClass");
        UITPadClass->resize(600, 400);
        menuBar = new QMenuBar(UITPadClass);
        menuBar->setObjectName("menuBar");
        UITPadClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(UITPadClass);
        mainToolBar->setObjectName("mainToolBar");
        UITPadClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(UITPadClass);
        centralWidget->setObjectName("centralWidget");
        UITPadClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(UITPadClass);
        statusBar->setObjectName("statusBar");
        UITPadClass->setStatusBar(statusBar);

        retranslateUi(UITPadClass);

        QMetaObject::connectSlotsByName(UITPadClass);
    } // setupUi

    void retranslateUi(QMainWindow *UITPadClass)
    {
        UITPadClass->setWindowTitle(QCoreApplication::translate("UITPadClass", "UITPad", nullptr));
    } // retranslateUi

};

namespace Ui {
    class UITPadClass: public Ui_UITPadClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_UITPAD_H
