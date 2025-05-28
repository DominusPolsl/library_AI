#include "mainwindow.h"
#include "textviewer.h"

#include <QFileDialog>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {

    QWidget *central = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout();

    mediaButton = new QPushButton("🎵 Video / Music Player");
    textButton = new QPushButton("📄 Text Viewer");
    imageButton = new QPushButton("🖼️ Image Viewer");
    fileExplorerButton = new QPushButton("📁 File Explorer");

    layout->addWidget(mediaButton);
    layout->addWidget(textButton);
    layout->addWidget(imageButton);
    layout->addWidget(fileExplorerButton);

    central->setLayout(layout);
    setCentralWidget(central);

    connect(mediaButton, &QPushButton::clicked, this, &MainWindow::openMediaPlayer);
    connect(textButton, &QPushButton::clicked, this, &MainWindow::openTextReader);
    connect(imageButton, &QPushButton::clicked, this, &MainWindow::openImageViewer);
    connect(fileExplorerButton, &QPushButton::clicked, this, &MainWindow::openFileExplorer);
}

MainWindow::~MainWindow() {}

void MainWindow::openMediaPlayer() {
    qDebug() << "Opening Media Player...";
    // Add code to launch media player window here
}

void MainWindow::openTextReader() {
    qDebug() << "Opening Text Reader...";
    TextViewer *viewer = new TextViewer(this);
    viewer->setAttribute(Qt::WA_DeleteOnClose);
    viewer->setWindowTitle("Text Viewer");
    viewer->showMaximized();
    viewer->show();
}

void MainWindow::openImageViewer() {
    qDebug() << "Opening Image Viewer...";
    // Add code to launch image viewer window here
}

void MainWindow::openFileExplorer() {
    QString dir = QFileDialog::getExistingDirectory(this, "Open Folder", QDir::homePath());
    if (!dir.isEmpty()) {
        qDebug() << "Selected folder:" << dir;
        // Optional: display contents or pass to file manager window
    }
}