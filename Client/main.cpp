#include "mainwindow.h"
#include <QApplication>
#include "client.h"
#include <thread>


int main(int argc, char *argv[])
{
    qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "1");
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    Client c;

    QObject::connect(&w, SIGNAL( requestSong(QString) ), &c, SLOT( requestSong(QString) ));

    c.startThreads();

    return a.exec();
}
