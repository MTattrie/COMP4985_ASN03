#ifndef MICROPHONEPLAYER_H
#define MICROPHONEPLAYER_H

#include <QIODevice>
#include <QDebug>
#include "wavfile.h"

class MicrophonePlayer : public QIODevice
{
    Q_OBJECT
public:
    MicrophonePlayer();
    bool stop();
    bool start();
    bool isPlaying();
    QByteArray readChunkData(qint64);
    bool addChunkData(const char *, qint64);
    void resetPlayer();
    qint64 bytesAvailable() const;

private:
    QByteArray buffer_in;
    QByteArray buffer_out;
    bool playing;

protected:
     qint64 readData(char *, qint64);
     qint64 writeData(const char *, qint64);

signals:
    void recorded(qint64);
};

#endif // MICROPHONEPLAYER_H
