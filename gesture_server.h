#pragma once 

#include <QObject>      
#include <QTcpServer>   // Klasa służąca do tworzenia serwera TCP
#include <QTcpSocket>   // Klasa reprezentująca połączenie klienta TCP
#include <QString>      // Klasa Qt dla łańcuchów tekstowych (stringów)

class GestureServer : public QObject {
    Q_OBJECT  // Makro Qt umożliwiające użycie sygnałów i slotów

public:
    // Konstruktor — może otrzymać rodzica w hierarchii Qt (domyślnie nullptr)
    explicit GestureServer(QObject *parent = nullptr);

signals:
    // Sygnał emitowany, gdy odebrano komendę z zewnętrznego źródła (np. Python)
    // Parametr: odebrana komenda jako tekst (np. "next_track", "zoom_in" itd.)
    void gestureReceived(const QString &command);

private:
    // Wskaźnik na serwer TCP, który nasłuchuje połączeń
    QTcpServer *server;
};