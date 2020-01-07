#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "tobiimouse.h"
#include <QMainWindow>
#include "tobii_interactive.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void reloadTobiiDeviceList();

//public slots:
//    void OnGazePositionReceived( tobii_gaze_point_t const* gaze_point );

private slots:
    void on_reloadListButton_clicked();
    void on_useSelectedDeviceButton_clicked();
    void on_actionQuit_triggered();

private:
    Ui::MainWindow *ui;
    QThread* gazeThread;
};


#endif // MAINWINDOW_H
