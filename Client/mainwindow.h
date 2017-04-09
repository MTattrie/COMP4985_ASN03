#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>
#include <QStringListModel>
#include <QDebug>
#include <QAudioOutput>
#include <QBuffer>
#include <thread>
#include "wavfile.h"
#include "audioplayer.h"
#include "client.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void findAvailableSongs();

signals:
    void requestSong(QString song);

private slots:

    void on_button_addSong_clicked();

    void on_button_play_clicked();

    void on_button_skip_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_3_clicked();

    void on_button_download_clicked();

    void handleReceivedHeader(char *data, qint64 len);

    void handleReceivedChunk(char *data, qint64 len);

private:
    Ui::MainWindow *ui;
    QStringListModel *available_song_model;
    QStringListModel *playlist_model;
    QAudioOutput* audio; // class member.
    AudioPlayer *audioPlayer;
    WavFile sourceFile;

    bool isSetHeader;

    void playNextSong();
    void initAudioOuput();
    bool setAudioHeader(QAudioFormat format);

    Client client;

};

#endif // MAINWINDOW_H
