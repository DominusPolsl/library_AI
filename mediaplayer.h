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
    void togglePlayPause();
    void fastForward();
    void rewind();
    void nextTrack();
    void previousTrack();
    void increaseVolume();
    void decreaseVolume();

private slots:
    void openFile();
    void updatePosition(qint64 position);
    void updateDuration(qint64 duration);
    void updateMediaDisplay();
    void playItemAtIndex(int index);

private:
    QMediaPlayer *mediaPlayer;
    QAudioOutput *audioOutput;
    QVideoWidget *videoWidget;
    QFrame *playlistPanel;
    QStackedLayout *mediaStack; //zdjęcie bez wideo i bez zdjęcia kiedy jest odtwarzane wideo
    QLabel *imageLabel;
    QListWidget *playlistWidget;
    QPushButton *addToPlaylistButton;
    QPushButton *removeFromPlaylistButton;
    bool isPlaylistVisible = false;

    QPushButton *openButton;
    QPushButton *playPauseButton;
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

signals:
    void backToMenuRequested();

};