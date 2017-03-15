#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_6_clicked();

    void on_pushButton_8_clicked();

    void on_ProgressSlider_sliderPressed();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
