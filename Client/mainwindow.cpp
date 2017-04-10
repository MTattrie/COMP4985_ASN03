#include "mainwindow.h"
#include "ui_mainwindow.h"

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
    QObject::connect(&client, SIGNAL( receivedHeader(char*, qint64)), this, SLOT(handleReceivedHeader(char*,qint64)));
    QObject::connect(&client, SIGNAL( receivedChunkData(char*,qint64)), this, SLOT(handleReceivedChunk(char*,qint64)));
    QObject::connect(&client, SIGNAL( receivedAvailSongs(char*)), this, SLOT(handleReceivedAvailSongs(char*)));
    QObject::connect(&client, SIGNAL( receivedPlaylist(char*)), this, SLOT(handleReceivedPlaylist(char*)));
    QObject::connect(&client, SIGNAL( receivedProgressData(char*)), this, SLOT(handleReceivedProgressData(char*)));


    std::thread(&Client::start, &client).detach();

    initAudioOuput();

    m_pPalette	= new QPalette();
    m_pPixmap = new QPixmap("../assets/ui/background.jpg");

    QPalette* palette = new QPalette();
    palette->setBrush(QPalette::Background,*(new QBrush(*(m_pPixmap))));
    setPalette(*palette);

    connect(ui->volumeSlider,SIGNAL(valueChanged(int)),this,SLOT(setVolume(int)));

    ui->ProgressSlider->setValue(40);
//    setStyleSheet("QSlider::handle:volumeSlider {background-color: red;}");
}


void MainWindow::findAvailableSongs(){

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_button_addSong_clicked()
{

}

void MainWindow::on_button_play_clicked()
{
    client.reqeustCommand(PLAYPAUSE);
}

void MainWindow::on_button_skip_clicked()
{
    client.reqeustCommand(SKIPTRACK);
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
    isSetHeader = false;
    QObject::connect(audioPlayer, SIGNAL( progressAudio(int)), this, SLOT(setProgress(int)));

}

bool MainWindow::setAudioHeader(QAudioFormat format){
    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    if (!info.isFormatSupported(format)) {
        qWarning() << "Raw audio format not supported by backend, cannot play audio.";
        return false;
    }
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

void MainWindow::handleReceivedHeader(char *data, qint64 len)
{
    if(audioPlayer->readHeader(data, len)){
        if(!setAudioHeader(audioPlayer->fileFormat()))
            return;
        audioPlayer->resetPlayer();
        if(isSetHeader){
            playlist_model->removeRow(0);
        }
        isSetHeader = true;
        qDebug()<<"size : " << data;
    }
    else{
        isSetHeader = false;
    }
}
void MainWindow::handleReceivedChunk(char *data, qint64 len){
    audioPlayer->addChunkData(data, len);
    qDebug()<<"audioPlayer->fileFormat().bytesForDuration(1000000) : " << audioPlayer->fileFormat().bytesForDuration(1000000);

    if(!audioPlayer->isPlaying()
            && !audioPlayer->isPaused()
            && audioPlayer->bytesAvailable() >= audioPlayer->fileFormat().bytesForDuration(1000000)){
        audio->start(audioPlayer);
        audioPlayer->start();
    }

}

void MainWindow::handleReceivedAvailSongs(char *list){
    QStringList stringList = QString(list).split('/');
    stringList.removeLast();
    available_song_model->setStringList(stringList);
}
void MainWindow::handleReceivedPlaylist(char *list){
    QStringList stringList = QString(list).split('/');
    stringList.removeLast();
    playlist_model->setStringList(stringList);
}

void MainWindow::handleReceivedProgressData(char *progressData){
    QStringList stringList = QString(progressData).split(',');
    audioPlayer->setProgressData(stringList.at(0).toInt(), stringList.at(1).toInt());
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: decodeMessage
--
-- DATE: April 7, 2017
--
-- DESIGNER:
--
-- PROGRAMMER:
--
-- INTERFACE: void MainWindow::decodeMessage(QString message)
--                  QString message: Text data recevied from the server
--
-- RETURNS: void.
--
-- NOTES:
--  Called when received a message from the server.
--  Reads the first character of the received message and handle the message by the code.
----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::decodeMessage(QString message) {
    qDebug() << "decodeMessage : " << message;


    if(!message.at(0).isNumber())
        return;
    switch(message.at(0).digitValue()){
    case 1: // Dong to Download

        break;
    case 2: // Update Playlist

        break;
    case 3: // Update Available Songs

        break;
    default:
        break;
    }
}


void MainWindow::setVolume(int value)
{
    audio->setVolume((float)value / 100);
}

void MainWindow::setProgress(int value){
    qDebug()<<"value : " << value;
    ui->ProgressSlider->setValue(value);
}
