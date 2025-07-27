#include "gesture_server.h"     
#include <QDebug>               

// Inicjalizuje serwer TCP i ustawia nasłuch na porcie 9999 (tylko lokalnie)
GestureServer::GestureServer(QObject *parent) : QObject(parent) {

    // Tworzenie obiektu serwera TCP i przypisanie go do obecnego obiektu jako rodzica
    server = new QTcpServer(this);

    // Obsługa nowego połączenia TCP od klienta (jest to niezbędne dla gesture_client.py)
    connect(server, &QTcpServer::newConnection, this, [=]() {
        // Gdy pojawia się nowe połączenie — pobieramy klienta
        QTcpSocket *client = server->nextPendingConnection();

        // Reakcja na zdarzenie: klient przesyła dane (komendę gestu)
        connect(client, &QTcpSocket::readyRead, this, [=]() {
            // Odczytanie wszystkich danych jako QString (UTF-8) i usunięcie białych znaków
            QString command = QString::fromUtf8(client->readAll()).trimmed();

            // Wypisanie komendy w konsoli (debug)
            qDebug() << "Received gesture command:" << command;

            // Wysłanie sygnału do innych komponentów Qt (np. MainWindow), że otrzymano gest
            emit gestureReceived(command);

            // Rozłączenie klienta po odebraniu komendy
            client->disconnectFromHost();
        });
    });

    // Uruchomienie nasłuchu na porcie 9999 tylko na lokalnym hoście
    if (!server->listen(QHostAddress::LocalHost, 9999)) {
        qDebug() << "Gesture server failed to start:" << server->errorString();
    } else {
        qDebug() << "Gesture server is listening on port 9999...";
    }
}
