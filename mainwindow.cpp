#include "mainwindow.h"
#include "gesture_server.h"
#include <QFileDialog>
#include <QDebug>
#include <QLabel>
#include <QFont>
#include <QIcon>
#include <QGridLayout>
#include <QMessageBox>
#include <QCloseEvent>
#include <QProcess>
#include <filesystem>
#include <stdexcept>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Multimedia Library");
    setWindowIcon(QIcon("./icons/app_icon.png"));

    stack = new QStackedWidget(this);
    setCentralWidget(stack);

    menuPage = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(menuPage);

    QLabel *titleLabel = new QLabel("Multimedia Library");
    QFont titleFont;
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel, 0, Qt::AlignTop);

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

    QGridLayout *grid = new QGridLayout();
    grid->setContentsMargins(50, 0, 50, 50);
    grid->setSpacing(20);
    grid->addWidget(mediaButton, 0, 0, Qt::AlignCenter);
    grid->addWidget(textButton, 0, 1, Qt::AlignCenter);
    grid->addWidget(imageButton, 1, 0, Qt::AlignCenter);
    grid->addWidget(cameraButton, 1, 1, Qt::AlignCenter);
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 1);

    mainLayout->addLayout(grid);
    menuPage->setLayout(mainLayout);

    mediaPlayerPage = new MediaPlayer(this);
    textViewerPage = new TextViewer(this);
    imageViewerPage = new ImageViewer({}, this);

    gestureServer = new GestureServer(this);

    connect(gestureServer, &GestureServer::gestureReceived, this, [=](const QString &cmd) {
        QWidget *current = stack->currentWidget();

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

        if (auto tv = qobject_cast<TextViewer *>(current)) {
            if (cmd == "next") {
                tv->nextPage();
            } else if (cmd == "prev") {
                tv->prevPage();
            } else if (cmd == "go_menu") {
                goBackToMenu();
            }
        }

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

    stack->addWidget(menuPage);
    stack->addWidget(mediaPlayerPage);
    stack->addWidget(textViewerPage);
    stack->addWidget(imageViewerPage);
    stack->setCurrentWidget(menuPage);

    connect(mediaButton, &QPushButton::clicked, this, &MainWindow::openMediaPlayer);
    connect(textButton, &QPushButton::clicked, this, &MainWindow::openTextReader);
    connect(imageButton, &QPushButton::clicked, this, &MainWindow::openImageViewer);
    connect(cameraButton, &QPushButton::clicked, this, &MainWindow::openCamera);
}

MainWindow::~MainWindow() {
    terminateCameraProcess();
}

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
    try {
        if (!cameraRunning) {
            QString exePath = "gesture_client.exe";

            if (!QFile::exists(exePath)) {
                throw std::runtime_error("gesture_client.exe not found. Check if it's in the application directory.");
            }

            cameraProcess = new QProcess(this);
            cameraProcess->setProgram(exePath);
            cameraProcess->setArguments({});
            cameraProcess->setProcessChannelMode(QProcess::MergedChannels);
            cameraProcess->setReadChannel(QProcess::StandardOutput);

            connect(cameraProcess, &QProcess::readyRead, this, [this]() {
                if (cameraProcess) {
                    QByteArray output = cameraProcess->readAllStandardOutput();
                    qDebug() << "[GESTURE]" << output;
                }
            });

            cameraProcess->start();
            if (!cameraProcess->waitForStarted(1000)) {
                QString errMsg = "Failed to start gesture_client.exe: " + cameraProcess->errorString();
                qDebug() << errMsg;
                delete cameraProcess;
                cameraProcess = nullptr;
                throw std::runtime_error(errMsg.toStdString());
            }

            cameraRunning = true;
            cameraButton->setText("🛑 Stop Camera");
        } else {
            terminateCameraProcess();
        }

        qDebug() << "Starting/Stopping Camera...";
    }
    catch (const std::exception &e) {
        QMessageBox::critical(this, "Camera error", QString("Error:\n%1").arg(e.what()));
    }
}

void MainWindow::goBackToMenu() {
    stack->setCurrentWidget(menuPage);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    qDebug() << "MainWindow is closing.";
    terminateCameraProcess();
    event->accept();
}

void MainWindow::terminateCameraProcess() {
    qDebug() << "Forcefully terminating gesture_client.exe";

    forceKillGestureClient();

    if (cameraProcess) {
        cameraProcess->deleteLater();
        cameraProcess = nullptr;
    }

    cameraRunning = false;
    cameraButton->setText("📷 Camera");
}

void MainWindow::forceKillGestureClient() {
    qDebug() << "Force-killing gesture_client.exe via taskkill...";

    QProcess killer;
    killer.start("taskkill", QStringList() << "/F" << "/IM" << "gesture_client.exe");
    killer.waitForFinished(3000);

    QByteArray output = killer.readAllStandardOutput();
    QByteArray errors = killer.readAllStandardError();

    qDebug() << "[taskkill output]" << output;
    qDebug() << "[taskkill errors]" << errors;
}
