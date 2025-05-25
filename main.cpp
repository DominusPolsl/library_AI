#include <iostream>
#include <QApplication>
#include "mainwindow.h"
using namespace std;

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MainWindow window;
    window.setWindowTitle("Multimedia Library Menu");
    window.resize(600, 400);
    window.show();

    return app.exec();
}
