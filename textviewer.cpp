#include "textviewer.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QDir>

TextViewer::TextViewer(QWidget *parent)
    : QWidget(parent), currentPage(0)
{
    pdfDoc = new QPdfDocument(this);
    pageLabel = new QLabel(this);
    pageLabel->setAlignment(Qt::AlignCenter);
    pageLabel->setMinimumSize(600, 700);

    openButton = new QPushButton("📁 File", this);
    prevButton = new QPushButton("←", this);
    nextButton = new QPushButton("→", this);

    QVBoxLayout *layout = new QVBoxLayout(this);

    // верхній ряд: кнопка "Назад"
    QPushButton *backButton = new QPushButton("🔙 Back to Menu", this);
    backButton->setFixedSize(120, 30);
    layout->addWidget(backButton, 0, Qt::AlignLeft);
    connect(backButton, &QPushButton::clicked, this, [this]() {
        emit backToMenuRequested();  // замість close()
    });

    // другий ряд: File + стрілки
    QHBoxLayout *controls = new QHBoxLayout();
    controls->addWidget(openButton);
    controls->addStretch();
    controls->addWidget(prevButton);
    controls->addWidget(nextButton);
    layout->addLayout(controls);

    // третій ряд: PDF рендер
    layout->addWidget(pageLabel);

    // встановити layout
    setLayout(layout);

    connect(openButton, &QPushButton::clicked, this, &TextViewer::openPdf);
    connect(prevButton, &QPushButton::clicked, this, &TextViewer::prevPage);
    connect(nextButton, &QPushButton::clicked, this, &TextViewer::nextPage);
}

void TextViewer::openPdf() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open PDF", QDir::homePath(), "PDF files (*.pdf)");
    if (fileName.isEmpty()) return;

    QPdfDocument::Error err = pdfDoc->load(fileName);
    if (err != QPdfDocument::Error::None) {
        QMessageBox::critical(this, "Error", "Failed to load PDF file.");
        return;
    }

    currentPage = 0;
    showPage();
}

void TextViewer::showPage() {
    if (!pdfDoc || pdfDoc->pageCount() <= 0) return;

    QSize imageSize(800, 1000);
    QPdfDocumentRenderOptions options;
    QImage image = pdfDoc->render(currentPage, imageSize, options);

    if (image.isNull()) {
        QMessageBox::warning(this, "Error", "Unable to render PDF page.");
        return;
    }

    pageLabel->setPixmap(QPixmap::fromImage(image));
}

void TextViewer::nextPage() {
    if (currentPage + 1 < pdfDoc->pageCount()) {
        ++currentPage;
        showPage();
    }
}

void TextViewer::prevPage() {
    if (currentPage > 0) {
        --currentPage;
        showPage();
    }
}
