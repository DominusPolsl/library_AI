#include "mediaplayer.h"
#include <QDebug>
#include <QFileInfo>

#include <QAudioFormat>      // формат аудіо
#include <QAudioOutput>      // вихід
#include <QAudioDevice>      // пристрої виводу
#include <QMediaDevices>     // доступні пристрої

MediaPlayer::MediaPlayer(QWidget *parent)
    : QWidget(parent)
{
    videoWidget = new QVideoWidget(this);
    openButton = new QPushButton("Open File", this);
    playPauseButton = new QPushButton("Play", this);
    statusLabel = new QLabel("Status: Ready", this);

    QAudioDevice device = QMediaDevices::defaultAudioOutput();

    audioOutput = new QAudioOutput(device, this);

    mediaPlayer = new QMediaPlayer(this);
    mediaPlayer->setVideoOutput(videoWidget);
    mediaPlayer->setAudioOutput(audioOutput);
    audioOutput->setVolume(1.0);

    // Вивід доступних аудіопристроїв
    for (const auto &device : QMediaDevices::audioOutputs()) {
        qDebug() << "Audio Device:" << device.description();
    }
    qDebug() << "Default Audio Device:" << QMediaDevices::defaultAudioOutput().description();

    // Layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(videoWidget);
    layout->addWidget(openButton);
    layout->addWidget(playPauseButton);
    layout->addWidget(statusLabel);
    setLayout(layout);

    connect(openButton, &QPushButton::clicked, this, &MediaPlayer::openFile);
    connect(playPauseButton, &QPushButton::clicked, this, &MediaPlayer::togglePlayPause);

    setWindowTitle("Qt Media Player");
    resize(640, 480);
}

MediaPlayer::~MediaPlayer() {}

void MediaPlayer::openFile() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open Media File",
                                                    QDir::homePath(),
                                                    "Media Files (*.mp3 *.mp4 *.wav *.avi *.mkv)");
    if (!fileName.isEmpty()) {
        mediaPlayer->setSource(QUrl::fromLocalFile(fileName));
        mediaPlayer->play();
        qDebug() << "Audio Output device:" << audioOutput->device().description();
        playPauseButton->setText("Pause");
        statusLabel->setText("Playing: " + QFileInfo(fileName).fileName());
    }
}

void MediaPlayer::togglePlayPause() {
    if (mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
        mediaPlayer->pause();
        playPauseButton->setText("Play");
        statusLabel->setText("Paused");
    } else {
        mediaPlayer->play();
        playPauseButton->setText("Pause");
        statusLabel->setText("Playing");
    }
}
