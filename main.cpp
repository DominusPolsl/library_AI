#include <iostream>         
#include <QApplication>      
#include "mainwindow.h"      

int main(int argc, char *argv[]) {
    qputenv("QT_MEDIA_BACKEND", "ffmpeg");  // Wymusza użycie FFmpeg jako backendu multimediów
    qputenv("QT_FFMPEG_HW", "0");           // Wyłącza sprzętowe przyspieszenie dekodowania

    QApplication app(argc, argv);           // Tworzenie aplikacji

    MainWindow window;                      // Tworzenie głownego okna
    window.setWindowTitle("Multimedia Library Menu");  
    window.showMaximized();                            // Pokazuje okno w trybie pełnoekranowym

    return app.exec();                      // Uruchamia główną pętlę zdarzeń aplikacji
}
