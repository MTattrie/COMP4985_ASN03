/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: AudioPlayer.cpp - the audioplayer device that plays the actual music.
--
-- PROGRAM: Client (ComAudio - Final Project)
--
-- FUNCTIONS:
--  AudioPlayer();
--  bool openWavFile(const QString &fileName);
--  const QAudioFormat &fileFormat() const;
--  qint64 headerLength() const;
--  qint64 mypos() const;
--  bool myseek(qint64 pos);
--  bool pause();
--  bool start();
--  bool isPlaying();
--  bool isPaused();
--  bool isFastForwarding();
--  bool isFastForwarding(bool forward);
--  QByteArray readHeaderData();
--  QByteArray readChunkData(qint64 len, qint64 pos);
--  bool addChunkData(const char *data, qint64 len);
--  bool readHeader(char *data, qint64 len);
--  void resetPlayer();
--  qint64 bytesAvailable() const;
--  qint64 audioBufferSize() const;
--
-- DATE: April 11 2017
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- NOTES:
-- AudioPlayer extends QIODevice. It is used to open, read, write wav files.
----------------------------------------------------------------------------------------------------------------------*/
#include "audioplayer.h"


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: AudioPlayer() - constructor
--
-- DATE: April 11 2017
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- INTERFACE: AudioPlayer(): audio_pos(0), playing(false), fastForwarding(false), paused(false)
--                          audio_pos = place in the song
--                          playing = is a song currently being played
--                          fastForwading = is a song being fastForwarded
--                          paused = is the song paused
-- RETURNS: void. is a constructor
--
-- NOTES:
-- Audio player constructor
----------------------------------------------------------------------------------------------------------------------*/
AudioPlayer::AudioPlayer(): audio_pos(0), playing(false), fastForwarding(false), paused(false)
{

}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: openWavFile
--
-- DATE: April 11 2017
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- INTERFACE: openWavFile(const QString &fileName)
--                          &fileName = reference to the filepath of the wav file you want to open
-- RETURNS: bool - successful open
--
-- NOTES:
-- Opens a wav file to play (sourceFile is a WavFile that extends QFile, us QFile function to open)
----------------------------------------------------------------------------------------------------------------------*/
bool AudioPlayer::openWavFile(const QString &fileName){
    paused = false;
    playing = false;
    fastForwarding = false;
    if(!sourceFile.open(fileName))
        return false;
    audio_buffer.resize(0);
    audio_pos = 0;
    progress_current = 0;
    audio_buffer = sourceFile.readAll();
    progress_max = audio_buffer.size();
    open(QIODevice::ReadOnly);
    return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: resetPlayer
--
-- DATE: April 11 2017
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- INTERFACE: resetPlayer()
--
-- RETURNS: void
--
-- NOTES:
-- resets the audioplayer to clear the buffer its playing and set everything to its initial value/size
----------------------------------------------------------------------------------------------------------------------*/
void AudioPlayer::resetPlayer(){
    paused = false;
    playing = false;
    fastForwarding = false;
    audio_buffer.resize(0);
    audio_pos = 0;
    progress_current = 0;
    progress_max = 0;
    open(QIODevice::ReadOnly);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: readHeader
--
-- DATE: April 11 2017
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- INTERFACE: readHeader(const char *data, qint64 len)
--                      const char *data - pointer to the buffer that contains the header (44bytes) of the wav file
--                      qint64 len - length of the buffer
--
-- RETURNS: return bool - success on reading the header (WavFile.cpp)
--
-- NOTES:
-- reads the headerdata and sets the wav player settings based on the wav file header
----------------------------------------------------------------------------------------------------------------------*/
bool AudioPlayer::readHeader(char *data, qint64 len){
    return sourceFile.readHeader(data, len);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: fileFormat
--
-- DATE: April 11 2017
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- INTERFACE: fileFormat() const
--
-- RETURNS: return QAudioFormay - qt wave file format
--
-- NOTES:
-- updated the QAudio format for the player
-- sets the codec, samplerate, samplesize, and sample type decided from the header.
----------------------------------------------------------------------------------------------------------------------*/
const QAudioFormat& AudioPlayer::fileFormat() const{
    return sourceFile.fileFormat();
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: headerLength
--
-- DATE: April 11 2017
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- INTERFACE: headerLength()
--
-- RETURNS: return qint64 - the header legthn
--
-- NOTES:
-- returns header legnth of the wav file
----------------------------------------------------------------------------------------------------------------------*/
qint64 AudioPlayer::headerLength() const{
    return sourceFile.headerLength();
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: readHeaderData
--
-- DATE: April 11 2017
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- INTERFACE: readHeaderData()
--
-- RETURNS: QByteArray - successful on reading the header data
--
-- NOTES:
-- reads header data from the wav file using the wavfile.cpp function
----------------------------------------------------------------------------------------------------------------------*/
QByteArray AudioPlayer::readHeaderData(){
    sourceFile.seek(0);
    return sourceFile.read(sourceFile.headerLength());
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: readChunkData
--
-- DATE: April 11 2017
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- INTERFACE: readChunkData(qint64 len, qint64 pos)
--                          qint64 len - length to read
--                          qint64 pos - posistion to read from
--
-- RETURNS: QByteArray - if there is enough data to read, returns from playing the audio data chunk
--                     - otherwise it will return a null ptr as it is past the buffer limit
--
-- NOTES:
-- reads header data from the wav file using the wavfile.cpp function
----------------------------------------------------------------------------------------------------------------------*/
QByteArray AudioPlayer::readChunkData(qint64 len, qint64 pos){
    if(pos + len < audio_buffer.size()){
        return audio_buffer.mid(pos, len);
    }
    return nullptr;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: readData
--
-- DATE: April 11 2017
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- INTERFACE: readData(char *data, qint64 len)
--                      char *data - pointer to the buffer of data stored from the wav file
--                      qint64 len - maxsize of the buffer
--
-- RETURNS: qint64 - Reads up to maxSize bytes from the device into data, and returns the number of bytes read
--                      or -1 if an error occurred.
--
-- NOTES:
-- QT function to read data. If there are no bytes to be read and there can never be more bytes available
----------------------------------------------------------------------------------------------------------------------*/
qint64 AudioPlayer::readData(char *data, qint64 len){
    qint64 chunk = 0;
    if (playing && audio_pos < audio_buffer.size()) {
        chunk = qMin((audio_buffer.size() - audio_pos), len);
        memcpy(data, audio_buffer.constData() + audio_pos, chunk);

        emit streamChunkAudio(chunk, audio_pos);

        audio_pos += chunk;
        progress_current += chunk;
        emit progressAudio((progress_current/(double)progress_max) * 100);

        if(isFastForwarding())
            qDebug()<<"isFastForwarding";
        if(audio_pos >= audio_buffer.size()){
            playing = false;
        }
    }
    return chunk;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: writeData
--
-- DATE: April 11 2017
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- INTERFACE: writeData(const char *data, qint64 len)
--
-- RETURNS: qint64 - Reads up to maxSize bytes from the device into data, and returns the number of bytes read
--                      or -1 if an error occurred.
--
-- NOTES:
-- QT function to read data. If there are no bytes to be read and there can never be more bytes available
----------------------------------------------------------------------------------------------------------------------*/
qint64 AudioPlayer::writeData(const char *data, qint64 len){
    Q_UNUSED(data);
    Q_UNUSED(len);
    return 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: bytesAvailable
--
-- DATE: April 11 2017
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- INTERFACE: bytesAvailable()
--
-- RETURNS: qint64 - Returns the number of bytes that are available for reading
--
-- NOTES:
-- QT function used to determine the number of bytes to allocate in a buffer before reading
----------------------------------------------------------------------------------------------------------------------*/
qint64 AudioPlayer::bytesAvailable() const{
    return audio_buffer.size() - audio_pos;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: mypos
--
-- DATE: April 11 2017
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- INTERFACE: mypos()
--
-- RETURNS: qint64 - Returns the current position in the audio player
--
-- NOTES:
-- Returns the current position in the audio player
----------------------------------------------------------------------------------------------------------------------*/
qint64 AudioPlayer::mypos() const{
    return audio_pos;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: myseek
--
-- DATE: April 11 2017
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- INTERFACE: myseek(qint64 pos)
--              qint64 pos - seek to this position
--
-- RETURNS: bool true if seeks to the position
--
-- NOTES:
-- moves the current position of the audio
----------------------------------------------------------------------------------------------------------------------*/
bool AudioPlayer::myseek(qint64 pos){
    if(pos < 0 || pos > audio_buffer.size())
        return false;
    qDebug()<<"seek";
    audio_pos = pos;
    return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: pause
--
-- DATE: April 11 2017
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- INTERFACE: AudioPlayer::pause()
--
-- RETURNS: bool - return true
--
-- NOTES:
-- QT function - sets the bool playing to false and paused to true if it is currently playing
----------------------------------------------------------------------------------------------------------------------*/
bool AudioPlayer::pause(){
    if(playing) {
        paused = true;
        playing = false;
        //close();
    }
    return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: start
--
-- DATE: April 11 2017
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- INTERFACE: AudioPlayer::start()
--
-- RETURNS: bool - return true
--
-- NOTES:
-- QT function - sets the bool playing to false if it is currently paused
----------------------------------------------------------------------------------------------------------------------*/
bool AudioPlayer::start(){
    playing = true;
    if(paused)
        paused = false;
    //open(QIODevice::ReadOnly);
    return playing;
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: isPlaying
--
-- DATE: April 11 2017
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- INTERFACE: isPlaying()
--
-- RETURNS: bool - getter for playing
--
-- NOTES: returns playing
----------------------------------------------------------------------------------------------------------------------*/
bool AudioPlayer::isPlaying(){
    return playing;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: isFastForwarding
--
-- DATE: April 11 2017
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- INTERFACE: isFastForwarding()
--
-- RETURNS: bool - getter for fastForwarding
--
-- NOTES:
----------------------------------------------------------------------------------------------------------------------*/
bool AudioPlayer::isFastForwarding(){
    return fastForwarding;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: isPaused
--
-- DATE: April 11 2017
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- INTERFACE: isPaused()
--
-- RETURNS: bool - getter for paused
--
-- NOTES:
----------------------------------------------------------------------------------------------------------------------*/
bool AudioPlayer::isPaused(){
    return paused;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: isFastForwarding
--
-- DATE: April 11 2017
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- INTERFACE: isFastForwarding(bool forward)
--                              bool forward - sets the bool fastFowarding
--
-- RETURNS: bool - getter for paused
--
-- NOTES:
----------------------------------------------------------------------------------------------------------------------*/
bool AudioPlayer::isFastForwarding(bool forward){
    fastForwarding = forward;
    return fastForwarding;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: addChunkData
--
-- DATE: April 11 2017
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- INTERFACE: addChunkData(const char *data, qint64 len)
--                          const char *data - wav file chunk from char QByte array
--                          qint64 len - size of data
--
-- RETURNS: bool - true
--
-- NOTES:
-- appends new audio chunk to be played at the end of the buffer
----------------------------------------------------------------------------------------------------------------------*/
bool AudioPlayer::addChunkData(const char *data, qint64 len){
    audio_buffer.append(QByteArray::fromRawData(data,len));
    return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: audioBufferSize
--
-- DATE: April 11 2017
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- INTERFACE: audioBufferSize()
--
-- RETURNS: qint64 - getter for audio_buffer size
--
-- NOTES:
-- getter for audio buffer size
----------------------------------------------------------------------------------------------------------------------*/
qint64 AudioPlayer::audioBufferSize() const{
    return audio_buffer.size();
}
