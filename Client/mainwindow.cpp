#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QObject::connect(this, SIGNAL( requestSong(QString) ), &client, SLOT( requestSong(QString) ));
    QObject::connect(&client, SIGNAL( receivedHeader(char*, qint64)), this, SLOT(handleReceivedHeader(char*,qint64)));
    QObject::connect(&client, SIGNAL( receivedChunkData(char*,qint64)), this, SLOT(handleReceivedChunk(char*,qint64)));


    std::thread(&Client::start, &client).detach();

    initAudioOuput();


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
        isSetHeader = true;
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

