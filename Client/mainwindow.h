#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>
#include <QStringListModel>
#include <QDebug>
#include <QAudioOutput>
#include "wavfile.h"


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
private slots:

    void on_button_addSong_clicked();

    void on_button_play_clicked();

    void handleStateChanged(QAudio::State newState);
    void on_button_skip_clicked();

private:
    Ui::MainWindow *ui;
    QStringListModel *available_song_model;
    QStringListModel *playlist_model;
    QFile sourceFile;   // class member.
    QAudioOutput* audio; // class member.
    bool isPlaying;
    bool isPaused;

    void playNextSong();
    void initAudioOuput();
    bool setAudioHeader(QAudioFormat format);


};

#endif // MAINWINDOW_H
