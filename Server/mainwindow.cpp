#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Server - ComAudio");

    // Create model
    available_song_model = new QStringListModel(this);
    playlist_model = new QStringListModel(this);

    // Glue model and view together
    ui->listView_availSongs->setModel(available_song_model);
    ui->listView_playlist->setModel(playlist_model);


    findAvailableSongs();
    initAudioOuput();

    std::thread(&Server::start, &server).detach();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete audio;
    delete audioPlayer;
}

void MainWindow::findAvailableSongs(){
    QStringList nameFilter("*.wav");
    QDir directory("../assets/musics");
    QStringList wavFilesList = directory.entryList(nameFilter);

    // Populate the model
    available_song_model->setStringList(wavFilesList);
}


void MainWindow::initAudioOuput(){
    audio = new QAudioOutput();
    audioPlayer = new AudioPlayer();
    connect(audioPlayer, SIGNAL(songFinished()), this, SLOT(handleSongFinished()));
    connect(audioPlayer, SIGNAL(streamChunkAudio(qint64,qint64)), this, SLOT(handleChunkStream(qint64,qint64)));
    connect(&server, SIGNAL(receivedCommand(int)), this, SLOT(handleReceivedCommand(int)));
    connect(&server, SIGNAL(newClientConnected(int)), this, SLOT(handleNewClient(int)));

}

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

void MainWindow::playNextSong(){
    if(playlist_model->stringList().length() <= 0)
        return;

    if(!audioPlayer->openWavFile("../assets/musics/" + playlist_model->stringList().at(0)))
        return;

    if(!setAudioHeader(audioPlayer->fileFormat()))
        return;
    audio->start(audioPlayer);
    audioPlayer->start();
    audio->setVolume(0);
    songFinished = false;
}

void MainWindow::on_addPlaylistBTN_clicked()
{
    QModelIndex index = ui->listView_availSongs->currentIndex();
    if(index.row() < 0)
        return;

    QString itemText = index.data(Qt::DisplayRole).toString();


    playlist_model->insertRow(playlist_model->rowCount());
    QModelIndex row = playlist_model->index(playlist_model->rowCount()-1);
    playlist_model->setData(row, itemText);
}

void MainWindow::on_skipBTN_clicked()
{
    if(audioPlayer->isPlaying()){
        //audioPlayer->pause();
        audio->stop();
        playlist_model->removeRow(0);
        playNextSong();
    }
}

void MainWindow::on_rewindBTN_clicked()
{
    int samplebytes = audioPlayer->fileFormat().bytesForDuration(3000000);
    qint64 pos = audioPlayer->pos() - samplebytes;

    qDebug()<< samplebytes;

    if(pos<=0)
        pos = 0;
    audioPlayer->seek(pos);
}

void MainWindow::on_ffBTN_clicked()
{
    if(!audioPlayer->isPlaying())
        return;

    QAudioFormat format = audioPlayer->fileFormat();
    if(audioPlayer->isFastForwarding()){
        audioPlayer->isFastForwarding(false);
    }else {
        format.setSampleRate(format.sampleRate() * 2);
        audioPlayer->isFastForwarding(true);
    }

    delete audio;
    audio = new QAudioOutput(format, this);
    connect(audio, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));
    audio->start(audioPlayer);
    audio->setVolume(0);

}

void MainWindow::on_playBTN_clicked()
{
    if(audioPlayer->isPaused()){
        qDebug()<<"Play";
        audioPlayer->start();
        audio->resume();
    }
    else if(audioPlayer->isPlaying()){
        qDebug()<<"Pause";
        audio->suspend();
        audioPlayer->pause();
    }
    else
        playNextSong();
}

void MainWindow::handleSongFinished(){
    //audioPlayer->pause();
    //audio->stop();
    //delete audio;
    //playlist_model->removeRow(0);
    //playNextSong();
    //songFinished = true;
}

void MainWindow::handleStateChanged(QAudio::State newState)
{
    switch (newState) {
        case QAudio::IdleState:
            qDebug() << "IdleState";
            playlist_model->removeRow(0);
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

void MainWindow::handleChunkStream(qint64 len, qint64 pos){
    server.addStreamData(audioPlayer->readChunkData(len, pos).prepend('7'));
    //server.addStreamData("DATA");
}

void MainWindow::sendHeader(){
    server.addStreamData(audioPlayer->readHeaderData().prepend('8'));
    QString progress = QString::number(audioPlayer->pos()) + "," +  QString::number(audioPlayer->audioBufferSize());
    server.addStreamData(QByteArray(progress.toStdString().c_str()).prepend('9'));
}

void MainWindow::on_serverStartBTN_clicked()
{
    QString ipAddr = ui->serverIP->text();
    QString port = ui->serverPort->text();

    if(port.length() < 1)
        return;

    if(server.setPort(port)){
    }

}

void MainWindow::handleReceivedCommand(int command)
{
    switch(command){
    case 3: //play or pause
        on_playBTN_clicked();
        return;
    case 4: // fastforward
        //on_ffBTN_clicked(); //Need to broadcast to all clients
        return;
    case 5: // rewind
        on_rewindBTN_clicked();
        return;
    case 6: // skip track
        on_skipBTN_clicked();
        return;
    }
}

void MainWindow::handleNewClient(int client_num)
{
    qDebug()<< "handleNewClient" ;
    if(audioPlayer->isPlaying()){ // SendHeader
        server.sendToClient(client_num, HEADER, audioPlayer->readHeaderData());
        QString progress = QString::number(audioPlayer->pos()) + "," +  QString::number(audioPlayer->audioBufferSize());
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

}
