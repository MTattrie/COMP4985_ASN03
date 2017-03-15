#include "mainwindow.h"
#include <QApplication>
#include  "server.h"
#include <thread>

Server     server;

void s_startTCP(){
    server.startTCP();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();


    std::thread(s_startTCP).detach();

    //QObject::connect(&server, SIGNAL( update_log(QString) ), &w, SLOT( output_log(QString) ));
    //QObject::connect(&server, SIGNAL( update_log2(QString) ), &w, SLOT( output_log2(QString) ));

    return a.exec();
}
