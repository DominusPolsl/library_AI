#pragma once
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QProcess>

#include "textviewer.h"
#include "mediaplayer.h"
#include "imageviewer.h"

class GestureServer; 

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void openMediaPlayer();
    void openTextReader();
    void openImageViewer();
    void openCamera();
    void goBackToMenu(); 

private:
    QStackedWidget *stack; // stos stron

    QWidget *menuPage;
    MediaPlayer *mediaPlayerPage;
    TextViewer *textViewerPage;
    ImageViewer *imageViewerPage;

    QPushButton *mediaButton;
    QPushButton *textButton;
    QPushButton *imageButton;
    QPushButton *cameraButton;

    QProcess *cameraProcess = nullptr;
    bool cameraRunning = false;

    GestureServer *gestureServer; // pole do serwera gestów
    void terminateCameraProcess();
    void forceKillGestureClient();
};    