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
    // === Widok wideo (dla plików MP4, AVI itp.) ===
    videoWidget = new QVideoWidget(this);  // Obiekt do wyświetlania obrazu wideo

    // === Obrazek zastępczy — widoczny tylko przy odtwarzaniu plików audio (np. MP3) ===
    imageLabel = new QLabel(this);
    imageLabel->setAlignment(Qt::AlignCenter);
    QPixmap placeholder("../icons/music-notes.png");
    imageLabel->setPixmap(placeholder.scaled(640, 360, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    imageLabel->setVisible(false);  // Ukryty na start

    rewindButton = new QPushButton("⏪", this);   // Cofnij 10 sekund
    playPauseButton = new QPushButton(this);     // Przycisk do pauzy/startu
    playPauseButton->setIcon(QIcon("../icons/play_button_proj.png"));
    playPauseButton->setIconSize(QSize(30, 30)); // Ikona startowa: play

    forwardButton = new QPushButton("⏩", this);  // Do przodu 10 sekund
    playlistButton = new QPushButton("📂", this); // Pokaż/ukryj playlistę
    prevTrackButton = new QPushButton("⏮", this); // Poprzedni utwór
    nextTrackButton = new QPushButton("⏭", this); // Następny utwór

    // Wszystkie przyciski są "płaskie", bez ramki

    prevTrackButton->setFlat(true);
    nextTrackButton->setFlat(true);
    rewindButton->setFlat(true);
    playPauseButton->setFlat(true);
    forwardButton->setFlat(true);
    playlistButton->setFlat(true);

    QLabel *soundIcon = new QLabel(this);
    QPixmap soundPixmap("../icons/volume_icon.png"); 
    soundIcon->setPixmap(soundPixmap.scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // === Suwak postępu odtwarzania (czas wideo/audio) ===
    progressSlider = new QSlider(Qt::Horizontal, this);
    // Tworzymy poziomy suwak, który będzie pokazywał bieżący czas odtwarzania
    // oraz pozwalał użytkownikowi przewinąć do wybranego miejsca w pliku

    // === Suwak głośności ===
    volumeSlider = new QSlider(Qt::Horizontal, this);
    // Tworzymy poziomy suwak służący do regulacji głośności
    volumeSlider->setRange(0, 100);    // Zakres od 0 (cisza) do 100 (maksymalna głośność)
    volumeSlider->setValue(100);       // Domyślna wartość to 100%

    // === Wyświetlacz czasu odtwarzania ===
    timeDisplay = new QLabel("00:00:00 / 00:00:00", this);
    // Etykieta tekstowa, która pokazuje aktualny czas odtwarzania i całkowity czas
    timeDisplay->setAlignment(Qt::AlignRight);  // Wyrównanie tekstu do prawej
    timeDisplay->setStyleSheet("font-family: monospace; font-size: 14px;");
    // Ustawiamy czcionkę monospace (stałej szerokości) dla czytelności

    // === Konfiguracja systemu audio ===
    QAudioDevice device = QMediaDevices::defaultAudioOutput();
    // Pobieramy domyślne urządzenie wyjściowe audio (np. głośniki)
    audioOutput = new QAudioOutput(device, this);
    // Tworzymy obiekt audio związany z tym urządzeniem
    audioOutput->setVolume(1.0); // Głośność ustawiona na 100% (1.0)

    // === Główny obiekt odtwarzacza multimediów ===
    mediaPlayer = new QMediaPlayer(this);
    // Inicjalizujemy QMediaPlayer — główny komponent do odtwarzania wideo/audio

    // Przypisujemy widżet wideo do odtwarzacza (jeśli plik zawiera obraz)
    mediaPlayer->setVideoOutput(videoWidget);

    // Przypisujemy wyjście dźwięku (głośniki/słuchawki) do odtwarzacza
    mediaPlayer->setAudioOutput(audioOutput);


    // Panel boczny z playlistą (na początku ukryty)
    playlistPanel = new QFrame(this);
    playlistPanel->setFrameShape(QFrame::StyledPanel);
    playlistPanel->setStyleSheet("background-color: #10152A; color: white;");
    playlistPanel->setVisible(false);

    // Layout wewnętrzny playlisty
    QVBoxLayout *playlistLayout = new QVBoxLayout(playlistPanel);
    QLabel *playlistTitle = new QLabel("Playlist", playlistPanel);
    playlistTitle->setStyleSheet("font-weight: bold; font-size: 16px;");

    // Przyciski dodawania i usuwania plików z playlisty
    addToPlaylistButton = new QPushButton("➕", playlistPanel);
    addToPlaylistButton->setFixedSize(24, 24);
    addToPlaylistButton->setFlat(true);

    removeFromPlaylistButton = new QPushButton("➖", playlistPanel);
    removeFromPlaylistButton->setFixedSize(24, 24);
    removeFromPlaylistButton->setFlat(true);

    // Przycisk powrotu do menu głównego
    QPushButton *backButton = new QPushButton("Back to Menu", this);
    backButton->setFixedSize(100, 30);
    connect(backButton, &QPushButton::clicked, this, [=]() {
        emit backToMenuRequested();  // Sygnał do MainWindow
    });

    // Pasek nagłówka playlisty: tytuł + przyciski
    QHBoxLayout *playlistHeader = new QHBoxLayout();
    playlistHeader->addWidget(playlistTitle);
    playlistHeader->addStretch();
    playlistHeader->addWidget(removeFromPlaylistButton);
    playlistHeader->addWidget(addToPlaylistButton);

    // Lista elementów multimedialnych
    playlistWidget = new QListWidget(playlistPanel);
    playlistWidget->setStyleSheet("background-color: #0A0F22; color: white;");

    // Umieszczenie nagłówka i listy w layoucie playlisty
    playlistLayout->addLayout(playlistHeader);
    playlistLayout->addWidget(playlistWidget);
    playlistPanel->setFixedWidth(300); // szerokość panelu

    // Układ główny interfejsu

    // Główne pionowe ułożenie
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Układ poziomy: wideo + playlista
    QHBoxLayout *videoLayout = new QHBoxLayout();
    mediaStack = new QStackedLayout();  // Stos: obrazek lub wideo
    mediaStack->addWidget(videoWidget);
    mediaStack->addWidget(imageLabel);
    videoLayout->addLayout(mediaStack);
    videoLayout->addWidget(playlistPanel);

    // Suwak postępu
    QHBoxLayout *progressLayout = new QHBoxLayout();
    progressLayout->addWidget(progressSlider);

    // Czas + przycisk playlisty
    QHBoxLayout *timeLayout = new QHBoxLayout();
    timeLayout->addWidget(timeDisplay);
    timeLayout->addStretch();
    timeLayout->addWidget(playlistButton);

    // Panel sterowania: przyciski
    QHBoxLayout *controlsLayout = new QHBoxLayout();
    controlsLayout->addStretch();
    controlsLayout->addWidget(prevTrackButton);
    controlsLayout->addWidget(rewindButton);
    controlsLayout->addWidget(playPauseButton);
    controlsLayout->addWidget(forwardButton);
    controlsLayout->addWidget(nextTrackButton);
    controlsLayout->addStretch();
    controlsLayout->addWidget(soundIcon);
    controlsLayout->addSpacing(5);
    controlsLayout->addWidget(volumeSlider);
    controlsLayout->addWidget(playlistButton);

    // Przycisk powrotu na dole
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->addWidget(backButton, 0, Qt::AlignLeft);
    bottomLayout->addStretch();

    // Dodanie layoutów do głównego
    mainLayout->addLayout(videoLayout, 9);
    mainLayout->addLayout(progressLayout);
    mainLayout->addLayout(timeLayout);
    mainLayout->addLayout(controlsLayout);
    mainLayout->addLayout(bottomLayout);

    // Finalizacja
    setLayout(mainLayout);
    showMaximized();
    setWindowTitle("Qt Media Player");


        // === Połączenia przycisków sterujących ===
    connect(playPauseButton, &QPushButton::clicked, this, &MediaPlayer::togglePlayPause);  // Play/Pause
    connect(rewindButton, &QPushButton::clicked, this, &MediaPlayer::rewind);              // Cofnij 10s
    connect(forwardButton, &QPushButton::clicked, this, &MediaPlayer::fastForward);        // Do przodu 10s

    // === Zmiana głośności na podstawie suwaka ===
    connect(volumeSlider, &QSlider::valueChanged, [=](int value) {
        audioOutput->setVolume(value / 100.0);
    });

    // === Przesuwanie odtwarzania przez suwak postępu ===
    connect(progressSlider, &QSlider::sliderMoved, [=](int value) {
        mediaPlayer->setPosition(value * 1000); // sekundy → ms
    });

    // === Aktualizacje suwaka i zegara podczas odtwarzania ===
    connect(mediaPlayer, &QMediaPlayer::positionChanged, this, &MediaPlayer::updatePosition);
    connect(mediaPlayer, &QMediaPlayer::durationChanged, this, &MediaPlayer::updateDuration);

    // === Pokaż/ukryj playlistę po kliknięciu przycisku 📂 ===
    connect(playlistButton, &QPushButton::clicked, [=]() {
        isPlaylistVisible = !isPlaylistVisible;
        playlistPanel->setVisible(isPlaylistVisible);
    });

    // === Dodanie pliku do playlisty (z okna dialogowego) ===
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

    // === Usuwanie zaznaczonego pliku z playlisty ===
    connect(removeFromPlaylistButton, &QPushButton::clicked, [=]() {
        QListWidgetItem *selectedItem = playlistWidget->currentItem();
        if (selectedItem) {
            int row = playlistWidget->row(selectedItem);
            delete playlistWidget->takeItem(row);
            // Obsługa sytuacji gdy usunięto aktualnie odtwarzany plik
            if (row == currentPlaylistIndex) {
                mediaPlayer->stop();
                playPauseButton->setIcon(QIcon("../icons/play_button_proj.png"));
                currentPlaylistIndex = -1;
            } else if (row < currentPlaylistIndex) {
                currentPlaylistIndex--;
            }
        }
    });

    // === Przejście do poprzedniego/następnego utworu ===
    connect(prevTrackButton, &QPushButton::clicked, this, &MediaPlayer::previousTrack);
    connect(nextTrackButton, &QPushButton::clicked, this, &MediaPlayer::nextTrack);

    // === Odtwórz plik po kliknięciu na liście ===
    connect(playlistWidget, &QListWidget::itemClicked, [=](QListWidgetItem *item) {
        int index = playlistWidget->row(item);
        playItemAtIndex(index);
    });
}

MediaPlayer::~MediaPlayer() {}

// === Otwieranie pliku multimedialnego przez okno dialogowe ===
void MediaPlayer::openFile() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open Media File",
                                                    QDir::homePath(),
                                                    "Media Files (*.mp3 *.mp4 *.wav *.avi *.mkv)");
    if (!fileName.isEmpty()) {
        mediaPlayer->setSource(QUrl::fromLocalFile(fileName)); // ustawienie źródła
        mediaPlayer->play();                                   // rozpoczęcie odtwarzania
        updateMediaDisplay();                                  // pokaż obraz/wideo
        playPauseButton->setIcon(QIcon("../icons/stop_button_proj.png"));
    }
}


