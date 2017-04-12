#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <fstream>
#include <ostream>
#include <QFileDialog>

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


void MainWindow::findAvailableSongs(){

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_button_addSong_clicked()
{
    QModelIndex index = ui->listView_availSongs->currentIndex();
    if(index.row() < 0)
        return;

    QString itemText = index.data(Qt::DisplayRole).toString();
    client.reqeustCommand(ADDLIST, itemText);
}

void MainWindow::on_button_play_clicked()
{
    qDebug()<<"on_button_play_clicked";
    client.reqeustCommand(PLAYPAUSE);
}

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


void MainWindow::on_button_FastForward_clicked()
{
    client.reqeustCommand(FASTFORWORD);
}

void MainWindow::on_button_rewind_clicked()
{
    client.reqeustCommand(REWIND);
}

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

void MainWindow::on_button_download_clicked()
{
    QModelIndex index = ui->listView_availSongs->currentIndex();
    if(index.row() < 0)
        return;
    QString itemText = index.data(Qt::DisplayRole).toString();
    emit requestSong(itemText);
}

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

void MainWindow::updateAvailSongs(char *list){
    QStringList stringList = QString(list).split('/');
    stringList.removeLast();
    available_song_model->setStringList(stringList);
}

void MainWindow::updatePlaylist(char *list){
    QStringList stringList = QString(list).split('/');
    stringList.removeLast();
    playlist_model->setStringList(stringList);
}

void MainWindow::addPlaylist(QString item){
    playlist_model->insertRow(playlist_model->rowCount());
    QModelIndex row = playlist_model->index(playlist_model->rowCount()-1);
    playlist_model->setData(row, item);
}

void MainWindow::updateProgressData(char *progressData){
    QStringList stringList = QString(progressData).split(',');
    qDebug()<<stringList;
    audioPlayer->setProgressData(stringList.at(0).toInt(), stringList.at(1).toInt());
}

void MainWindow::connectToServer() {
    QString hostname = ui->lineEdit_serverHostname->text();
    QString portNumber = ui->lineEdit_serverPortNumber->text();
    std::thread(&Client::start, &client, hostname, portNumber).detach();
}


void MainWindow::setVolume(int value)
{
    audio_volume = value;
    audio->setVolume((float)value / 100);
}

void MainWindow::setProgress(int value){
    ui->ProgressSlider->setValue(value);
}

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

void MainWindow::receivedSkipTrack(){
    qDebug()<<"receivedSkipTrack";
    audio->stop();
    audioPlayer->pause();
    audio->reset();
    audioPlayer->resetPlayer();
}

void MainWindow::rewind(){
    audio->stop();
    audioPlayer->pause();
    audioPlayer->resetPlayer();
}

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

void MainWindow::handleReceivedRecoredData(qint64 len){
    //qDebug()<<"handleReceivedRecoredData";
    client.addMicStream(microphonePlayer->readChunkData(len));
}

void MainWindow::handleReceivedPeerData(char *data, int len){
    microphonePlayer->addChunkData(data, len);
    if(client.peerUDPRunning && mic_out->state() == QAudio::IdleState ){
        mic_out->stop();
        mic_out->start(microphonePlayer);
    }
    delete data;
}

void MainWindow::on_button_upload_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Wav"), "", tr("Audio Files (*.wav)"));
    emit sendSong(fileName);
}
