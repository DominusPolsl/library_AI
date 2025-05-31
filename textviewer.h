#pragma once
#include <QMainWindow>

class QTextEdit;
class QListWidget;

class TextViewer : public QMainWindow
{
    Q_OBJECT

public:
    TextViewer(QWidget *parent = nullptr);

private slots:
    void openFile();
    void loadSelectedFile();
    void showContextMenu(const QPoint &pos);

private:
    QTextEdit *textEdit;
    QListWidget *sidebar;
    void createMenu();
};