#ifndef AUDIOBUFFER_H
#define AUDIOBUFFER_H

#include <QIODevice>
#include <QDebug>
#include "wavfile.h"



class AudioPlayer :public QIODevice
{
    Q_OBJECT
public:
    AudioPlayer();
    bool openWavFile(const QString &fileName);
    const QAudioFormat &fileFormat() const;
    qint64 headerLength() const;
    qint64 pos() const;
    bool seek(qint64 pos);
    bool pause();
    bool start();
    QAudioFormat &fastForward();
    bool isPlaying();
    bool isPaused();
    bool isFastForwarding();
    bool isFastForwarding(bool forward);
    QByteArray readHeaderData();
    QByteArray readChunkData(qint64 len, qint64 pos);


private:
    qint64     audio_pos;
    QByteArray audio_buffer;
    QByteArray audio_header;
    WavFile sourceFile;

    bool playing;
    bool fastForwarding;
    bool paused;

protected:
     qint64 readData(char *data, qint64 len);
     qint64 writeData(const char *data, qint64 len);
     qint64 bytesAvailable() const;

signals:
    void songFinished();
    void streamChunkAudio(qint64 chunk, qint64 pos);
};

#endif // AUDIOPLAYER_H