// === Odtwórz plik z playlisty według indeksu ===
void MediaPlayer::playItemAtIndex(int index) {
    if (index >= 0 && index < playlistWidget->count()) {
        currentPlaylistIndex = index;
        playlistWidget->setCurrentRow(currentPlaylistIndex);

        QString filePath = playlistWidget->item(index)->data(Qt::UserRole).toString();
        mediaPlayer->setSource(QUrl::fromLocalFile(filePath));

        // Odśwież widok i rozpocznij odtwarzanie z małym opóźnieniem
        QTimer::singleShot(150, this, [=]() { updateMediaDisplay(); });
        QTimer::singleShot(100, this, [=]() {
            mediaPlayer->setPosition(100);
            mediaPlayer->play();
        });

        playPauseButton->setIcon(QIcon("../icons/stop_button_proj.png"));
    }
}

// === Przewijanie do przodu o 10 sekund ===
void MediaPlayer::fastForward() {
    mediaPlayer->setPosition(mediaPlayer->position() + 10000);
}

// === Cofanie o 10 sekund ===
void MediaPlayer::rewind() {
    mediaPlayer->setPosition(mediaPlayer->position() - 10000);
}

// === Zwiększanie głośności o 5 ===
void MediaPlayer::increaseVolume() {
    int value = volumeSlider->value();
    if (value < 100) {
        value += 5;
        volumeSlider->setValue(qMin(value, 100));
    }
}

