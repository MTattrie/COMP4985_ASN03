#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    isPlaying = false;
    isPaused = false;
    isForwarding = false;

    // Create model
    available_song_model = new QStringListModel(this);
    playlist_model = new QStringListModel(this);

    // Glue model and view together
    ui->listView_availSongs->setModel(available_song_model);
    ui->listView_playlist->setModel(playlist_model);

    findAvailableSongs();
    initAudioOuput();
}
void MainWindow::findAvailableSongs(){
    QStringList nameFilter("*.wav");
    QDir directory("../assets/musics");
    QStringList wavFilesList = directory.entryList(nameFilter);

    // Populate the model
    available_song_model->setStringList(wavFilesList);
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


    playlist_model->insertRow(playlist_model->rowCount());
    QModelIndex row = playlist_model->index(playlist_model->rowCount()-1);
    playlist_model->setData(row, itemText);
}

void MainWindow::on_button_play_clicked()
{
    playNextSong();
}

void MainWindow::initAudioOuput(){
    audio = new QAudioOutput();
    audioBuffer = new QBuffer();
}

bool MainWindow::setAudioHeader(QAudioFormat format){
    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    if (!info.isFormatSupported(format)) {
        qWarning() << "Raw audio format not supported by backend, cannot play audio.";
        return false;
    }
    delete audio;
    delete audioBuffer;
    audio = new QAudioOutput(format, this);
    audioBuffer = new QBuffer();
    connect(audio, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));

    return true;
}

void MainWindow::playNextSong(){
    if(!isPlaying){
        if(playlist_model->stringList().length() <= 0)
            return;
        isPlaying = true;
        qDebug() << audio;
        if(audio->state() == QAudio::SuspendedState){
            audio->resume();
            return;
        }

        if(!sourceFile.open("../assets/musics/" + playlist_model->stringList().at(0)))
            return;


        if(!setAudioHeader(sourceFile.fileFormat()))
            return;

        isForwarding = false;
        QByteArray data = sourceFile.readAll();
        audioBuffer->open(QIODevice::ReadWrite);
        audioBuffer->reset();
        audioBuffer->seek(0);
        audioBuffer->write(data);
        audioBuffer->seek(0);
        audio->start(audioBuffer);
        QEventLoop loop;
        do {
            loop.exec();
        } while(audio->state() == QAudio::ActiveState);
    }else{
        audio->suspend();
        isPlaying = false;
    }
}

void MainWindow::handleStateChanged(QAudio::State newState)
{
    switch (newState) {
        case QAudio::IdleState:
            qDebug() << "IdleState";
            // Finished playing (no more data)
            if(isPlaying){
                isPlaying = false;
                playlist_model->removeRow(0);
                audio->stop();
                sourceFile.close();
                playNextSong();
            }
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

void MainWindow::on_button_skip_clicked()
{
    if(isPlaying){
        isPlaying = false;
        playlist_model->removeRow(0);
        audio->stop();
        sourceFile.close();
        playNextSong();
    }
}

void MainWindow::on_pushButton_5_clicked()
{

    int samplebytes = sourceFile.fileFormat().bytesForDuration(1000000);
    qint64 pos = audioBuffer->pos() - samplebytes;

    qDebug()<< samplebytes;

    if(pos<=0)
        pos = 0;
    audioBuffer->seek(0);
}

void MainWindow::on_pushButton_3_clicked()
{
    if(!isPlaying)
        return;
    QAudioFormat format = sourceFile.fileFormat();
    if(isForwarding){
        format.setSampleRate(format.sampleRate());
        isForwarding = false;
    }else {
        format.setSampleRate(format.sampleRate() * 2);
        isForwarding = true;
    }
    delete audio;
    audio = new QAudioOutput(format, this);
    connect(audio, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));

    audio->start(audioBuffer);
}
