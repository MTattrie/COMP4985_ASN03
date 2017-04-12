/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: mainWindow.cpp
--
-- PROGRAM: inotd
--
-- FUNCTIONS:
--  MainWindow(QWidget *parent = 0);
--  ~MainWindow();
--  void findAvailableSongs();
--  void resizeEvent (QResizeEvent* event);
--  void on_addPlaylistBTN_clicked();
--  void on_skipBTN_clicked();
--  void on_rewindBTN_clicked();
--  void on_ffBTN_clicked();
--  void on_playBTN_clicked();
--  void on_serverStartBTN_clicked();
--  void on_uploadBTN_clicked();
--  void handleStateChanged(QAudio::State newState);
--  void handleChunkStream(qint64 len, qint64 pos);
--  void handleReceivedCommand(int command);
--  void handleNewClient(int client_num);
--  void handleDisconnetClient(QString);
--  void handleReceivedAddPlaylist(QString);
--  void setProgress(int value);
--  void setVolume(int value);
--  void updateAvaliableSongs();
--  void playNextSong();
--  void initAudioOuput();
--  bool setAudioHeader(QAudioFormat format);
--  void sendHeader();
--  void removePlaylist();
--  void addPlaylist(QString &item);
--
-- DATE: April 11, 2017
--
-- DESIGNER: Mark Tattrie, Deric Mccadden, Terry Kang, Jacob Frank
--
-- PROGRAMMER: Mark Tattrie, Deric Mccadden, Terry Kang, Jacob Frank
--
-- NOTES:
-- This class handles the user input and connection initializations for connecting to the client and server.
-- Once connected to the server or client, class handles incoming requests and calls the applicable function
-- to complete the request.
----------------------------------------------------------------------------------------------------------------------*/
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: MainWindow
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: MainWindow(QWidget *parent)
--                  @parent : parent class
--
-- RETURNS: N/A.
--
-- NOTES:
--  Constructor of MainWindow. Intializes UI windows.
----------------------------------------------------------------------------------------------------------------------*/
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Server - ComAudio");

    m_pPalette	= new QPalette();
    m_pPixmap = new QPixmap("../assets/ui/background.jpg");

    m_pPalette->setBrush(QPalette::Background,*(new QBrush(*(m_pPixmap))));
    setPalette(*m_pPalette);

    // Create model
    available_song_model = new QStringListModel(this);
    playlist_model = new QStringListModel(this);
    clientlist_model = new QStringListModel(this);

    // Glue model and view together
    ui->listView_availSongs->setModel(available_song_model);
    ui->listView_playlist->setModel(playlist_model);
    ui->listView_clientlist->setModel(clientlist_model);

    ui->serverPort->setText("7000");


    findAvailableSongs();
    initAudioOuput();

    connect(ui->volumeSlider,SIGNAL(valueChanged(int)),this,SLOT(setVolume(int)));
    audio_volume = 0;
    ui->volumeSlider->setValue(0);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: ~MainWindow
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: ~MainWindow()
--
-- RETURNS: N/A.
--
-- NOTES:
--  Destructor of MainWindow. Destorys all pointers
----------------------------------------------------------------------------------------------------------------------*/
MainWindow::~MainWindow()
{
    delete ui;
    delete audio;
    delete audioPlayer;
    delete available_song_model;
    delete playlist_model;
    delete clientlist_model;
    delete m_pPixmap;
    delete m_pPalette;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: findAvailableSongs
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: void findAvailableSongs()
--
-- RETURNS: void
--
-- NOTES:
--  Find all of wav files in the specific directory and populate the list view with it.
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::findAvailableSongs(){
    QStringList nameFilter("*.wav");
    QDir directory("../assets/musics");
    QStringList wavFilesList = directory.entryList(nameFilter);

    // Populate the model
    available_song_model->setStringList(wavFilesList);
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: initAudioOuput
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: void initAudioOuput()
--
-- RETURNS: void
--
-- NOTES:
--  Initialize a audio output device and a audio player (audio buffer).
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::initAudioOuput(){
    audio = new QAudioOutput();
    audioPlayer = new AudioPlayer();
    connect(audioPlayer, SIGNAL(streamChunkAudio(qint64,qint64)), this, SLOT(handleChunkStream(qint64,qint64)));
    connect(&server, SIGNAL(receivedCommand(int)), this, SLOT(handleReceivedCommand(int)));
    connect(&server, SIGNAL(newClientConnected(int)), this, SLOT(handleNewClient(int)));
    connect(&server, SIGNAL(clientDisconnected(QString)), this, SLOT(handleDisconnetClient(QString)));
    connect(&server, SIGNAL( receivedAddPlaylist(QString)), this, SLOT(handleReceivedAddPlaylist(QString)));
    connect(audioPlayer, SIGNAL( progressAudio(int)), this, SLOT(setProgress(int)));
    connect(&server, SIGNAL(updateAvaliableSongs()), this, SLOT(updateAvaliableSongs()));


}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: setAudioHeader
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: bool setAudioHeader(QAudioFormat format)
--                  @format : format of audio file(sampling rate, channel, etc..)
--
-- RETURNS: true if success in changing the format
--
-- NOTES:
--  Change the audio output device with the passed audio format.
----------------------------------------------------------------------------------------------------------------------*/
bool MainWindow::setAudioHeader(QAudioFormat format){
    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    if (!info.isFormatSupported(format)) {
        qWarning() << "Raw audio format not supported by backend, cannot play audio.";
        return false;
    }
    delete audio;
    audio = new QAudioOutput(format, this);
    connect(audio, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));
    sendHeader();
    return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: playNextSong
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: void playNextSong()
--
-- RETURNS: void
--
-- NOTES:
--  Play next song in the playlist if exists after reading a audio file and changing the format of the audio device
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::playNextSong(){
    if(playlist_model->stringList().length() <= 0)
        return;

    if(!audioPlayer->openWavFile("../assets/musics/" + playlist_model->stringList().at(0)))
        return;

    if(!setAudioHeader(audioPlayer->fileFormat()))
        return;
    audio->start(audioPlayer);
    audioPlayer->start();
    setVolume(audio_volume);
    songFinished = false;
    ui->playBTN->setStyleSheet(QString("QPushButton {border-image:url(../assets/ui/pause);}"));
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: on_addPlaylistBTN_clicked
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: void on_addPlaylistBTN_clicked()
--
-- RETURNS: void
--
-- NOTES:
--  Called when user clicks a buttion to add a song to the playlist from the list of available songs.
--  Adds a selected song in the list view of available songs to the list view of playlist
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_addPlaylistBTN_clicked()
{
    QModelIndex index = ui->listView_availSongs->currentIndex();
    if(index.row() < 0)
        return;

    QString itemText = index.data(Qt::DisplayRole).toString();

    addPlaylist(itemText);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: on_addPlaylistBTN_clicked
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: void on_addPlaylistBTN_clicked()
--
-- RETURNS: void
--
-- NOTES:
--  Called when user clicks a buttion to skip the current playing song
--  If a song is playing, stop and skip the song and play the next song in the playlist.
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_skipBTN_clicked()
{
    if(audioPlayer->isPlaying()){
        ui->playBTN->setStyleSheet(QString("QPushButton {border-image:url(../assets/ui/play);}"));
        audio->stop();
        audioPlayer->resetPlayer();
        removePlaylist();
        playNextSong();
    }
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: on_rewindBTN_clicked
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: void on_rewindBTN_clicked()
--
-- RETURNS: void
--
-- NOTES:
--  Called when user clicks a buttion to rewind the current playing song.
--  If a song is playing, stop and move the position of the audio buffer to 5 seconds before.
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_rewindBTN_clicked()
{
    audioPlayer->pause();
    int samplebytes = audioPlayer->fileFormat().bytesForDuration(5000000);
    qint64 pos = audioPlayer->pos() - samplebytes;

    if(pos<=0)
        pos = 0;
    audioPlayer->myseek(pos);
    QString progress = QString::number(pos) + "," +  QString::number(audioPlayer->audioBufferSize())  + ",";
    server.TCPBroadCast(REWIND, QByteArray(progress.toStdString().c_str()));
    audioPlayer->start();
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: on_ffBTN_clicked
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: void on_ffBTN_clicked()
--
-- RETURNS: void
--
-- NOTES:
--  Called when user clicks a buttion to fastforward the current playing song.
--  If a song is playing, change the sampling rate of the audio device to 1.5 times faster.
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_ffBTN_clicked()
{
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
    connect(audio, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));
    audio->start(audioPlayer);
    setVolume(audio_volume);

    server.TCPBroadCast(FASTFORWORD);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: on_playBTN_clicked
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: void on_playBTN_clicked()
--
-- RETURNS: void
--
-- NOTES:
--  Called when user clicks a buttion to play or pause the current playing song.
--  If a song is playing, puase the song, otherwise, play the song in the first row of the playlist.
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_playBTN_clicked()
{
    server.TCPBroadCast(PLAYPAUSE);
    if(audioPlayer->isPaused()){
        qDebug()<<"Play";
        audioPlayer->start();
        audio->resume();
        ui->playBTN->setStyleSheet(QString("QPushButton {border-image:url(../assets/ui/pause);}"));
    }
    else if(audioPlayer->isPlaying()){
        qDebug()<<"Pause";
        audio->suspend();
        audioPlayer->pause();
        ui->playBTN->setStyleSheet(QString("QPushButton {border-image:url(../assets/ui/play);}"));
    }
    else
        playNextSong();
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: handleStateChanged
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: void handleStateChanged(QAudio::State newState)
--                  @newState : current state of audio output device
--
-- RETURNS: void
--
-- NOTES:
--  Called whenever the state of the audio device is changed.
--  If the state is IdleState, it means playing song is finished so play the next song
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::handleStateChanged(QAudio::State newState)
{
    switch (newState) {
        case QAudio::IdleState:
            qDebug() << "IdleState";
            removePlaylist();
            audio->stop();
            playNextSong();
            break;

        case QAudio::StoppedState:
            qDebug() << "StoppedState";
            // Stopped for other reasons
            if (audio->error() != QAudio::NoError) {
                // Error handling
            }
            break;
        case QAudio::SuspendedState:
            qDebug() << "SuspendedState";
            break;
        case QAudio::ActiveState:
            qDebug() << "ActiveState";
            break;
        default:
            // ... other cases as appropriate
            break;
    }
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: handleChunkStream
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: void handleChunkStream(qint64 len, qint64 pos)
--                  @len : size of audio file chunk to be streamed
                    @pos : position of the chunk to be streamed
--
-- RETURNS: void
--
-- NOTES:
--  Called whenever the audio device read a chunk of audio file from the buffer.
--  Adds the chunk audio into the queue to be streamed to the clients
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::handleChunkStream(qint64 len, qint64 pos){
    server.addStreamData(audioPlayer->readChunkData(len, pos).prepend('7'));
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: sendHeader
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: void sendHeader()
--
-- RETURNS: void
--
-- NOTES:
--  Called whenever the audio format of the server's audio device is changed.
--  Adds the header of audio file that contains the format of audio device into the queue to be streamed to clients.
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::sendHeader(){
    server.addStreamData(audioPlayer->readHeaderData().prepend('8'));
    QString progress = QString::number(audioPlayer->pos()) + "," +  QString::number(audioPlayer->audioBufferSize())  + ",";
    qDebug()<<progress;
    server.addStreamData(QByteArray(progress.toStdString().c_str()).prepend('9'));
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: on_serverStartBTN_clicked
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: void on_serverStartBTN_clicked()
--
-- RETURNS: void
--
-- NOTES:
--  Called when user clicks a buttion to start running this server
--  Reads a port number from the input field and start TCP and UDP server by passing the port number.
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_serverStartBTN_clicked()
{
    //QString ipAddr = ui->serverIP->text();
    QString port = ui->serverPort->text();

    if(port.length() < 1)
        return;

    if(server.setPort(port)){
        std::thread(&Server::start, &server).detach();
    }

}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: handleReceivedCommand
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: void handleReceivedCommand(int command)
--                  @command : command received from client
--
-- RETURNS: void
--
-- NOTES:
--  Called when a command message is received from a client over TCP.
--  Handles the received command by matching with the defined command.
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::handleReceivedCommand(int command)
{
    switch(command){
    case PLAYPAUSE: //play or pause
        on_playBTN_clicked();
        return;
    case FASTFORWORD: // fastforward
        on_ffBTN_clicked(); //Need to broadcast to all clients
        return;
    case REWIND: // rewind
        on_rewindBTN_clicked();
        return;
    case SKIPTRACK: // skip track
        on_skipBTN_clicked();
        return;
    }
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: handleDisconnetClient
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: void handleDisconnetClient(QString client)
--                  @client : ip address of client
--
-- RETURNS: void
--
-- NOTES:
--  Called when a client disconnects from this server.
--  Removes the client ip from the list view.
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::handleDisconnetClient(QString client){
    QStringList list = clientlist_model->stringList();
    foreach (QString ip, list) {
        if(ip.compare(client) == 0) {
            list.removeOne(ip);
            break;
        }
    }

    clientlist_model->setStringList(list);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: handleNewClient
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: void handleNewClient(int client_num)
--                  @client_num : index of client socket in vector array
--
-- RETURNS: void
--
-- NOTES:
--  Called when a client has connected to this server.
--  Sends the format of current playing song to the client if it is playing over TCP.
--  Sends the lists of playlist and available songs to the clients over TCP.
--  Adds the client's ip to the list view
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::handleNewClient(int client_num)
{
    if(audioPlayer->isPlaying()){ // SendHeader
        server.sendToClient(client_num, HEADER, audioPlayer->readHeaderData());
        QString progress = QString::number(audioPlayer->pos()) + "," +  QString::number(audioPlayer->audioBufferSize()) + ",";
        qDebug()<<progress;
        server.sendToClient(client_num, PROGRESS, QByteArray(progress.toStdString().c_str()));
    }

    //Send list of songs.
    QString availableSongs;
    foreach(QString item, available_song_model->stringList()){
        availableSongs += item + "/";
    }
    QString playList;
    foreach(QString item, playlist_model->stringList()){
        playList += item + "/";
    }

    server.sendToClient(client_num, AVAILSONG, QByteArray(availableSongs.toStdString().c_str()));
    server.sendToClient(client_num, PLAYLIST, QByteArray(playList.toStdString().c_str()));

    clientlist_model->insertRow(clientlist_model->rowCount());
    QModelIndex row = clientlist_model->index(clientlist_model->rowCount()-1);
    clientlist_model->setData(row, server.getClientIP(client_num));
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: handleReceivedAddPlaylist
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: void handleReceivedAddPlaylist(QString item)
--                      @item : name of song to be added
--
-- RETURNS: void
--
-- NOTES:
--  Called when a client selects and adds a song from the list of available songs to playlist
--  Adds the song to playlist.
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::handleReceivedAddPlaylist(QString item){
    addPlaylist(item);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: removePlaylist
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: void removePlaylist()
--
-- RETURNS: void
--
-- NOTES:
--  Called when a current playing song is finished or skipped.
--  Removes the song from the playlist and broadcasts the playlist to clients
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::removePlaylist(){
    playlist_model->removeRow(0);
    QString playList;
    foreach(QString item, playlist_model->stringList()){
        playList += item + "/";
    }
    server.TCPBroadCast(PLAYLIST, QByteArray(playList.toStdString().c_str()));
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: addPlaylist
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: void addPlaylist(QString &item)
--                  @item : name of song to be added
--
-- RETURNS: void
--
-- NOTES:
--  Called when a current playing song is finished or skipped.
--  Adds the song to the playlist and broadcasts the name of song to clients
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::addPlaylist(QString &item){
    playlist_model->insertRow(playlist_model->rowCount());
    QModelIndex row = playlist_model->index(playlist_model->rowCount()-1);
    playlist_model->setData(row, item);
    server.TCPBroadCast(ADDLIST, QByteArray(item.toStdString().c_str()));
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: setVolume
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Jacob frank
--
-- INTERFACE: void setVolume(int value)
--                  @value : value to change the volume of audio
--
-- RETURNS: void
--
-- NOTES:
--  Called when user changes the slider of volume control.
--  Changes the volume of the audio device.
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::setVolume(int value)
{
    audio_volume = value;
    audio->setVolume((float)value / 100);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: setProgress
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: void setProgress(int value)
--                  @value : value to change the progress bar
--
-- RETURNS: void
--
-- NOTES:
--  Called when the current playing song is progressed
--  Changes the progress bar.
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::setProgress(int value){
    ui->ProgressSlider->setValue(value);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: on_uploadBTN_clicked
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: void on_uploadBTN_clicked()
--
-- RETURNS: void
--
-- NOTES:
--  Called when user clicks a buttion to upload a file to the list of available songs
--  Opens a dialog to select a file locally and copies the file to the folder where available songs are placed.
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::on_uploadBTN_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this ,
                                                    QObject::tr("wav files"),
                                                    "C:/",
                                                    "wav(*.wav)");
    QFileInfo fi(filePath);
    QString fileName= fi.fileName();
    QString destinationPath= "C:\\Users\\Administrator\\Desktop\\COMP4985_ASN03\\assets\\musics\\"+fileName;
    if(QFile::copy(filePath, destinationPath))
        qDebug() << "success";
    else
        qDebug() << "failed";
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: updateAvaliableSongs
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: void updateAvaliableSongs()
--
-- RETURNS: void
--
-- NOTES:
--  Called when a new song is uploaded.
--  Broadcasts the updated list of available songs to clients.
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::updateAvaliableSongs(){
    findAvailableSongs();
    QString availableSongs;
    foreach(QString item, available_song_model->stringList()){
        availableSongs += item + "/";
    }
    server.TCPBroadCast(AVAILSONG, QByteArray(availableSongs.toStdString().c_str()));
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: resizeEvent
--
-- DATE: April 10, 2017
--
-- DESIGNER: Jacob frank
--
-- PROGRAMMER: Jacob frank
--
-- INTERFACE: void resizeEvent(QResizeEvent* event)
--                      @event : resize event.
--
-- RETURNS: void
--
-- NOTES:
--  Upldates the backround image to match the resized window
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::resizeEvent (QResizeEvent* event) {
  Q_UNUSED(event);
  m_pPalette->setBrush(QPalette::Background,QBrush(m_pPixmap->scaled(width(),height())));
  setPalette(*m_pPalette);
}
