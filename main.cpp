#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QStringList args = a.arguments();
    QString author = args.count() > 1 ? args.at(1) : "";
    MainWindow w (author);
    w.show();
    return a.exec();
}
