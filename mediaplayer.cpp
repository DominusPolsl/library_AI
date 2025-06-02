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
    playPauseButton = new QPushButton(this);
    playPauseButton->setIcon(QIcon("../icons/play_button_proj.png"));  // lub bez ":/" jeśli masz pełną ścieżkę
    playPauseButton->setIconSize(QSize(30, 30));          // dopasuj rozmiar
    playPauseButton->setFlat(true); 
    statusLabel = new QLabel("Status: Ready", this);

    progressSlider = new QSlider(Qt::Horizontal, this);
    volumeSlider = new QSlider(Qt::Horizontal, this);
    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(100);

    timeDisplay = new QLCDNumber(this);
    timeDisplay->setSegmentStyle(QLCDNumber::Flat);
    timeDisplay->setDigitCount(5);
    timeDisplay->display("00:00");

    QAudioDevice device = QMediaDevices::defaultAudioOutput();
    audioOutput = new QAudioOutput(device, this);
    audioOutput->setVolume(1.0);

    mediaPlayer = new QMediaPlayer(this);
    mediaPlayer->setVideoOutput(videoWidget);
    mediaPlayer->setAudioOutput(audioOutput);

    // Layouts
    QVBoxLayout *layout = new QVBoxLayout(this);
    QHBoxLayout *progressLayout = new QHBoxLayout();
    QHBoxLayout *controlsLayout = new QHBoxLayout();

    progressLayout->addWidget(progressSlider);
    progressLayout->addWidget(timeDisplay);

    controlsLayout->addWidget(openButton);
    controlsLayout->addWidget(playPauseButton);
    controlsLayout->addWidget(volumeSlider);

    videoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(videoWidget, 9);              // 90% wysokości
    layout->addLayout(progressLayout, 1);           // 5%
    layout->addLayout(controlsLayout, 1);           // 5%
    layout->addWidget(statusLabel);                 // mała etykieta na końcu

    setLayout(layout);

    connect(openButton, &QPushButton::clicked, this, &MediaPlayer::openFile);
    connect(playPauseButton, &QPushButton::clicked, this, &MediaPlayer::togglePlayPause);
    connect(volumeSlider, &QSlider::valueChanged, [=](int value){
        audioOutput->setVolume(value / 100.0);
    });
    connect(progressSlider, &QSlider::sliderMoved, [=](int value){
        mediaPlayer->setPosition(value * 1000); // milisekundy
    });
    connect(mediaPlayer, &QMediaPlayer::positionChanged, this, &MediaPlayer::updatePosition);
    connect(mediaPlayer, &QMediaPlayer::durationChanged, this, &MediaPlayer::updateDuration);

    resize(640, 480);
    setWindowTitle("Qt Media Player");
}

MediaPlayer::~MediaPlayer() {}

void MediaPlayer::openFile() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open Media File",
                                                    QDir::homePath(),
                                                    "Media Files (*.mp3 *.mp4 *.wav *.avi *.mkv)");
    if (!fileName.isEmpty()) {
        mediaPlayer->setSource(QUrl::fromLocalFile(fileName));
        mediaPlayer->play();
        playPauseButton->setIcon(QIcon("../icons/stop_button_proj.png"));
        statusLabel->setText("Playing: " + QFileInfo(fileName).fileName());
    }
}

void MediaPlayer::togglePlayPause() {
    if (mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
        mediaPlayer->pause();
        playPauseButton->setIcon(QIcon("../icons/play_button_proj.png"));
        statusLabel->setText("Paused");
    } else {
        mediaPlayer->play();
        playPauseButton->setIcon(QIcon("../icons/stop_button_proj.png"));
        statusLabel->setText("Playing");
    }
}

void MediaPlayer::updatePosition(qint64 position) {
    int seconds = position / 1000;
    progressSlider->setValue(seconds);
    QString timeStr = QString("%1:%2")
                        .arg(seconds / 60, 2, 10, QChar('0'))
                        .arg(seconds % 60, 2, 10, QChar('0'));
    timeDisplay->display(timeStr);
}

void MediaPlayer::updateDuration(qint64 duration) {
    progressSlider->setMaximum(duration / 1000);
}
