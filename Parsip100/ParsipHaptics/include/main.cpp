#include <QtGui/QApplication>
#include <QString>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
        if(argc > 1)
        {
                QString str(argv[1]);
                w.setCommandLineParam(str);
        }
    w.show();

    return a.exec();
}
