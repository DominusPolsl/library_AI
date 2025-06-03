#include "mediaplayer.h"
#include <QDebug>
#include <QFileInfo>
#include <QTimer>

#include <QAudioFormat>      // формат аудіо
#include <QAudioOutput>      // вихід
#include <QAudioDevice>      // пристрої виводу
#include <QMediaDevices>     // доступні пристрої



MediaPlayer::MediaPlayer(QWidget *parent)
    : QWidget(parent)
{
    // === Відео ===
    videoWidget = new QVideoWidget(this);

    // === Zdjęcie widoczne pod czas odtwarzania plików muzycznych ==
    imageLabel = new QLabel(this);
    imageLabel->setAlignment(Qt::AlignCenter);
    QPixmap placeholder("../icons/music-notes.png");
    imageLabel->setPixmap(placeholder.scaled(640, 360, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    imageLabel->setVisible(false); // приховати спочатку

    // === Кнопки керування ===
    rewindButton = new QPushButton("⏪", this);
    playPauseButton = new QPushButton(this);
    playPauseButton->setIcon(QIcon("../icons/play_button_proj.png"));  // lub bez ":/" jeśli masz pełną ścieżkę
    playPauseButton->setIconSize(QSize(30, 30));          // dopasuj rozmiar
    forwardButton = new QPushButton("⏩", this);
    playlistButton = new QPushButton("📂", this);
    prevTrackButton = new QPushButton("⏮", this);
    nextTrackButton = new QPushButton("⏭", this);

    prevTrackButton->setFlat(true);
    nextTrackButton->setFlat(true);
    rewindButton->setFlat(true);
    playPauseButton->setFlat(true);
    forwardButton->setFlat(true);
    playlistButton->setFlat(true);

    QLabel *soundIcon = new QLabel(this);
    QPixmap soundPixmap("../icons/volume_icon.png");  // шлях до твоєї картинки
    soundIcon->setPixmap(soundPixmap.scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation));


    // === Слайдери ===
    progressSlider = new QSlider(Qt::Horizontal, this);
    volumeSlider = new QSlider(Qt::Horizontal, this);
    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(100);

    // === Годинник часу ===
    timeDisplay = new QLabel("00:00:00 / 00:00:00", this);
    timeDisplay->setAlignment(Qt::AlignRight);
    timeDisplay->setStyleSheet("font-family: monospace; font-size: 14px;");

    // === Статус ===
    statusLabel = new QLabel("Status: Ready", this);

    // === Аудіо ===
    QAudioDevice device = QMediaDevices::defaultAudioOutput();
    audioOutput = new QAudioOutput(device, this);
    audioOutput->setVolume(1.0);

    mediaPlayer = new QMediaPlayer(this);
    mediaPlayer->setVideoOutput(videoWidget);
    mediaPlayer->setAudioOutput(audioOutput);

    // === Плейліст-панель ===
    playlistPanel = new QFrame(this);
    playlistPanel->setFrameShape(QFrame::StyledPanel);
    playlistPanel->setStyleSheet("background-color: #10152A; color: white;");
    playlistPanel->setVisible(false);

    QVBoxLayout *playlistLayout = new QVBoxLayout(playlistPanel);
    QLabel *playlistTitle = new QLabel("Playlist", playlistPanel);
    playlistTitle->setStyleSheet("font-weight: bold; font-size: 16px;");

    addToPlaylistButton = new QPushButton("➕", playlistPanel);
    addToPlaylistButton->setFixedSize(24, 24);
    addToPlaylistButton->setFlat(true);

    QHBoxLayout *playlistHeader = new QHBoxLayout();
    playlistHeader->addWidget(playlistTitle);
    playlistHeader->addStretch();
    playlistHeader->addWidget(addToPlaylistButton);

    playlistWidget = new QListWidget(playlistPanel);
    playlistWidget->setStyleSheet("background-color: #0A0F22; color: white;");

    playlistLayout->addLayout(playlistHeader);
    playlistLayout->addWidget(playlistWidget);
    playlistPanel->setFixedWidth(300);

    // === Layout ===
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QHBoxLayout *videoLayout = new QHBoxLayout();           // відео + плейліст
    QHBoxLayout *progressLayout = new QHBoxLayout();       // лише слайдер
    QHBoxLayout *timeLayout = new QHBoxLayout();          // прогрес + таймер
    QHBoxLayout *controlsLayout = new QHBoxLayout();        // кнопки

    mediaStack = new QStackedLayout();
    mediaStack->addWidget(videoWidget);
    mediaStack->addWidget(imageLabel);
    videoLayout->addLayout(mediaStack);
    videoLayout->addWidget(playlistPanel);

    progressLayout->addWidget(progressSlider);  // тільки слайдер

    timeLayout->addWidget(timeDisplay);
    timeLayout->addStretch();
    timeLayout->addWidget(playlistButton);  

    controlsLayout->addStretch();
    controlsLayout->addWidget(prevTrackButton);
    controlsLayout->addWidget(rewindButton);
    controlsLayout->addWidget(playPauseButton);
    controlsLayout->addWidget(forwardButton);
    controlsLayout->addWidget(nextTrackButton);
    controlsLayout->addStretch();

    controlsLayout->addWidget(soundIcon);             // 🔊 ПЕРЕД слайдером
    controlsLayout->addSpacing(5);                    // (опціонально)
    controlsLayout->addWidget(volumeSlider);          // 🎚
    controlsLayout->addWidget(playlistButton);  

    mainLayout->addLayout(videoLayout, 9);
    mainLayout->addLayout(progressLayout);   // слайдер (широкий)
    mainLayout->addLayout(timeLayout);       // час + кнопка
    mainLayout->addLayout(controlsLayout);   // кнопки керування
    mainLayout->addWidget(statusLabel);

    setLayout(mainLayout);
    resize(800, 480);
    setWindowTitle("Qt Media Player");

    // === Події ===
    connect(playPauseButton, &QPushButton::clicked, this, &MediaPlayer::togglePlayPause);
    connect(rewindButton, &QPushButton::clicked, [=]() {
        mediaPlayer->setPosition(mediaPlayer->position() - 10000);
    });
    connect(forwardButton, &QPushButton::clicked, [=]() {
        mediaPlayer->setPosition(mediaPlayer->position() + 10000);
    });
    connect(volumeSlider, &QSlider::valueChanged, [=](int value) {
        audioOutput->setVolume(value / 100.0);
    });
    connect(progressSlider, &QSlider::sliderMoved, [=](int value) {
        mediaPlayer->setPosition(value * 1000);
    });
    connect(mediaPlayer, &QMediaPlayer::positionChanged, this, &MediaPlayer::updatePosition);
    connect(mediaPlayer, &QMediaPlayer::durationChanged, this, &MediaPlayer::updateDuration);

    connect(playlistButton, &QPushButton::clicked, [=]() {
        isPlaylistVisible = !isPlaylistVisible;
        playlistPanel->setVisible(isPlaylistVisible);
    });

    connect(addToPlaylistButton, &QPushButton::clicked, [=]() {
        QString fileName = QFileDialog::getOpenFileName(this, "Add to Playlist",
                                                        QDir::homePath(),
                                                        "Media Files (*.mp3 *.mp4 *.wav *.avi *.mkv *.mov)");
        if (!fileName.isEmpty()) {
            QListWidgetItem *item = new QListWidgetItem(QFileInfo(fileName).fileName());
            item->setData(Qt::UserRole, fileName);
            playlistWidget->addItem(item);
        }
    });

    auto playItemAtIndex = [&](int index) {
    if (index >= 0 && index < playlistWidget->count()) {
        currentPlaylistIndex = index;
        playlistWidget->setCurrentRow(currentPlaylistIndex);

        QListWidgetItem *item = playlistWidget->item(index);
        QString filePath = item->data(Qt::UserRole).toString();
        mediaPlayer->setSource(QUrl::fromLocalFile(filePath));
        QTimer::singleShot(150, this, [=]() {
            updateMediaDisplay();
        });

        QTimer::singleShot(100, [=]() {
            mediaPlayer->setPosition(100);  // легенько вперед
            mediaPlayer->play();
        });

        playPauseButton->setIcon(QIcon("../icons/stop_button_proj.png"));
        statusLabel->setText("Playing: " + QFileInfo(filePath).fileName());
        }
    };



    connect(prevTrackButton, &QPushButton::clicked, [=]() {
        if (currentPlaylistIndex > 0) playItemAtIndex(currentPlaylistIndex - 1);
    });
    connect(nextTrackButton, &QPushButton::clicked, [=]() {
        if (currentPlaylistIndex < playlistWidget->count() - 1) playItemAtIndex(currentPlaylistIndex + 1);
    });
    connect(playlistWidget, &QListWidget::itemClicked, [=](QListWidgetItem *item) {
        int index = playlistWidget->row(item);
        playItemAtIndex(index);
    });
}

MediaPlayer::~MediaPlayer() {}

void MediaPlayer::openFile() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open Media File",
                                                    QDir::homePath(),
                                                    "Media Files (*.mp3 *.mp4 *.wav *.avi *.mkv)");
    if (!fileName.isEmpty()) {
        mediaPlayer->setSource(QUrl::fromLocalFile(fileName));
        mediaPlayer->play();
        updateMediaDisplay();
        playPauseButton->setIcon(QIcon("../icons/stop_button_proj.png"));
        statusLabel->setText("Playing: " + QFileInfo(fileName).fileName());
    }
}

void MediaPlayer::updateMediaDisplay() {
    if (mediaPlayer->hasVideo()) {
        mediaStack->setCurrentWidget(videoWidget);
    } else {
        mediaStack->setCurrentWidget(imageLabel);
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

void MediaPlayer::updateDuration(qint64 duration) {
    totalDuration = duration;
    progressSlider->setMaximum(duration / 1000);
}

void MediaPlayer::updatePosition(qint64 position) {
    int currentSecs = position / 1000;
    int totalSecs = totalDuration / 1000;

    progressSlider->setValue(currentSecs);

    QTime currentTime(0, 0, 0);
    currentTime = currentTime.addSecs(currentSecs);

    QTime totalTime(0, 0, 0);
    totalTime = totalTime.addSecs(totalSecs);

    QString timeStr = currentTime.toString("hh:mm:ss") + " / " + totalTime.toString("hh:mm:ss");
    timeDisplay->setText(timeStr); 
}
    

