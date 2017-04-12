#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <winsock2.h>
#include <QMainWindow>
#include <QDir>
#include <QStringListModel>
#include <QDebug>
#include <QAudioInput>
#include <QAudioOutput>
#include <QBuffer>
#include <thread>
#include <QtGui>
#include <QPalette>
#include "wavfile.h"
#include "audioplayer.h"
#include "microphoneplayer.h"

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
    void resizeEvent (QResizeEvent* event) {
      Q_UNUSED(event);
      m_pPalette->setBrush(QPalette::Background,QBrush(m_pPixmap->scaled(width(),height())));
      setPalette(*m_pPalette);
    }

signals:
    void requestSong(QString song);
    void sendSong(QString song);

private slots:

    void on_button_addSong_clicked();
    void on_button_play_clicked();
    void on_button_skip_clicked();
    void on_button_download_clicked();
    void on_button_FastForward_clicked();
    void on_button_rewind_clicked();
    void on_button_connectToClient_clicked();
    void on_button_upload_clicked();

    void handleReceivedCommand(int, char *, int);
    void handleReceivedRecoredData(qint64 len);
    void handleReceivedPeerData(char *data, int len);
    void setVolume(int value);
    void setProgress(int value);
    void connectToServer();

private:
    Ui::MainWindow *ui;
    QPixmap* m_pPixmap;
    QPalette* m_pPalette;

    QStringListModel *available_song_model;
    QStringListModel *playlist_model;

    QAudioOutput* audio; // class member.
    QAudioInput* mic_in; // class member.
    QAudioOutput* mic_out; // class member.
    AudioPlayer *audioPlayer;
    MicrophonePlayer *microphonePlayer;
    QAudioFormat mic_format;

    Client client;

    bool isSetHeader;
    int audio_volume;

    void playNextSong();
    void initAudioOuput();
    bool setAudioHeader(QAudioFormat);
    void updateHeader(char *, qint64 );
    void addChunk(char *, qint64);
    void updateAvailSongs(char *);
    void updatePlaylist(char *);
    void updateProgressData(char *);
    void addPlaylist(QString);
    void fastforward();
    void writeFile();
    void receivedSkipTrack();
    void rewind();
    void playpause();
};

#endif // MAINWINDOW_H
