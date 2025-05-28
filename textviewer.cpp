#include "textviewer.h"
#include <QTextEdit>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QSplitter>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>


TextViewer::TextViewer(const QStringList &initialFiles, QWidget *parent)
    : QMainWindow(parent)
{
    QSplitter *splitter = new QSplitter(this);
    splitter->setHandleWidth(0);

    // Przycisk powrotu do menu głównego
    returnButton = new QPushButton("⬅️ Powrót do menu głównego");
    connect(returnButton, &QPushButton::clicked, this, &TextViewer::returnToMainMenuClicked);

    // Dla lepszej organizacji, umieszczamy przycisk i splitter w centralnym kontenerze i ustawiamy układ pionowy
    QWidget *container = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(container);
    mainLayout->addWidget(returnButton);
    mainLayout->addWidget(splitter);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Sidebar
    sidebar = new QListWidget(splitter);
    sidebar->setFixedWidth(400);

    sidebar->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(sidebar, &QListWidget::customContextMenuRequested, this, &TextViewer::showContextMenu);

    connect(sidebar, &QListWidget::itemClicked, this, &TextViewer::loadSelectedFile);

    // Czytnik
    textEdit = new QTextEdit(splitter);
    textEdit->setReadOnly(true);
    textEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    //pdfView = new QPdfView(splitter); // dodaj to

    pdfDocument = new QPdfDocument(this);
    pdfView = new QPdfView(splitter);
    pdfView->setDocument(pdfDocument);
    pdfView->hide();

    setCentralWidget(container);
    createMenu();

    for (const QString &filePath : initialFiles) 
    {
        QFileInfo info(filePath);
        QString name = info.fileName();

        QListWidgetItem *item = new QListWidgetItem(name);
        item->setData(Qt::UserRole, filePath);
        sidebar->addItem(item);
    }
}

void TextViewer::createMenu()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&Dodaj Plik"));

    QAction *openAction = new QAction(tr("&Otwórz..."), this);
    connect(openAction, &QAction::triggered, this, &TextViewer::openFile);

    fileMenu->addAction(openAction);
}

void TextViewer::openFile()
{
    /* QString fileName = QFileDialog::getOpenFileName(this, tr("Otwórz książkę"), "", 
    tr("Pliki tekstowe (*.txt);;Wszystkie pliki (*)"));

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) 
    {
        QMessageBox::warning(this, tr("Błąd"),
            tr("Nie można otworzyć pliku:\n%1").arg(file.errorString()));
        return;
    }

    QTextStream in(&file);
    textEdit->setPlainText(in.readAll());

    QString displayName = QFileInfo(fileName).fileName();

    QList<QListWidgetItem*> items = sidebar->findItems(displayName, Qt::MatchExactly);
    if (items.isEmpty()) 
    {
        QListWidgetItem *item = new QListWidgetItem(displayName);
        item->setData(Qt::UserRole, fileName);
        sidebar->addItem(item);
        emit fileAdded(fileName);
    } */

    QString fileName = QFileDialog::getOpenFileName(this, tr("Otwórz książkę"), "",
    tr("Pliki tekstowe (*.txt);;Pliki PDF (*.pdf);;Wszystkie pliki (*)"));

    if (fileName.isEmpty()) return;

    QString ext = QFileInfo(fileName).suffix().toLower();

    if (ext == "txt") {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::warning(this, tr("Błąd"), tr("Nie można otworzyć pliku:\n%1").arg(file.errorString()));
            return;
        }
        QTextStream in(&file);
        textEdit->setPlainText(in.readAll());
        textEdit->show();
        pdfView->hide();
    }
   else if (ext == "pdf") {
    // pobieramy wynik jako QPdfDocument::Error
    QPdfDocument::Error error = pdfDocument->load(fileName);
    // porównujemy z Error::None
    if (error != QPdfDocument::Error::None) {
        QMessageBox::warning(this, tr("Błąd"),
            tr("Nie można załadować pliku PDF."));
        return;
    }

    // przekazujemy dokument do widoku
    pdfView->setDocument(pdfDocument);
    pdfView->setZoomMode(QPdfView::ZoomMode::FitInView);
    pdfView->show();
    textEdit->hide();
    }


    else 
    {
        QMessageBox::warning(this, tr("Nieobsługiwany format"), tr("Tylko pliki .txt i .pdf są obsługiwane."));
        return;
    }

    // Dodanie do sidebaru, jak wcześniej:
    QString displayName = QFileInfo(fileName).fileName();
    QListWidgetItem *item = new QListWidgetItem(displayName);
    item->setData(Qt::UserRole, fileName);
    sidebar->addItem(item);
    emit fileAdded(fileName);
}

void TextViewer::loadSelectedFile()
{
    QListWidgetItem *item = sidebar->currentItem();
    if (!item)
        return;
    QString filePath = item->data(Qt::UserRole).toString();
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Błąd"),
            tr("Nie można otworzyć pliku:\n%1").arg(file.errorString()));
        return;
    }

    QTextStream in(&file);
    textEdit->setPlainText(in.readAll());
}

void TextViewer::showContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = sidebar->itemAt(pos);
    if (!item)
        return;

    QMenu contextMenu;
    QAction *removeAction = contextMenu.addAction("Usuń z listy");

    QAction *selectedAction = contextMenu.exec(sidebar->viewport()->mapToGlobal(pos));
    if (selectedAction == removeAction) {
        // Pobierz ścieżkę z klikniętego elementu
        QString removedFilePath = item->data(Qt::UserRole).toString();

        // Sprawdź, czy to aktualnie otwarty plik
        QString currentText = textEdit->toPlainText();
        QFile file(removedFilePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString content = in.readAll();
            if (content == currentText) {
                textEdit->clear(); // Wyczyszczenie widoku
            }
        }

        delete sidebar->takeItem(sidebar->row(item)); // Usunięcie z listy
        emit fileRemoved(removedFilePath);
    }
}