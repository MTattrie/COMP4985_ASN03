#ifndef MICROPHONEPLAYER_H
#define MICROPHONEPLAYER_H

#include <QIODevice>
#include <QDebug>
#include "wavfile.h"

class MicrophonePlayer :public QIODevice
{
    Q_OBJECT
public:
    MicrophonePlayer();
    bool stop();
    bool start();
    bool isPlaying();
    QByteArray readChunkData(qint64 len);
    bool addChunkData(const char *data, qint64 len);
    void resetPlayer();
    qint64 bytesAvailable() const;

private:
    QByteArray buffer_in;
    QByteArray buffer_out;
    bool playing;

protected:
     qint64 readData(char *data, qint64 len);
     qint64 writeData(const char *data, qint64 len);

signals:
    void recorded(qint64);

};

#endif // MICROPHONEPLAYER_H
