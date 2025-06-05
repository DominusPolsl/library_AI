// gesture_server.h
#pragma once

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QString>

class GestureServer : public QObject {
    Q_OBJECT

public:
    explicit GestureServer(QObject *parent = nullptr);

signals:
    // Сигнал буде емісійований при отриманні команди з Python-скрипту
    void gestureReceived(const QString &command);

private:
    QTcpServer *server;
};
