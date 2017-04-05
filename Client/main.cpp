#include "mainwindow.h"
#include <QApplication>
#include "client.h"
#include <thread>


Client client;

void client_start(){
    client.startTCP();
}


int main(int argc, char *argv[])
{
    qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "1");
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    std::thread(client_start).detach();

    QObject::connect(&w, SIGNAL( requestSong(QString) ), &client, SLOT( requestSong(QString) ));

    return a.exec();
}
