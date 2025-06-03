#pragma once

#include <QWidget>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVideoWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QLabel>
#include <QMediaDevices>
#include <QSlider>
#include <QLCDNumber>
#include <QListWidget>
#include <QFrame>
#include <QStackedLayout>

class MediaPlayer : public QWidget {
    Q_OBJECT

public:
    MediaPlayer(QWidget *parent = nullptr);
    ~MediaPlayer();

private slots:
    void openFile();
    void togglePlayPause();
    void updatePosition(qint64 position);
    void updateDuration(qint64 duration);
    void updateMediaDisplay();

private:
    QMediaPlayer *mediaPlayer;
    QAudioOutput *audioOutput;
    QVideoWidget *videoWidget;
    QFrame *playlistPanel;
    QStackedLayout *mediaStack; //zdjęcie bez wideo i bez zdjęcia kiedy jest odtwarzane wideo
    QLabel *imageLabel;
    QListWidget *playlistWidget;
    QPushButton *addToPlaylistButton;
    bool isPlaylistVisible = false;

    QPushButton *openButton;
    QPushButton *playPauseButton;
    QLabel *statusLabel;
    QPushButton *rewindButton;
    QPushButton *forwardButton;
    QPushButton *playlistButton;
    QPushButton *prevTrackButton;
    QPushButton *nextTrackButton;
    int currentPlaylistIndex = -1;

    QSlider *progressSlider;
    QSlider *volumeSlider;
    QLabel *timeDisplay;
    qint64 totalDuration = 0;
};