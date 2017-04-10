#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

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
    bool addChunkData(const char *data, qint64 len);
    bool readHeader(char *data, qint64 len);
    void resetPlayer();
    qint64 bytesAvailable() const;
    qint64 audioBufferSize() const;


private:
    qint64     audio_pos;
    QByteArray audio_buffer;
    WavFile sourceFile;

    bool playing;
    bool fastForwarding;
    bool paused;

protected:
     qint64 readData(char *data, qint64 len);
     qint64 writeData(const char *data, qint64 len);

signals:
    void songFinished();
    void streamChunkAudio(qint64 chunk, qint64 pos);
};

#endif // AUDIOPLAYER_H
