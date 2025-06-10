#include "mainwindow.h"
#include "gesture_server.h"
#include <QFileDialog>
#include <QDebug>
#include <QLabel>
#include <QFont>
#include <QGridLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Multimedia Library");
    setWindowIcon(QIcon("../icons/app_icon.png"));

    stack = new QStackedWidget(this);
    setCentralWidget(stack);

    // === Головне меню ===
    menuPage = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(menuPage);

    // Заголовок
    QLabel *titleLabel = new QLabel("Multimedia Library");
    QFont titleFont;
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel, 0, Qt::AlignTop);

    // Кнопки
    mediaButton = new QPushButton("🎵 Video / Music Player");
    textButton = new QPushButton("📄 Text Viewer");
    imageButton = new QPushButton("🖼️ Image Viewer");
    cameraButton = new QPushButton("📷 Camera");

    QFont btnFont;
    btnFont.setPointSize(14);
    QList<QPushButton*> buttons = { mediaButton, textButton, imageButton, cameraButton };
    for (auto *btn : buttons) {
        btn->setFont(btnFont);
        btn->setMinimumWidth(250);
        btn->setFixedHeight(100);
        btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    }

    // Сітка 2x2
    QGridLayout *grid = new QGridLayout();
    grid->setContentsMargins(50, 0, 50, 50);
    grid->setSpacing(20);
    grid->addWidget(mediaButton,        0, 0, Qt::AlignCenter);
    grid->addWidget(textButton,         0, 1, Qt::AlignCenter);
    grid->addWidget(imageButton,        1, 0, Qt::AlignCenter);
    grid->addWidget(cameraButton,       1, 1, Qt::AlignCenter);
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 1);

    mainLayout->addLayout(grid);
    menuPage->setLayout(mainLayout);

    // === Сторінки ===
    mediaPlayerPage = new MediaPlayer(this);
    textViewerPage = new TextViewer(this);
    imageViewerPage = new ImageViewer({}, this);

    gestureServer = new GestureServer(this);

    connect(gestureServer, &GestureServer::gestureReceived, this, [=](const QString &cmd) {
        QWidget *current = stack->currentWidget();

        // Жести відкриття програм — лише якщо користувач на головному меню
        if (stack->currentWidget() == menuPage) {
            if (cmd == "open_media") {
                openMediaPlayer();
                return;
            } else if (cmd == "open_text") {
                openTextReader();
                return;
            } else if (cmd == "open_image") {
                openImageViewer();
                return;
            } else if (cmd == "open_camera") {
                openCamera();
                return;
            }
        }


        // === Обробка жестів для TextViewer ===
        if (auto tv = qobject_cast<TextViewer *>(current)) {
            if (cmd == "next") {
                tv->nextPage();
            } else if (cmd == "prev") {
                tv->prevPage();
            } else if (cmd == "go_menu") {
                goBackToMenu();
            }
        }

        // === Обробка жестів для MediaPlayer ===
        if (auto mp = qobject_cast<MediaPlayer *>(current)) {
            if (cmd == "toggle_play_pause") {
                mp->togglePlayPause();
            } else if (cmd == "fast_forward") {
                mp->fastForward();
            } else if (cmd == "rewind") {
                mp->rewind();
            } else if (cmd == "next_track") {
                mp->nextTrack();
            } else if (cmd == "prev_track") {
                mp->previousTrack();
            } else if (cmd == "volume_up") {
                mp->increaseVolume();
            } else if (cmd == "volume_down") {
                mp->decreaseVolume();
            } else if (cmd == "go_menu") {
                goBackToMenu();
            }        
        }


        if (auto iv = qobject_cast<ImageViewer *>(current)) {
            if (cmd == "pan_left") {
                iv->panImage(-50, 0);
            } else if (cmd == "pan_right") {
                iv->panImage(50, 0);
            } else if (cmd == "pan_up") {
                iv->panImage(0, -50);
            } else if (cmd == "pan_down") {
                iv->panImage(0, 50);
            } else if (cmd == "zoom_in") {
                iv->zoomInAtCenter();
            } else if (cmd == "zoom_out") {
                iv->zoomOutAtCenter();
            } else if (cmd == "go_menu") {
                goBackToMenu();
            }
        }
    });
    connect(textViewerPage, &TextViewer::backToMenuRequested, this, &MainWindow::goBackToMenu);
    connect(mediaPlayerPage, &MediaPlayer::backToMenuRequested, this, &MainWindow::goBackToMenu);
    connect(imageViewerPage, &ImageViewer::returnToMainMenuClicked, this, &MainWindow::goBackToMenu);

    // === Додати сторінки до стеку ===
    stack->addWidget(menuPage);         // index 0
    stack->addWidget(mediaPlayerPage);  // index 1
    stack->addWidget(textViewerPage);   // index 2
    stack->addWidget(imageViewerPage);  // index 3

    // === Стартова сторінка — меню ===
    stack->setCurrentWidget(menuPage);

    // === З'єднання кнопок ===
    connect(mediaButton, &QPushButton::clicked, this, &MainWindow::openMediaPlayer);
    connect(textButton, &QPushButton::clicked, this, &MainWindow::openTextReader);
    connect(imageButton, &QPushButton::clicked, this, &MainWindow::openImageViewer);
    connect(cameraButton, &QPushButton::clicked, this, &MainWindow::openCamera);
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
    stack->setCurrentWidget(imageViewerPage);
}

void MainWindow::openCamera() {
    if (!cameraRunning) {
        // 🔁 Запуск gesture_client.py
        cameraProcess = new QProcess(this);
        cameraProcess->setProgram("python");
        cameraProcess->setArguments(QStringList() << "../gesture_client.py");
        cameraProcess->setProcessChannelMode(QProcess::MergedChannels);
        cameraProcess->setReadChannel(QProcess::StandardOutput);

        connect(cameraProcess, &QProcess::readyRead, this, [=]() {
            QByteArray output = cameraProcess->readAllStandardOutput();
            qDebug() << "[PYTHON]" << output;
        });

        cameraProcess->start();
        if (!cameraProcess->waitForStarted(1000)) {
            qDebug() << "Не вдалося запустити gesture_client.py:" << cameraProcess->errorString();
            delete cameraProcess;
            cameraProcess = nullptr;
            return;
        }

        cameraRunning = true;
        cameraButton->setText("🛑 Stop Camera");
    } else {
        // 🔁 Зупинка gesture_client.py
        if (cameraProcess) {
            cameraProcess->kill();
            cameraProcess->deleteLater();
            cameraProcess = nullptr;
        }
        cameraRunning = false;
        cameraButton->setText("📷 Camera");
    }
    qDebug() << "Starting/Stopping Camera...";
}

void MainWindow::goBackToMenu() {
    stack->setCurrentWidget(menuPage);
}