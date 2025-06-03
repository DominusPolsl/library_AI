#pragma once
#include <QWidget>
#include <QPdfDocument>
#include <QPdfDocumentRenderOptions>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

class TextViewer : public QWidget {
    Q_OBJECT

public:
    explicit TextViewer(QWidget *parent = nullptr);

private slots:
    void openPdf();
    void nextPage();
    void prevPage();

private:
    QPdfDocument *pdfDoc;
    QLabel *pageLabel;
    QPushButton *openButton;
    QPushButton *nextButton;
    QPushButton *prevButton;
    int currentPage;
    void showPage();

signals:
    void backToMenuRequested();    

}; 