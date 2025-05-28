#ifndef TEXTVIEWER_H
#define TEXTVIEWER_H

#include <QMainWindow>
#include <QPushButton>
#include <QtPdf/QPdfDocument>
#include <QtPdfWidgets/QPdfView>


class QTextEdit;
class QListWidget;

class TextViewer : public QMainWindow
{
    Q_OBJECT

public:
    TextViewer(const QStringList &initialFiles = {}, QWidget *parent = nullptr);

private slots:
    void openFile();
    void loadSelectedFile();
    void showContextMenu(const QPoint &pos);

private:
    QTextEdit *textEdit;
    QListWidget *sidebar;
    QPushButton *returnButton;
    QPdfDocument *pdfDocument;
    QPdfView *pdfView;
    void createMenu();
signals:
    void returnToMainMenuClicked();
    void fileAdded(const QString &filePath);
    void fileRemoved(const QString &filePath);
};

#endif // TEXTVIEWER_H