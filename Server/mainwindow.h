#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <winsock2.h>
#include <QMainWindow>
#include <QDir>
#include <QStringListModel>
#include <QDebug>
#include <QAudioOutput>
#include <QBuffer>
#include <thread>
#include <QtGui>
#include <QPalette>
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

    void resizeEvent (QResizeEvent* event) {
      Q_UNUSED(event);
      m_pPalette->setBrush(QPalette::Background,QBrush(m_pPixmap->scaled(width(),height())));
      setPalette(*m_pPalette);
    }
signals:
    void requestSong(QString song);

private slots:
    void on_addPlaylistBTN_clicked();
    void on_skipBTN_clicked();
    void on_rewindBTN_clicked();
    void on_ffBTN_clicked();
    void on_playBTN_clicked();
    void on_serverStartBTN_clicked();
    void on_uploadBTN_clicked();

    void handleSongFinished();
    void handleStateChanged(QAudio::State newState);
    void handleChunkStream(qint64 len, qint64 pos);
    void handleReceivedCommand(int command);
    void handleNewClient(int client_num);
    void handleDisconnetClient(QString);
    void handleReceivedAddPlaylist(QString);

    void setProgress(int value);
    void setVolume(int value);
    void updateAvaliableSongs();

private:
    Ui::MainWindow *ui;
    QPixmap* m_pPixmap;
    QPalette* m_pPalette;

    QStringListModel *available_song_model;
    QStringListModel *playlist_model;
    QStringListModel *clientlist_model;

    QAudioOutput* audio; // class member.
    AudioPlayer *audioPlayer;

    Server     server;

    bool songFinished;
    int audio_volume;

    void playNextSong();
    void initAudioOuput();
    bool setAudioHeader(QAudioFormat format);
    void sendHeader();
    void removePlaylist();
    void addPlaylist(QString &item);
};

#endif // MAINWINDOW_H
