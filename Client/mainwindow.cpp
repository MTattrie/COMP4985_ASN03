/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: mainWindow.cpp
--
-- PROGRAM: inotd
--
-- FUNCTIONS:
--    void playNextSong();
--    void initAudioOuput();
--    bool setAudioHeader(QAudioFormat format);
--    void updateHeader(char *data, qint64 len);
--    void addChunk(char *data, qint64 len);
--    void updateAvailSongs(char *);
--    void updatePlaylist(char *);
--    void updateProgressData(char *);
--    void addPlaylist(QString item);
--    void fastforward();
--    void writeFile();
--    void receivedSkipTrack();
--    void rewind();
--    void playpause();
--    void on_button_addSong_clicked();
--    void on_button_play_clicked();
--    void on_button_skip_clicked();
--    void on_button_download_clicked();
--    void on_button_FastForward_clicked();
--    void on_button_rewind_clicked();
--    void handleReceivedCommand(int command, char *data, int len);
--    void setVolume(int value);
--    void setProgress(int value);
--    void connectToServer();
--    void on_button_connectToClient_clicked();
--    void handleReceivedRecoredData(qint64 len);
--    void handleReceivedPeerData(char *data, int len);
--    void on_button_upload_clicked();
--    void requestSong(QString song);
--    void sendSong(QString song);
--    void findAvailableSongs();
--    void decodeMessage(QString message);
--
-- DATE: April 11, 2017
--
-- DESIGNER: Mark Tattrie, Deric Mccadden, Terry Kang, Jacob Frank
--
-- PROGRAMMER:
--
-- NOTES:
-- This class handles the user input and connection initializations for connecting to the client and server.
-- Once connected to the server or client, class handles incoming requests and calls the applicable function
-- to complete the request.
----------------------------------------------------------------------------------------------------------------------*/
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <fstream>
#include <ostream>
#include <QFileDialog>

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: MainWindow Ctor
--
-- DATE: April 11, 2017
--
-- DESIGNER: Mark Tattrie, Deric Mccadden, Terry Kang, Jacob Frank
--
-- PROGRAMMER:
--
-- INTERFACE: MainWindow(QWidget *parent)
--                  QWidget *parent: The main window
--
-- RETURNS: void.
--
-- NOTES:
--  Initializes the main window and sets up default UI behaviour
----------------------------------------------------------------------------------------------------------------------*/
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    available_song_model = new QStringListModel(this);
    playlist_model = new QStringListModel(this);

    // Glue model and view together
    ui->listView_availSongs->setModel(available_song_model);
    ui->listView_playlist->setModel(playlist_model);

    QObject::connect(this, SIGNAL( requestSong(QString) ), &client, SLOT( requestSong(QString) ));
    QObject::connect(this, SIGNAL( sendSong(QString) ), &client, SLOT(sendSong(QString)));
    QObject::connect(&client, SIGNAL( receivedCommand(int,char*,int)), this, SLOT(handleReceivedCommand(int,char*,int)));
    QObject::connect(&client, SIGNAL( receivedPeerData(char*,int)), this, SLOT(handleReceivedPeerData(char*,int)));

    initAudioOuput();

    m_pPalette	= new QPalette();
    m_pPixmap = new QPixmap("../assets/ui/background.jpg");

    QPalette* palette = new QPalette();
    palette->setBrush(QPalette::Background,*(new QBrush(*(m_pPixmap))));
    setPalette(*palette);

    connect(ui->volumeSlider,SIGNAL(valueChanged(int)),this,SLOT(setVolume(int)));
    connect(ui->button_connectToServer,SIGNAL(pressed()),this,SLOT(connectToServer()));


    ui->lineEdit_serverHostname->setText("localhost");
    ui->lineEdit_serverPortNumber->setText("7000");
    ui->client_IP->setText("127.0.0.1");
    ui->client_Port->setText("9000");
    audio_volume = 99;

}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: MainWindow Dtor
--
-- DATE: April 11, 2017
--
-- DESIGNER: Mark Tattrie, Deric Mccadden, Terry Kang, Jacob Frank
--
-- PROGRAMMER:
--
-- INTERFACE: ~MainWindow()
--
-- NOTES:
-- Deletes the UI when the application is exitted
----------------------------------------------------------------------------------------------------------------------*/
MainWindow::~MainWindow()
{
    delete ui;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: on_button_addSong_clicked
--
-- DATE: April 11, 2017
--
-- DESIGNER: Mark Tattrie, Deric Mccadden, Terry Kang, Jacob Frank
--
-- PROGRAMMER:
--
-- INTERFACE: void MainWindow::on_button_addSong_clicked()
--
-- RETURNS: void.
--
-- NOTES:
-- Sends an "add song to playlist" request to the server over TCP
-- Reads in the song title at the selected index and writes the command and title to the server
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_button_addSong_clicked()
{
    QModelIndex index = ui->listView_availSongs->currentIndex();
    if(index.row() < 0)
        return;

    QString itemText = index.data(Qt::DisplayRole).toString();
    client.reqeustCommand(ADDLIST, itemText);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: on_button_play_clicked
--
-- DATE: April 11, 2017
--
-- DESIGNER: Mark Tattrie, Deric Mccadden, Terry Kang, Jacob Frank
--
-- PROGRAMMER:
--
-- INTERFACE: void MainWindow::on_button_play_clicked()
--
-- RETURNS: void.
--
-- NOTES:
-- Sends a "Play/Pause" request to the server over TCP
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_button_play_clicked()
{
    qDebug()<<"on_button_play_clicked";
    client.reqeustCommand(PLAYPAUSE);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: on_button_skip_clicked
--
-- DATE: April 11, 2017
--
-- DESIGNER: Mark Tattrie, Deric Mccadden, Terry Kang, Jacob Frank
--
-- PROGRAMMER:
--
-- INTERFACE: void MainWindow::on_button_skip_clicked()
--
-- RETURNS: void.
--
-- NOTES:
-- Sends a "Skip track" request to the server over TCP then
-- stops the currently playing song and removes it from the current playlist
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_button_skip_clicked()
{
    if(audioPlayer->isPlaying()){
        client.reqeustCommand(SKIPTRACK);
        isSetHeader = false;
        audio->stop();
        audioPlayer->pause();
        audio->reset();
        audioPlayer->resetPlayer();
        playlist_model->removeRow(0);
    }
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: on_button_FastForward_clicked
--
-- DATE: April 11, 2017
--
-- DESIGNER: Mark Tattrie, Deric Mccadden, Terry Kang, Jacob Frank
--
-- PROGRAMMER:
--
-- INTERFACE: void MainWindow::on_button_FastForward_clicked
--
-- RETURNS: void.
--
-- NOTES:
-- Sends a "Fast Forward" request to the server over TCP
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_button_FastForward_clicked()
{
    client.reqeustCommand(FASTFORWORD);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: on_button_rewind_clicked
--
-- DATE: April 11, 2017
--
-- DESIGNER: Mark Tattrie, Deric Mccadden, Terry Kang, Jacob Frank
--
-- PROGRAMMER:
--
-- INTERFACE:
--
-- RETURNS: void.
--
-- NOTES:
-- Sends a "Rewind" request to the server over TCP
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_button_rewind_clicked()
{
    client.reqeustCommand(REWIND);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: initAudioOuput
--
-- DATE: April 11, 2017
--
-- DESIGNER: Mark Tattrie, Deric Mccadden, Terry Kang, Jacob Frank
--
-- PROGRAMMER:
--
-- INTERFACE: void MainWindow::initAudioOuput()
--
-- RETURNS: void.
--
-- NOTES:
-- Function initializes all audio recording and playing devices to their initial values
-- This enables them to be used by other functions
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::initAudioOuput(){
    audio = new QAudioOutput();
    audioPlayer = new AudioPlayer();
    microphonePlayer = new MicrophonePlayer();
    mic_in = nullptr;
    mic_out = nullptr;
    isSetHeader = false;
    QObject::connect(audioPlayer, SIGNAL( progressAudio(int)), this, SLOT(setProgress(int)));
    QObject::connect(microphonePlayer, SIGNAL( recorded(qint64)), this, SLOT(handleReceivedRecoredData(qint64)));

    mic_format.setSampleRate(22050);
    mic_format.setChannelCount(1);
    mic_format.setSampleSize(16);
    mic_format.setCodec("audio/pcm");
    mic_format.setByteOrder(QAudioFormat::LittleEndian);
    mic_format.setSampleType(QAudioFormat::SignedInt);


}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: setAudioHeader
--
-- DATE: April 11, 2017
--
-- DESIGNER: Mark Tattrie, Deric Mccadden, Terry Kang, Jacob Frank
--
-- PROGRAMMER:
--
-- INTERFACE: bool MainWindow::setAudioHeader(QAudioFormat format)
--
-- RETURNS: bool.
--
-- NOTES:
-- Function configures the Audio output device to match the audio format of the currently playing song
----------------------------------------------------------------------------------------------------------------------*/
bool MainWindow::setAudioHeader(QAudioFormat format){
    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    if (!info.isFormatSupported(format)) {
        qWarning() << "Raw audio format not supported by backend, cannot play audio.";
        return false;
    }
    audio->stop();
    audioPlayer->pause();
    audio->reset();
    delete audio;
    audio = new QAudioOutput(format, this);
    return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: on_button_download_clicked
--
-- DATE: April 11, 2017
--
-- DESIGNER: Mark Tattrie, Deric Mccadden, Terry Kang, Jacob Frank
--
-- PROGRAMMER:
--
-- INTERFACE: void MainWindow::on_button_download_clicked()
--
-- RETURNS: void.
--
-- NOTES:
-- Sends a "Download" request to the server over TCP for the currently selected song
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_button_download_clicked()
{
    QModelIndex index = ui->listView_availSongs->currentIndex();
    if(index.row() < 0)
        return;
    QString itemText = index.data(Qt::DisplayRole).toString();
    emit requestSong(itemText);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: updateHeader
--
-- DATE: April 11, 2017
--
-- DESIGNER: Mark Tattrie, Deric Mccadden, Terry Kang, Jacob Frank
--
-- PROGRAMMER:
--
-- INTERFACE: void MainWindow::updateHeader(char *data, qint64 len)
--                              char *data: Header data
--                              qint64 len: length of the header
--
-- RETURNS: void.
--
-- NOTES:
-- Sets the audio file header for reading audio data
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::updateHeader(char *data, qint64 len)
{
    if(audioPlayer->readHeader(data, len)){
        if(!setAudioHeader(audioPlayer->fileFormat()))
            return;
        audioPlayer->resetPlayer();
        if(isSetHeader){
           // playlist_model->removeRow(0);
        }
        isSetHeader = true;
        qDebug()<<"size : " << data;
    }
    else{
        isSetHeader = false;
    }
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: addChunk(char *data, qint64 len)
--
-- DATE: April 11, 2017
--
-- DESIGNER: Mark Tattrie, Deric Mccadden, Terry Kang, Jacob Frank
--
-- PROGRAMMER:
--
-- INTERFACE: void MainWindow::addChunk(char *data, qint64 len)
--                              char *data: Chunk of audio data
--                              qint64 len: length of the data
--
-- RETURNS: void.
--
-- NOTES:
-- Adds a chunk of audio data to the audio player's buffer for playing
-- Updates the UI to display whether a song is playing or has been paused
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::addChunk(char *data, qint64 len){
    audioPlayer->addChunkData(data, len);

    if(!audioPlayer->isPlaying()
            && !audioPlayer->isPaused()
            && audioPlayer->bytesAvailable() >= audioPlayer->fileFormat().bytesForDuration(1000000)){
        audioPlayer->start();
        audio->start(audioPlayer);
        setVolume(audio_volume);
        ui->button_play->setStyleSheet(QString("QPushButton {border-image:url(../assets/ui/pause);}"));
    }else if(audioPlayer->isPaused()
             && audioPlayer->bytesAvailable() >= audioPlayer->fileFormat().bytesForDuration(1000000)){
        audioPlayer->start();
        audio->resume();
        ui->button_play->setStyleSheet(QString("QPushButton {border-image:url(../assets/ui/pause);}"));
    }
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: updateAvailSongs(char *list)
--
-- DATE: April 11, 2017
--
-- DESIGNER: Mark Tattrie, Deric Mccadden, Terry Kang, Jacob Frank
--
-- PROGRAMMER:
--
-- INTERFACE: void MainWindow::updateAvailSongs(char *list)
--                              char *list: List of available song titles
--
-- RETURNS: void.
--
-- NOTES:
-- Retrieves a list of available songs from the surver and updates the list in the UI
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::updateAvailSongs(char *list){
    QStringList stringList = QString(list).split('/');
    stringList.removeLast();
    available_song_model->setStringList(stringList);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: updatePlaylist(char *list)
--
-- DATE: April 11, 2017
--
-- DESIGNER: Mark Tattrie, Deric Mccadden, Terry Kang, Jacob Frank
--
-- PROGRAMMER:
--
-- INTERFACE: void MainWindow::updatePlaylist(char *list)
--                              char *list: List of songs in the playlist
--
-- RETURNS: void.
--
-- NOTES:
-- Retrieves a list of songs in the playlist from the surver and updates the list in the UI
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::updatePlaylist(char *list){
    QStringList stringList = QString(list).split('/');
    stringList.removeLast();
    playlist_model->setStringList(stringList);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: addPlaylist(QString item)
--
-- DATE: April 11, 2017
--
-- DESIGNER: Mark Tattrie, Deric Mccadden, Terry Kang, Jacob Frank
--
-- PROGRAMMER:
--
-- INTERFACE: void MainWindow::addPlaylist(QString item)
--
-- RETURNS: void.
--
-- NOTES:
-- Retrieves a song from the surver and updates the playlist in the UI
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::addPlaylist(QString item){
    playlist_model->insertRow(playlist_model->rowCount());
    QModelIndex row = playlist_model->index(playlist_model->rowCount()-1);
    playlist_model->setData(row, item);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: updateProgressData(char *progressData)
--
-- DATE: April 11, 2017
--
-- DESIGNER: Mark Tattrie, Deric Mccadden, Terry Kang, Jacob Frank
--
-- PROGRAMMER:
--
-- INTERFACE: void MainWindow::updateProgressData(char *progressData)
--                            char *progressData: The distance along the progress bar, the song is
--
-- RETURNS: void.
--
-- NOTES:
-- Function updates the progress bar to display how far into the song, the player is
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::updateProgressData(char *progressData){
    QStringList stringList = QString(progressData).split(',');
    qDebug()<<stringList;
    audioPlayer->setProgressData(stringList.at(0).toInt(), stringList.at(1).toInt());
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: connectToServer()
--
-- DATE: April 11, 2017
--
-- DESIGNER: Mark Tattrie, Deric Mccadden, Terry Kang, Jacob Frank
--
-- PROGRAMMER:
--
-- INTERFACE: void MainWindow::connectToServer()
--
-- RETURNS: void.
--
-- NOTES:
-- Reads the host name and port number form the UI and connects to the server
----------------------------------------------------------------------------------------------------------------------*/

void MainWindow::connectToServer() {
    QString hostname = ui->lineEdit_serverHostname->text();
    QString portNumber = ui->lineEdit_serverPortNumber->text();
    std::thread(&Client::start, &client, hostname, portNumber).detach();
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: setVolume(int value)
--
-- DATE: April 11, 2017
--
-- DESIGNER: Mark Tattrie, Deric Mccadden, Terry Kang, Jacob Frank
--
-- PROGRAMMER:
--
-- INTERFACE: void MainWindow::setVolume(int value)
--                      int value: The volume level to be set (0 - 100)
--
-- RETURNS: void.
--
-- NOTES:
-- Updates the audio output player to the volume specified
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::setVolume(int value)
{
    audio_volume = value;
    audio->setVolume((float)value / 100);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: setProgress(int value)
--
-- DATE: April 11, 2017
--
-- DESIGNER: Mark Tattrie, Deric Mccadden, Terry Kang, Jacob Frank
--
-- PROGRAMMER:
--
-- INTERFACE: void MainWindow::setProgress(int value)
--                                  int value: distance along the progress bar, based on how much song has played
--
-- RETURNS: void.
--
-- NOTES:
-- Sets the progress bar to the correct distance to reflect the amount of song that has played
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::setProgress(int value){
    ui->ProgressSlider->setValue(value);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: fastforward()
--
-- DATE: April 11, 2017
--
-- DESIGNER: Mark Tattrie, Deric Mccadden, Terry Kang, Jacob Frank
--
-- PROGRAMMER:
--
-- INTERFACE: void MainWindow::fastforward()
--
-- RETURNS: void.
--
-- NOTES:
-- Increases the samplerate to play the audio file more quickly
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::fastforward(){
    if(!audioPlayer->isPlaying())
        return;

    QAudioFormat format = audioPlayer->fileFormat();
    if(audioPlayer->isFastForwarding()){
        audioPlayer->isFastForwarding(false);
    }else {
        format.setSampleRate(format.sampleRate() * 1.5);
        audioPlayer->isFastForwarding(true);
    }

    delete audio;
    audio = new QAudioOutput(format, this);
    audio->start(audioPlayer);
    setVolume(audio_volume);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: receivedSkipTrack()
--
-- DATE: April 11, 2017
--
-- DESIGNER: Mark Tattrie, Deric Mccadden, Terry Kang, Jacob Frank
--
-- PROGRAMMER:
--
-- INTERFACE: void MainWindow::receivedSkipTrack()
--
-- RETURNS: void.
--
-- NOTES:
-- Stops the currently playing song and resets the audio player
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::receivedSkipTrack(){
    qDebug()<<"receivedSkipTrack";
    audio->stop();
    audioPlayer->pause();
    audio->reset();
    audioPlayer->resetPlayer();
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: rewind()
--
-- DATE: April 11, 2017
--
-- DESIGNER: Mark Tattrie, Deric Mccadden, Terry Kang, Jacob Frank
--
-- PROGRAMMER:
--
-- INTERFACE: void MainWindow::rewind()
--
-- RETURNS: void.
--
-- NOTES:
-- Stops the currently playing song and resets the audio player
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::rewind(){
    audio->stop();
    audioPlayer->pause();
    audioPlayer->resetPlayer();
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: playpause()
--
-- DATE: April 11, 2017
--
-- DESIGNER: Mark Tattrie, Deric Mccadden, Terry Kang, Jacob Frank
--
-- PROGRAMMER:
--
-- INTERFACE: void MainWindow::playpause()
--
-- RETURNS: void.
--
-- NOTES:
-- Suspends the audioplayer and updates the UI to display whether the song is paused or playing
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::playpause(){
    if(audioPlayer->isPlaying()){
        audio->suspend();
        audioPlayer->pause();
        ui->button_play->setStyleSheet(QString("QPushButton {border-image:url(../assets/ui/play);}"));
    }else if(audioPlayer->isPaused()){
        audio->resume();
        audioPlayer->start();
        ui->button_play->setStyleSheet(QString("QPushButton {border-image:url(../assets/ui/pause);}"));
    }
}
int count = 0;

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: handleReceivedCommand(int command, char *data, int len)
--
-- DATE: April 11, 2017
--
-- DESIGNER: Mark Tattrie, Deric Mccadden, Terry Kang, Jacob Frank
--
-- PROGRAMMER:
--
-- INTERFACE: void MainWindow::handleReceivedCommand(int command, char *data, int len)
--                                  int command: The command to handle
--                                  char *data: Data for the command
--                                  int len: Length of the data
--
-- RETURNS: void.
--
-- NOTES:
-- Functions handles decoding packets from the server to call the correct functions
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::handleReceivedCommand(int command, char *data, int len){
    switch(command){
        case UPLOAD:
            break;
        case DOWNLOAD:{
            client.downloads.append(QByteArray(data, len));
            break;
        }
        case COMPLETE:{
            std::thread(&MainWindow::writeFile, this).detach();
            break;
        }
        case ADDLIST:
            addPlaylist(QString(data));
            break;
        case PLAYPAUSE:
            playpause();
            break;
        case FASTFORWORD:
            fastforward();
            break;
        case REWIND:
            rewind();
            updateProgressData(data);
            break;
        case SKIPTRACK:
            receivedSkipTrack();
            break;
        case STREAM:
            addChunk(data, len);
            break;
        case HEADER:
            updateHeader(data, len);
            break;
        case PROGRESS:
            updateProgressData(data);
            break;
        case AVAILSONG:
            updateAvailSongs(data);
            break;
        case PLAYLIST:
            updatePlaylist(data);
            break;
    }
    delete data;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: writeFile()
--
-- DATE: April 11, 2017
--
-- DESIGNER: Mark Tattrie, Deric Mccadden, Terry Kang, Jacob Frank
--
-- PROGRAMMER:
--
-- INTERFACE: void MainWindow::writeFile()
--
-- RETURNS: void.
--
-- NOTES:
-- Receives an audio file from the server and writes it to a file
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::writeFile(){
    const QString filename(client.filenames);
    QFile file(filename);
    if(file.open(QIODevice::WriteOnly)){
        file.write(client.downloads, client.downloads.size());
        file.close();
    }

    client.filenames.clear();
    client.downloads.clear();
    client.isDonwloading=false;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: on_button_connectToClient_clicked()
--
-- DATE: April 11, 2017
--
-- DESIGNER: Mark Tattrie, Deric Mccadden, Terry Kang, Jacob Frank
--
-- PROGRAMMER:
--
-- INTERFACE: void MainWindow::on_button_connectToClient_clicked()
--
-- RETURNS: void.
--
-- NOTES:
-- Reads in a client's IP Address and port number
-- Connects to the specified client and configures the microphone for communication
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_button_connectToClient_clicked()
{
    if(!microphonePlayer->isPlaying()){
        QString ip = ui->client_IP->text();
        QString port = ui->client_Port->text();

        if(ip.size()==0 || port.size() == 0 )
            return;

        if(!client.startPeerUDP(ip, port)){
            qDebug()<<"setPeerUDP Fail";
            return;
        }

        QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
        if (!info.isFormatSupported(mic_format)) {
            qWarning()<<"default format not supported try to use nearest";
            mic_format = info.nearestFormat(mic_format);
            //client.peerUDPRunning = false;
            //return;
        }
        qDebug()<<"Format";
        mic_out  = new QAudioOutput(mic_format, this);
        mic_in = new QAudioInput(mic_format, this);
        mic_out->start(microphonePlayer);
        mic_in->start(microphonePlayer);
        setVolume(audio_volume);
        microphonePlayer->start();
    }else{
        mic_out->stop();
        mic_in->stop();
        microphonePlayer->stop();
        //microphonePlayer->resetPlayer();
        delete mic_out;
        delete mic_in;
        mic_out = nullptr;
        mic_in = nullptr;
        client.peerUDPRunning = false;
    }

}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: handleReceivedRecoredData(qint64 len)
--
-- DATE: April 11, 2017
--
-- DESIGNER: Mark Tattrie, Deric Mccadden, Terry Kang, Jacob Frank
--
-- PROGRAMMER:
--
-- INTERFACE: void MainWindow::handleReceivedRecoredData(qint64 len)
--                                  qint64 len: Length of the received recorded data
--
-- RETURNS: void.
--
-- NOTES:
-- Receives audio data from a connected client's microphone and adds the chunck of data to the audio stream
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::handleReceivedRecoredData(qint64 len){
    //qDebug()<<"handleReceivedRecoredData";
    client.addMicStream(microphonePlayer->readChunkData(len));
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: handleReceivedPeerData(char *data, int len)
--
-- DATE: April 11, 2017
--
-- DESIGNER: Mark Tattrie, Deric Mccadden, Terry Kang, Jacob Frank
--
-- PROGRAMMER:
--
-- INTERFACE: void MainWindow::handleReceivedPeerData(char *data, int len)
--                                      char *data: audio data
--                                      int len: length of the audio data
--
-- RETURNS: void.
--
-- NOTES:
-- Reads audio data and adds it to the microphone player
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::handleReceivedPeerData(char *data, int len){
    microphonePlayer->addChunkData(data, len);
    if(client.peerUDPRunning && mic_out->state() == QAudio::IdleState ){
        mic_out->stop();
        mic_out->start(microphonePlayer);
    }
    delete data;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: on_button_upload_clicked
--
-- DATE: April 11, 2017
--
-- DESIGNER: Mark Tattrie, Deric Mccadden, Terry Kang, Jacob Frank
--
-- PROGRAMMER:
--
-- INTERFACE: void MainWindow::on_button_upload_clicked()
--
-- RETURNS: void.
--
-- NOTES:
-- Opens a dialog box to select a song to transfer to the server.
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_button_upload_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Wav"), "", tr("Audio Files (*.wav)"));
    emit sendSong(fileName);
}
