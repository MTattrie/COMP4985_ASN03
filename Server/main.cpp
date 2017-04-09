#include "mainwindow.h"
#include <QApplication>
#include  "server.h"
#include <thread>



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    Server s;
    s.start();

    //QObject::connect(&server, SIGNAL( update_log(QString) ), &w, SLOT( output_log(QString) ));
    //QObject::connect(&server, SIGNAL( update_log2(QString) ), &w, SLOT( output_log2(QString) ));

    return a.exec();
}
