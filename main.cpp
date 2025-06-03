#include <iostream>
#include <QApplication>
#include "mainwindow.h"

using namespace std;

int main(int argc, char *argv[]) {
    qputenv("QT_MEDIA_BACKEND", "ffmpeg"); // переконайся, що ffmpeg використовується
    qputenv("QT_FFMPEG_HW", "0"); 
    QApplication app(argc, argv);

    MainWindow window;
    window.setWindowTitle("Multimedia Library Menu");
    window.showMaximized();

    return app.exec();
}