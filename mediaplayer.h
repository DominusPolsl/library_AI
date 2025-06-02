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

private:
    QMediaPlayer *mediaPlayer;
    QAudioOutput *audioOutput;
    QVideoWidget *videoWidget;

    QPushButton *openButton;
    QPushButton *playPauseButton;
    QLabel *statusLabel;

    QSlider *progressSlider;
    QSlider *volumeSlider;
    QLCDNumber *timeDisplay;
};