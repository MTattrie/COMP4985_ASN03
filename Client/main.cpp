#include "mainwindow.h"
#include <QApplication>
#include "client.h"
#include <thread>


Client client;

void client_start(){
    client.start();
}


int main(int argc, char *argv[])
{
    qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "1");
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    //std::thread(client_start).detach();

    return a.exec();
}
