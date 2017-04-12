/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: microphoneplayer.cpp - gets microphone input and output
--
-- PROGRAM: Client (ComAudio - Final Project)
--
-- FUNCTIONS:
--  MicrophonePlayer();
--  bool stop();
--  bool start();
--  bool isPlaying();
--  QByteArray readChunkData(qint64);
--  bool addChunkData(const char *, qint64);
--  void resetPlayer();
--  qint64 bytesAvailable() const;
--
-- DATE: April 11 2017
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER: Terry Kang, Mark Tattrie
--
-- NOTES:
-- takes care of the microphoneplayer, seperated from audio player to allow both to play at the same time as well as
-- make it more clear by being seperate.
----------------------------------------------------------------------------------------------------------------------*/
#include "microphoneplayer.h"

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: MicrophonePlayer()
--
-- DATE: April 10, 2017
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER: Terry Kang, Mark Tattrie
--
-- INTERFACE: MicrophonePlayer
--
-- RETURNS: is a constructor
--
-- NOTES:
-- MicrophonePlayer constructor
----------------------------------------------------------------------------------------------------------------------*/
MicrophonePlayer::MicrophonePlayer(): buffer_in(0), buffer_out(0), playing(false)
{
    playing = false;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: MicrophonePlayer()
--
-- DATE: April 10, 2017
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER: Terry Kang, Mark Tattrie
--
-- INTERFACE: resetPlayer()
--
-- RETURNS: void
--
-- NOTES:
-- resets the media player for the microphone
----------------------------------------------------------------------------------------------------------------------*/
void MicrophonePlayer::resetPlayer(){
    playing = false;
    buffer_in.clear();
    buffer_out.clear();
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: readChunkData
--
-- DATE: April 11 2017
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER: Terry Kang, Mark Tattrie
--
-- INTERFACE: readChunkData(qint64 len)
--                          qint64 len - length to read
--
-- RETURNS: QByteArray - if there is enough data to read, returns from playing the audio data chunk
--                     - otherwise it will return a null ptr as it is past the buffer limit
--
-- NOTES:
-- reads header data from the wav file using the wavfile.cpp function
----------------------------------------------------------------------------------------------------------------------*/
QByteArray MicrophonePlayer::readChunkData(qint64 len){
    qint64 chunk = 0;
    chunk = qMin((qint64)buffer_in.size(), len);
    QByteArray data(buffer_in.mid(0, chunk));
    buffer_in.remove(0, chunk);
    return data;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: readData
--
-- DATE: April 11 2017
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER: Terry Kang, Mark Tattrie
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
qint64 MicrophonePlayer::readData(char *data, qint64 len){
    qDebug()<<"readData : " << buffer_out.size();
    qint64 chunk = 0;
    chunk = qMin((qint64)buffer_out.size(), len);
    memcpy(data, buffer_out.constData(), chunk);
    buffer_out.remove(0, chunk);
    return chunk;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: writeData
--
-- DATE: April 11 2017
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER: Terry Kang, Mark Tattrie
--
-- INTERFACE: writeData(const char *data, qint64 len)
--
-- RETURNS: qint64 - Reads up to maxSize bytes from the device into data, and returns the number of bytes read
--                      or -1 if an error occurred.
--
-- NOTES:
-- QT function to read data. If there are no bytes to be read and there can never be more bytes available
----------------------------------------------------------------------------------------------------------------------*/
qint64 MicrophonePlayer::writeData(const char *data, qint64 len){
    buffer_in.append(QByteArray::fromRawData(data,len));
    emit recorded(len);
    return len;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: bytesAvailable
--
-- DATE: April 11 2017
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER: Terry Kang, Mark Tattrie
--
-- INTERFACE: bytesAvailable()
--
-- RETURNS: qint64 - returns the buffer size
--
-- NOTES:
-- returns the available bytes
----------------------------------------------------------------------------------------------------------------------*/
qint64 MicrophonePlayer::bytesAvailable() const{
    return buffer_out.size();
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: stop
--
-- DATE: April 11 2017
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER: Terry Kang, Mark Tattrie
--
-- INTERFACE: AudioPlayer::stop()
--
-- RETURNS: bool - return true
--
-- NOTES:
-- QT function - sets the bool playing to false, closes the microphone session
----------------------------------------------------------------------------------------------------------------------*/
bool MicrophonePlayer::stop(){
    playing = false;
    close();
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
-- PROGRAMMER: Terry Kang, Mark Tattrie
--
-- INTERFACE: AudioPlayer::start()
--
-- RETURNS: bool - return true
--
-- NOTES:
-- QT function - sets the bool playing to true, opens the microphone to record into
----------------------------------------------------------------------------------------------------------------------*/
bool MicrophonePlayer::start(){
    playing = true;
    open(QIODevice::ReadWrite);
    return playing;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: isPlaying
--
-- DATE: April 11 2017
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER: Terry Kang, Mark Tattrie
--
-- INTERFACE: isPlaying()
--
-- RETURNS: bool - getter for isPlaying
--
-- NOTES:
----------------------------------------------------------------------------------------------------------------------*/
bool MicrophonePlayer::isPlaying(){
    return playing;
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
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER: Terry Kang, Mark Tattrie
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
bool MicrophonePlayer::addChunkData(const char *data, qint64 len){
    buffer_out.append(QByteArray::fromRawData(data,len));
    qDebug()<<"addChunkData " << buffer_out.size();

    return true;
}
