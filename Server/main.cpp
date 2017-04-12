#include "mainwindow.h"
#include <QApplication>
#include  "server.h"
#include <thread>



int main(int argc, char *argv[])
{
    qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "1");
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    //QObject::connect(&server, SIGNAL( update_log(QString) ), &w, SLOT( output_log(QString) ));
    //QObject::connect(&server, SIGNAL( update_log2(QString) ), &w, SLOT( output_log2(QString) ));

    return a.exec();
}
