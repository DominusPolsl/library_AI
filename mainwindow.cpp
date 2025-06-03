#include "mainwindow.h"
#include "textviewer.h"
#include "imageviewer.h"

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

    QWidget *oldCentral = centralWidget();
    if (oldCentral)
        oldCentral->deleteLater(); // робота на "одному" вікні
    textViewer = new TextViewer(rememberedFiles);
    connect(textViewer, &TextViewer::returnToMainMenuClicked, this, &MainWindow::showMainMenu);
    connect(textViewer, &TextViewer::fileAdded, this, [this](const QString &path) {if (!rememberedFiles.contains(path)) rememberedFiles.append(path);});
    connect(textViewer, &TextViewer::fileRemoved, this, [this](const QString &path) {rememberedFiles.removeAll(path);});
    setCentralWidget(textViewer);
}

void MainWindow::openImageViewer() 
{
    qDebug() << "Opening Image Viewer...";

    QWidget *oldCentral = centralWidget();
    if (oldCentral)
        oldCentral->deleteLater();
    // Tworzymy nowy obiekt ImageViewer, przekazując ewentualnie listę wcześniej otwartych obrazów
    imageViewer = new ImageViewer(rememberedImages);

    // Podłączamy sygnał powrotu do głównego menu
    connect(imageViewer, &ImageViewer::returnToMainMenuClicked, this, &MainWindow::showMainMenu);

    // Podłączamy sygnał, który mówi, że użytkownik dodał nowy plik (ścieżkę) do "ostatnio otwartych"
    connect(imageViewer, &ImageViewer::fileAdded,this, [this](const QString &path) {if (!rememberedImages.contains(path)) {rememberedImages.append(path);}});

    // Podłączamy sygnał, który mówi, że jakiś plik został usunięty z "ostatnio otwartych"
    connect(imageViewer, &ImageViewer::fileRemoved,this, [this](const QString &path) {rememberedImages.removeAll(path);});

    // Ustawiamy ImageViewer jako nowy centralny widget w MainWindow
    setCentralWidget(imageViewer);
}

void MainWindow::openFileExplorer() {
    QString dir = QFileDialog::getExistingDirectory(this, "Open Folder", QDir::homePath());
    if (!dir.isEmpty()) {
        qDebug() << "Selected folder:" << dir;
        // Optional: display contents or pass to file manager window
    }
}

void MainWindow::showMainMenu() // знадобиться для всіх вікон
{
QWidget *oldCentral = centralWidget();
    if (oldCentral)
        oldCentral->deleteLater();

    // Tworzymy nowy układ przycisków
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

