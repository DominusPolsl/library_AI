#include <iostream>
#include <QApplication>
#include "mainwindow.h"
using namespace std;
double func(double, double);

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    func(3.14, 5.43);
    MainWindow window;
    window.setWindowTitle("Multimedia Library Menu");
    window.resize(600, 400);
    window.show();

    return app.exec();
}
double func(double a, double b){
    double res = 1;
    for (int i = 0; i < b; i++){
        res = res * a;
    }
    return res;
}

