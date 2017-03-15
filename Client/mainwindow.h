#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>
#include <QStringListModel>
#include <QDebug>
#include <QAudioOutput>


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


};

#endif // MAINWINDOW_H
