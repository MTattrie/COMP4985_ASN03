#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    isPlaying = false;
    isPaused = false;

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
    QAudioFormat format;
    // Set up the format, eg.
    format.setSampleRate(44100);
    format.setChannelCount(2);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    if (!info.isFormatSupported(format)) {
        qWarning() << "Raw audio format not supported by backend, cannot play audio.";
        return;
    }

    audio = new QAudioOutput(format, this);
    connect(audio, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));
}

void MainWindow::playNextSong(){
    if(!isPlaying){
        if(playlist_model->stringList().length() <= 0)
            return;
        isPlaying = true;

        if(audio && audio->state() == QAudio::SuspendedState){
            audio->resume();
            return;
        }

        QFile sourceFile;
        sourceFile.setFileName("../assets/musics/" + playlist_model->stringList().at(0));
        sourceFile.open(QIODevice::ReadOnly);


        audio->start(&sourceFile);
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
