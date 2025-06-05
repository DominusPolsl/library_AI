// gesture_server.cpp
#include "gesture_server.h"
#include <QTcpSocket>
#include <QDebug>

GestureServer::GestureServer(QObject *parent) : QObject(parent) {
    server = new QTcpServer(this);

    connect(server, &QTcpServer::newConnection, this, [=]() {
        QTcpSocket *client = server->nextPendingConnection();

        connect(client, &QTcpSocket::readyRead, this, [=]() {
            QString command = QString::fromUtf8(client->readAll()).trimmed();
            qDebug() << "Received gesture command:" << command;

            emit gestureReceived(command);
            client->disconnectFromHost();
        });
    });

    if (!server->listen(QHostAddress::LocalHost, 9999)) {
        qDebug() << "Gesture server failed to start:" << server->errorString();
    } else {
        qDebug() << "Gesture server is listening on port 9999...";
    }
}
