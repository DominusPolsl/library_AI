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
    void togglePlayPause();   
    void fastForward();       
    void rewind();           
    void nextTrack();         
    void previousTrack();     
    void increaseVolume();   
    void decreaseVolume();    

private:
    QMediaPlayer *mediaPlayer;        
    QAudioOutput *audioOutput;        
    QVideoWidget *videoWidget;        

    QFrame *playlistPanel;            
    QStackedLayout *mediaStack;       
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

    // Funkcje reagujące na działania użytkownika
    void openFile();                     
    void updatePosition(qint64 position); 
    void updateDuration(qint64 duration); 
    void updateMediaDisplay();            
    void playItemAtIndex(int index);      

signals:
    // Sygnał informujący, że użytkownik chce wrócić do menu głównego
    void backToMenuRequested();
};
