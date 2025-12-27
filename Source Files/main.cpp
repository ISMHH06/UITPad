#include "MainWindow.h"

#include <QApplication>
#include <QCoreApplication>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("UITPad");
    QCoreApplication::setApplicationName("UITPad");
    MainWindow w;
    w.show();
    return a.exec();
}
