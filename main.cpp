#include "MainWindow.h"
#include <QApplication>
#include <QSettings> // Inclus

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    // Configuration OBLIGATOIRE pour QSettings
    QCoreApplication::setOrganizationName("UITPadCorp");
    QCoreApplication::setApplicationName("UITPadEditor");

    MainWindow w;
    w.show();

    return a.exec();
}
