#include "mainwindow.h"
#include <QApplication>
#include  "server.h"

Server     server;

void s_startTCP(){
    server.startTCP();
}
void s_startUDP(){
    server.startUDP();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();


    //std::thread(s_startTCP).detach();
    //std::thread(s_startUDP).detach();

    //QObject::connect(&server, SIGNAL( update_log(QString) ), &w, SLOT( output_log(QString) ));
    //QObject::connect(&server, SIGNAL( update_log2(QString) ), &w, SLOT( output_log2(QString) ));

    return a.exec();
}
