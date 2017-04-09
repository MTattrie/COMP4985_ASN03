#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>
#include <QStringListModel>
#include <QDebug>
#include <QAudioOutput>
#include <QBuffer>
#include <thread>
#include "server.h"
#include "audioplayer.h"

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
    void on_addPlaylistBTN_clicked();

    void on_skipBTN_clicked();

    void on_rewindBTN_clicked();

    void on_ffBTN_clicked();

    void on_playBTN_clicked();

    void handleSongFinished();

    void handleStateChanged(QAudio::State newState);

    void handleChunkStream(qint64 len, qint64 pos);

    void on_serverStartBTN_clicked();

    void handleReceivedCommand(int command);

private:
    Ui::MainWindow *ui;

    QStringListModel *available_song_model;
    QStringListModel *playlist_model;
    QAudioOutput* audio; // class member.
    AudioPlayer *audioPlayer;

    Server     server;

    bool songFinished;


    void playNextSong();
    void initAudioOuput();
    bool setAudioHeader(QAudioFormat format);
    void sendHeader();
};

#endif // MAINWINDOW_H
