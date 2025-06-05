#pragma once
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QStackedWidget>

#include "textviewer.h"
#include "mediaplayer.h"
#include "imageviewer.h"

class GestureServer; // 🔹 forward declaration (не забути)

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openMediaPlayer();
    void openTextReader();
    void openImageViewer();
    void openCamera();
    void goBackToMenu(); // Повернення назад

private:
    QStackedWidget *stack;

    QWidget *menuPage;
    MediaPlayer *mediaPlayerPage;
    TextViewer *textViewerPage;
    ImageViewer *imageViewerPage;

    QPushButton *mediaButton;
    QPushButton *textButton;
    QPushButton *imageButton;
    QPushButton *cameraButton;

    GestureServer *gestureServer; // 🔹 нове поле для сервера
};