// === Zmniejszanie głośności o 5 ===
void MediaPlayer::decreaseVolume() {
    int value = volumeSlider->value();
    if (value > 0) {
        value -= 5;
        volumeSlider->setValue(qMax(value, 0));
    }
}

// === Przełączanie widoku — wideo lub obrazek muzyczny ===
void MediaPlayer::updateMediaDisplay() {
    if (mediaPlayer->hasVideo()) {
        mediaStack->setCurrentWidget(videoWidget);
    } else {
        mediaStack->setCurrentWidget(imageLabel);
    }
}

// === Pauza / odtwarzanie + zmiana ikony ===
void MediaPlayer::togglePlayPause() {
    if (mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
        mediaPlayer->pause();
        playPauseButton->setIcon(QIcon("../icons/play_button_proj.png"));
    } else {
        mediaPlayer->play();
        playPauseButton->setIcon(QIcon("../icons/stop_button_proj.png"));
    }
}

// === Przejdź do następnego utworu na playliście ===
void MediaPlayer::nextTrack() {
    if (currentPlaylistIndex < playlistWidget->count() - 1)
        playItemAtIndex(currentPlaylistIndex + 1);
}

// === Przejdź do poprzedniego utworu na playliście ===
void MediaPlayer::previousTrack() {
    if (currentPlaylistIndex > 0)
        playItemAtIndex(currentPlaylistIndex - 1);
}

// === Ustaw maksymalny czas suwaka postępu ===
void MediaPlayer::updateDuration(qint64 duration) {
    totalDuration = duration;
    progressSlider->setMaximum(duration / 1000); // w sekundach
}

// === Aktualizacja pozycji suwaka i wyświetlacza czasu ===
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