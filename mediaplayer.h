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

// Obsługuje odtwarzanie multimediów (audio + wideo), playlistę, regulację głośności, suwak czasu, widok wideo lub obrazka
class MediaPlayer : public QWidget {
    Q_OBJECT  // Umożliwia korzystanie z sygnałów i slotów Qt

public:
    // Konstruktor i destruktor
    MediaPlayer(QWidget *parent = nullptr);
    ~MediaPlayer();

    // Publiczne funkcje sterujące (używane przez gesty i przyciski)
    void togglePlayPause();   // Odtwórz/pauzuj
    void fastForward();       // Przewiń do przodu o 10 sek
    void rewind();            // Cofnij o 10 sek
    void nextTrack();         // Następny utwór z playlisty
    void previousTrack();     // Poprzedni utwór z playlisty
    void increaseVolume();    // Zwiększ głośność
    void decreaseVolume();    // Zmniejsz głośność

private slots:
    // Funkcje reagujące na działania użytkownika
    void openFile();                      // Otwórz plik z dysku
    void updatePosition(qint64 position); // Aktualizuj czas i suwak
    void updateDuration(qint64 duration); // Ustaw maksymalny czas suwaka
    void updateMediaDisplay();            // Przełącz widok między wideo a obrazkiem
    void playItemAtIndex(int index);      // Odtwórz element z playlisty

private:
    QMediaPlayer *mediaPlayer;        // Główne źródło mediów (obsługuje dźwięk i obraz)
    QAudioOutput *audioOutput;        // Wyjście dźwiękowe (dla głośności)
    QVideoWidget *videoWidget;        // Widżet do odtwarzania wideo

    QFrame *playlistPanel;            // Panel boczny z playlistą
    QStackedLayout *mediaStack;       // Przełącznik między wideo a obrazem (placeholder)
    QLabel *imageLabel;               // Obrazek używany gdy plik to tylko audio
    QListWidget *playlistWidget;      // Lista utworów/plików multimedialnych
    QPushButton *addToPlaylistButton;       // Dodaj do playlisty
    QPushButton *removeFromPlaylistButton;  // Usuń z playlisty
    bool isPlaylistVisible = false;         // Czy panel playlisty jest widoczny?

    QPushButton *openButton;          // Otwórz plik
    QPushButton *playPauseButton;     // Play/pause
    QPushButton *rewindButton;        // Cofnij 10s
    QPushButton *forwardButton;       // Do przodu 10s
    QPushButton *playlistButton;      // Pokaż/ukryj playlistę
    QPushButton *prevTrackButton;     // Poprzedni utwór
    QPushButton *nextTrackButton;     // Następny utwór
    int currentPlaylistIndex = -1;    // Numer aktualnie odtwarzanego elementu w liście

    QSlider *progressSlider;          // Suwak postępu odtwarzania
    QSlider *volumeSlider;            // Suwak głośności
    QLabel *timeDisplay;              // Tekstowy wyświetlacz czasu (hh:mm:ss)
    qint64 totalDuration = 0;         // Całkowita długość aktualnego pliku (w ms)

signals:
    // Sygnał informujący, że użytkownik chce wrócić do menu głównego
    void backToMenuRequested();
};
