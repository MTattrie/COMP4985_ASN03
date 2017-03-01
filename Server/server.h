#ifndef SERVER_H
#define SERVER_H

#include <QObject>

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = 0);

    void startTCP();
    void startUDP();

signals:
    void update_log(QString packet);
    void update_log2(QString packet);

public slots:
};

#endif // SERVER_H
