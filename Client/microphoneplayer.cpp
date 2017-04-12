#include "microphoneplayer.h"

MicrophonePlayer::MicrophonePlayer(): buffer_in(0), buffer_out(0), playing(false)
{
    playing = false;
}

void MicrophonePlayer::resetPlayer(){
    playing = false;
    buffer_in.clear();
    buffer_out.clear();
}
QByteArray MicrophonePlayer::readChunkData(qint64 len){
    qint64 chunk = 0;
    chunk = qMin((qint64)buffer_in.size(), len);
    QByteArray data(buffer_in.mid(0, chunk));
    buffer_in.remove(0, chunk);
    return data;
}

qint64 MicrophonePlayer::readData(char *data, qint64 len){
    qDebug()<<"readData : " << buffer_out.size();
    qint64 chunk = 0;
    chunk = qMin((qint64)buffer_out.size(), len);
    memcpy(data, buffer_out.constData(), chunk);
    buffer_out.remove(0, chunk);
    return chunk;
}


qint64 MicrophonePlayer::writeData(const char *data, qint64 len){
    buffer_in.append(QByteArray::fromRawData(data,len));
    emit recorded(len);
    return len;
}

qint64 MicrophonePlayer::bytesAvailable() const{
    return buffer_out.size();
}

bool MicrophonePlayer::stop(){
    playing = false;
    close();
    return true;
}

bool MicrophonePlayer::start(){
    playing = true;
    open(QIODevice::ReadWrite);
    return playing;
}

bool MicrophonePlayer::isPlaying(){
    return playing;
}


bool MicrophonePlayer::addChunkData(const char *data, qint64 len){
    buffer_out.append(QByteArray::fromRawData(data,len));
    qDebug()<<"addChunkData " << buffer_out.size();

    return true;
}
