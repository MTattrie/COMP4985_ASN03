#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>

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
    connect(&server, SIGNAL(clientDisconnected(QString)), this, SLOT(handleDisconnetClient(QString)));
    connect(&server, SIGNAL( receivedAddPlaylist(QString)), this, SLOT(handleReceivedAddPlaylist(QString)));
    connect(audioPlayer, SIGNAL( progressAudio(int)), this, SLOT(setProgress(int)));
    connect(&server, SIGNAL(updateAvaliableSongs()), this, SLOT(updateAvaliableSongs()));


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
    setVolume(audio_volume);
    songFinished = false;
    ui->playBTN->setStyleSheet(QString("QPushButton {border-image:url(../assets/ui/pause);}"));
}

void MainWindow::on_addPlaylistBTN_clicked()
{
    QModelIndex index = ui->listView_availSongs->currentIndex();
    if(index.row() < 0)
        return;

    QString itemText = index.data(Qt::DisplayRole).toString();

    addPlaylist(itemText);
}

void MainWindow::on_skipBTN_clicked()
{
    if(audioPlayer->isPlaying()){
        ui->playBTN->setStyleSheet(QString("QPushButton {border-image:url(../assets/ui/play);}"));
        //audioPlayer->pause();
        audio->stop();
        audioPlayer->resetPlayer();
        removePlaylist();
        //server.resetStreamData();
        playNextSong();
    }
}

void MainWindow::on_rewindBTN_clicked()
{
    audioPlayer->pause();
    int samplebytes = audioPlayer->fileFormat().bytesForDuration(5000000);
    qint64 pos = audioPlayer->pos() - samplebytes;

    qDebug()<< audioPlayer->mypos();
    qDebug()<< pos;

    if(pos<=0)
        pos = 0;
    audioPlayer->myseek(pos);
    QString progress = QString::number(pos) + "," +  QString::number(audioPlayer->audioBufferSize())  + ",";
    server.TCPBroadCast(REWIND, QByteArray(progress.toStdString().c_str()));
    audioPlayer->start();
}

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

void MainWindow::handleChunkStream(qint64 len, qint64 pos){
    server.addStreamData(audioPlayer->readChunkData(len, pos).prepend('7'));
    //server.addStreamData("DATA");
}

void MainWindow::sendHeader(){
    server.addStreamData(audioPlayer->readHeaderData().prepend('8'));
    QString progress = QString::number(audioPlayer->pos()) + "," +  QString::number(audioPlayer->audioBufferSize())  + ",";
    qDebug()<<progress;
    server.addStreamData(QByteArray(progress.toStdString().c_str()).prepend('9'));
}

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

void MainWindow::handleNewClient(int client_num)
{
    qDebug()<< "handleNewClient" ;
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
void MainWindow::handleReceivedAddPlaylist(QString item){
    addPlaylist(item);
}

void MainWindow::removePlaylist(){
    playlist_model->removeRow(0);
    QString playList;
    foreach(QString item, playlist_model->stringList()){
        playList += item + "/";
    }
    server.TCPBroadCast(PLAYLIST, QByteArray(playList.toStdString().c_str()));
}

void MainWindow::addPlaylist(QString &item){
    playlist_model->insertRow(playlist_model->rowCount());
    QModelIndex row = playlist_model->index(playlist_model->rowCount()-1);
    playlist_model->setData(row, item);
    server.TCPBroadCast(ADDLIST, QByteArray(item.toStdString().c_str()));
}

void MainWindow::setVolume(int value)
{
    audio_volume = value;
    audio->setVolume((float)value / 100);
}

void MainWindow::setProgress(int value){
    ui->ProgressSlider->setValue(value);
}

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

void MainWindow::updateAvaliableSongs(){
    findAvailableSongs();
    QString availableSongs;
    foreach(QString item, available_song_model->stringList()){
        availableSongs += item + "/";
    }
    server.TCPBroadCast(AVAILSONG, QByteArray(availableSongs.toStdString().c_str()));
}


