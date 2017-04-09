#include "audioplayer.h"

AudioPlayer::AudioPlayer(): audio_pos(0), playing(false), fastForwarding(false), paused(false)
{

}
bool AudioPlayer::openWavFile(const QString &fileName){
    paused = false;
    playing = false;
    fastForwarding = false;
    if(!sourceFile.open(fileName))
        return false;
    audio_buffer.resize(0);
    audio_header.resize(0);
    audio_pos = 0;
    audio_buffer = sourceFile.readAll();
    audio_header = audio_buffer.mid(0, sourceFile.headerLength());
    open(QIODevice::ReadOnly);
    return true;
}



const QAudioFormat& AudioPlayer::fileFormat() const{
    return sourceFile.fileFormat();
}

qint64 AudioPlayer::headerLength() const{
    return sourceFile.headerLength();
}

QByteArray AudioPlayer::readHeaderData(){
    return audio_header;
}

QByteArray AudioPlayer::readChunkData(qint64 len, qint64 pos){
    if(pos + len < audio_buffer.size()){
        return audio_buffer.mid(pos, len);
        //memcpy(data, audio_buffer.constData() + pos, len);
    }
    return nullptr;
}

qint64 AudioPlayer::readData(char *data, qint64 len){
    qint64 chunk = 0;
    if (playing && audio_pos < audio_buffer.size()) {
        chunk = qMin((audio_buffer.size() - audio_pos), len);
        memcpy(data, audio_buffer.constData() + audio_pos, chunk);

        emit streamChunkAudio(chunk, audio_pos);

        audio_pos = audio_pos + chunk;

        if(audio_pos >= audio_buffer.size()){
            playing = false;
            qDebug() << "songFinished ";
            emit songFinished();
        }
    }
    return chunk;
}


qint64 AudioPlayer::writeData(const char *data, qint64 len){
    return 0;
}

qint64 AudioPlayer::bytesAvailable() const{
    return 0;
}
qint64 AudioPlayer::pos() const{
    return audio_pos;
}
bool AudioPlayer::seek(qint64 pos){
    if(pos < 0 || pos > audio_buffer.size())
        return false;

    audio_pos = pos;
    return true;
}


bool AudioPlayer::pause(){
    if(playing) {
        paused = true;
        playing = false;
        //close();
    }
    return true;
}

bool AudioPlayer::start(){
    playing = true;
    if(paused)
        paused = false;
    //open(QIODevice::ReadOnly);
    return playing;
}

bool AudioPlayer::isPlaying(){
    return playing;
}

bool AudioPlayer::isFastForwarding(){
    return fastForwarding;
}

bool AudioPlayer::isPaused(){
    return paused;
}

bool AudioPlayer::isFastForwarding(bool forward){
    fastForwarding = forward;
    return fastForwarding;
}

