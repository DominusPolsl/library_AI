#ifndef TEXTVIEWER_H
#define TEXTVIEWER_H

#include <QMainWindow>
#include <QPushButton>
#include <QStackedWidget>
#include <QVector>
#include <QString>
#include <QTextBrowser>
#include <QLabel>

#include <QPdfDocument>
#include <QPdfDocumentRenderOptions>
#include <QScrollArea>
#include <QPdfPageRenderer>


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
    void loadPdf(const QString &fileName);
    void showContextMenu(const QPoint &pos);
    void nextPage();
    void previousPage();
    void switchFont();
    void increaseFontSize();
    void decreaseFontSize();
    void showMainMenu();

private:
    void createMenu();
    void paginateText(const QString &text);
    void updatePages();
    bool eventFilter(QObject *obj, QEvent *event);
    void renderPdfPage(int pageIndex);
    
    QTextBrowser *leftPage;
    QTextBrowser *rightPage;
    QListWidget *sidebar;
    QPushButton *returnButton;
    QPushButton *increaseFontButton;
    QPushButton *decreaseFontButton;
    QStackedWidget *pageStack;

    QPdfDocument *pdfDocument = nullptr;
    QLabel *pdfLabel = nullptr;
    QScrollArea *pdfScrollArea  = nullptr;
    int currentPdfPage = 0;
    int totalPdfPages = 0;
    bool isPdf = false;

    QPushButton *nextButton;
    QPushButton *prevButton;
    QPushButton *switchFontButton;

    QStringList fontList;
    int currentFontIndex = 0;

    QVector<QString> pages;
    int currentPageIndex = 0;
    int fontSize = 12;
    
signals:
    void returnToMainMenuClicked();
    void fileAdded(const QString &filePath);
    void fileRemoved(const QString &filePath);
    
};

#endif // TEXTVIEWER_H;