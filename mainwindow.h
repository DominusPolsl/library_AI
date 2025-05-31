#pragma once
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QStackedWidget>

#include "textviewer.h"
#include "mediaplayer.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openMediaPlayer();
    void openTextReader();
    void openImageViewer();
    void openFileExplorer();
    void goBackToMenu(); // Повернення назад

private:
    QStackedWidget *stack;

    QWidget *menuPage;
    MediaPlayer *mediaPlayerPage;
    TextViewer *textViewerPage;

    QPushButton *mediaButton;
    QPushButton *textButton;
    QPushButton *imageButton;
    QPushButton *fileExplorerButton;
};