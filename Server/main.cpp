#include "mainwindow.h"
#include <QApplication>
#include  "server.h"
#include <thread>


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: main
--
-- DATE: April 10, 2017
--
-- DESIGNER:
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: int main(int argc, char *argv[])
--
-- RETURNS: int.
--
-- NOTES:
--  Proigram entry point.
----------------------------------------------------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "1");
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
