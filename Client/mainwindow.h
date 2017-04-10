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
    void decodeMessage(QString message);

    void resizeEvent (QResizeEvent* event)
      {
          m_pPalette->setBrush(QPalette::Background,QBrush(m_pPixmap->scaled(width(),height())));
          setPalette(*m_pPalette);
      };

signals:
    void requestSong(QString song);

private slots:

    void on_button_addSong_clicked();

    void on_button_play_clicked();

    void on_button_skip_clicked();

    void on_button_download_clicked();

    void on_button_FastForward_clicked();

    void on_button_rewind_clicked();

    void handleReceivedHeader(char *data, qint64 len);

    void handleReceivedChunk(char *data, qint64 len);

    void handleReceivedAvailSongs(char *);

    void handleReceivedPlaylist(char *);


    void handleReceivedProgressData(char *);


    void setVolume(int value);
    void setProgress(int value);

private:
    Ui::MainWindow *ui;
    QStringListModel *available_song_model;
    QStringListModel *playlist_model;
    QAudioOutput* audio; // class member.
    AudioPlayer *audioPlayer;
    WavFile sourceFile;
    QPixmap* m_pPixmap;
    QPalette* m_pPalette;

    bool isSetHeader;

    void playNextSong();
    void initAudioOuput();
    bool setAudioHeader(QAudioFormat format);

    Client client;

};

#endif // MAINWINDOW_H
