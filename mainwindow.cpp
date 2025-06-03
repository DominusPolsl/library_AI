#include "mainwindow.h"
#include <QFileDialog>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    stack = new QStackedWidget(this);
    setCentralWidget(stack);

    // === Головне меню ===
    menuPage = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(menuPage);

    mediaButton = new QPushButton("🎵 Video / Music Player");
    textButton = new QPushButton("📄 Text Viewer");
    imageButton = new QPushButton("🖼️ Image Viewer");
    fileExplorerButton = new QPushButton("📁 File Explorer");

    layout->addWidget(mediaButton);
    layout->addWidget(textButton);
    layout->addWidget(imageButton);
    layout->addWidget(fileExplorerButton);

    menuPage->setLayout(layout);

    // === Сторінки ===
    mediaPlayerPage = new MediaPlayer(this);
    textViewerPage = new TextViewer(this);

    // Кнопка "Назад" у медіаплеєрі
    QPushButton *backBtn1 = new QPushButton("🔙 Back to Menu", mediaPlayerPage);
    backBtn1->move(10, 10);
    connect(backBtn1, &QPushButton::clicked, this, &MainWindow::goBackToMenu);
    connect(textViewerPage, &TextViewer::backToMenuRequested, this, &MainWindow::goBackToMenu);


    // Кнопка "Назад" у текстовому вікні
    //QPushButton *backBtn2 = new QPushButton("🔙 Back to Menu", textViewerPage);
    //backBtn2->move(10, 10);
    //connect(backBtn2, &QPushButton::clicked, this, &MainWindow::goBackToMenu);

    // === Додати сторінки до стеку ===
    stack->addWidget(menuPage);         // index 0
    stack->addWidget(mediaPlayerPage);  // index 1
    stack->addWidget(textViewerPage);   // index 2

    // === Стартова сторінка — меню ===
    stack->setCurrentWidget(menuPage);

    // === З'єднання кнопок ===
    connect(mediaButton, &QPushButton::clicked, this, &MainWindow::openMediaPlayer);
    connect(textButton, &QPushButton::clicked, this, &MainWindow::openTextReader);
    connect(imageButton, &QPushButton::clicked, this, &MainWindow::openImageViewer);
    connect(fileExplorerButton, &QPushButton::clicked, this, &MainWindow::openFileExplorer);
}

MainWindow::~MainWindow() {}

void MainWindow::openMediaPlayer() {
    qDebug() << "Switching to Media Player...";
    stack->setCurrentWidget(mediaPlayerPage);
}

void MainWindow::openTextReader() {
    qDebug() << "Switching to Text Viewer...";
    stack->setCurrentWidget(textViewerPage);
}

void MainWindow::openImageViewer() {
    qDebug() << "Opening Image Viewer...";
    // Можеш додати imageViewerPage за аналогією
}

void MainWindow::openFileExplorer() {
    QString dir = QFileDialog::getExistingDirectory(this, "Open Folder", QDir::homePath());
    if (!dir.isEmpty()) {
        qDebug() << "Selected folder:" << dir;
        // Додай логіку, якщо потрібно
    }
}

void MainWindow::goBackToMenu() {
    stack->setCurrentWidget(menuPage);
}