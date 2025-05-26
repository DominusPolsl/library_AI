#pragma once
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

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

private:
    QPushButton *mediaButton;
    QPushButton *textButton;
    QPushButton *imageButton;
    QPushButton *fileExplorerButton;
};