#include "textviewer.h"
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMenuBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QMessageBox>
#include <QListWidget>
#include <QMenu>
#include <QSettings>
#include <QEvent>
#include <QWheelEvent>

TextViewer::TextViewer(const QStringList &initialFiles, QWidget *parent): QMainWindow(parent) 
{
    pdfDocument = new QPdfDocument(this);

    pdfLabel = new QLabel();
    pdfLabel->setAlignment(Qt::AlignCenter);
    pdfLabel->setBackgroundRole(QPalette::Base);
    pdfLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    pdfLabel->setScaledContents(true);

    pdfScrollArea = new QScrollArea();
    pdfScrollArea->setBackgroundRole(QPalette::Dark);
    pdfScrollArea->setWidget(pdfLabel);

    pdfScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    pdfScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    pdfScrollArea->setVisible(false);

    // 1) Główny widget (central) i layout pionowy
    QWidget *central = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout();
    central->setLayout(layout);

    // 2) Sidebar po lewej (lista plików)
    QSplitter *splitter = new QSplitter(this);
    sidebar = new QListWidget();
    sidebar->setFixedWidth(300);
    sidebar->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(sidebar, &QListWidget::itemClicked,
            this, &TextViewer::loadSelectedFile);
    connect(sidebar, &QListWidget::customContextMenuRequested,
            this, &TextViewer::showContextMenu);
    splitter->addWidget(sidebar);

    // 3) Widok tekstowy – dwa QTextBrowser obok siebie
    QHBoxLayout *pagesLayout = new QHBoxLayout();
    leftPage = new QTextBrowser();
    rightPage = new QTextBrowser();
    leftPage->installEventFilter(this);
    rightPage->installEventFilter(this);
    leftPage->setTextInteractionFlags(Qt::NoTextInteraction);
    rightPage->setTextInteractionFlags(Qt::NoTextInteraction);
    pagesLayout->addWidget(leftPage);
    pagesLayout->addWidget(rightPage);

    QWidget *textWidget = new QWidget();
    textWidget->setLayout(pagesLayout);

    // 4) QStackedWidget: index 0 = textWidget, index 1 = pdfScrollArea
    pageStack = new QStackedWidget();
    pageStack->addWidget(textWidget);       // indeks 0 – widok .txt
    pageStack->addWidget(pdfScrollArea);    // indeks 1 – widok PDF
    pageStack->setCurrentIndex(0);          // start → widok tekstowy

    splitter->addWidget(pageStack);

    // 5) Przyciski nawigacji i zmiany czcionki
    prevButton = new QPushButton("⬅️ Poprzednia strona");
    nextButton = new QPushButton("➡️ Następna strona");
    connect(prevButton, &QPushButton::clicked,
            this, &TextViewer::previousPage);
    connect(nextButton, &QPushButton::clicked,
            this, &TextViewer::nextPage);

    switchFontButton   = new QPushButton("🔁 Zmień czcionkę");
    increaseFontButton = new QPushButton("➕");
    decreaseFontButton = new QPushButton("➖");
    connect(switchFontButton,  &QPushButton::clicked,
            this, &TextViewer::switchFont);
    connect(increaseFontButton,&QPushButton::clicked,
            this, &TextViewer::increaseFontSize);
    connect(decreaseFontButton,&QPushButton::clicked,
            this, &TextViewer::decreaseFontSize);

    QHBoxLayout *navLayout = new QHBoxLayout();
    navLayout->addWidget(prevButton);
    navLayout->addWidget(decreaseFontButton);
    navLayout->addWidget(increaseFontButton);
    navLayout->addWidget(switchFontButton);
    navLayout->addWidget(nextButton);

    // 6) Dodaj splitter i pasek przycisków do layoutu głównego
    layout->addWidget(splitter);
    layout->addLayout(navLayout);

    setCentralWidget(central);
    createMenu();

    // 7) Wczytywanie „recentFiles” z QSettings
    QSettings settings("MyApp", "TextViewer");
    QStringList savedFiles = settings.value("recentFiles").toStringList();
    for (const QString &filePath : savedFiles) {
        QFileInfo info(filePath);
        QString name = info.fileName();
        QListWidgetItem *item = new QListWidgetItem(name);
        item->setData(Qt::UserRole, filePath);
        sidebar->addItem(item);
    }

    // 8) Ustawienia początkowe dla tekstu
    fontList = {"Arial", "Times New Roman", "Courier New"};
    currentFontIndex = 0;
    fontSize = 12;
    currentPageIndex = 0;

    // 9) Ustawiamy, że na początku widok PDF jest nieaktywny
    isPdf = false;
    currentPdfPage = 0;
    totalPdfPages = 0;
}

void TextViewer::createMenu() {
    QMenu *fileMenu = menuBar()->addMenu(tr("&Plik"));
    QAction *openAction = new QAction(tr("&Otwórz..."), this);
    connect(openAction, &QAction::triggered, this, &TextViewer::openFile);
    fileMenu->addAction(openAction);

    QAction *returnAction = new QAction(tr("&Powrót"), this);
    connect(returnAction, &QAction::triggered,this, &TextViewer::returnToMainMenuClicked);
    menuBar()->addAction(returnAction);
}

void TextViewer::openFile() 
{
    QString filter = tr("Pliki tekstowe (*.txt);;Pliki PDF (*.pdf)");
    QString fileName = QFileDialog::getOpenFileName(this, tr("Otwórz plik"), "", filter);
    if (fileName.isEmpty()) return;

    for (int i = 0; i < sidebar->count(); ++i) 
    {
        QListWidgetItem *existingItem = sidebar->item(i);
        QString existingPath = existingItem->data(Qt::UserRole).toString();
        if (existingPath == fileName) {
            // Plik jest już na liście – ustawiamy go jako zaznaczony i ładujemy
            sidebar->setCurrentItem(existingItem);
            loadSelectedFile();
            return; // nie dodajemy duplikatu
        }
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) 
    {
        QMessageBox::warning(this, tr("Błąd"), tr("Nie można otworzyć pliku: %1").arg(file.errorString()));
        return;
    }

    QString ext = QFileInfo(fileName).suffix().toLower();
    if (ext == "pdf") 
    {
        // wywołujemy naszą funkcję
        loadPdf(fileName);
    } else 
    {
        QTextStream in(&file);
        QString content = in.readAll();
        paginateText(content);
        updatePages();

        QString displayName = QFileInfo(fileName).fileName();
        QListWidgetItem *item = new QListWidgetItem(displayName);
        item->setData(Qt::UserRole, fileName);
        sidebar->addItem(item);
        sidebar->setCurrentItem(item);

        isPdf = false;
        pageStack->setCurrentIndex(0);
    }

    QStringList savedFiles;
    for (int i = 0; i < sidebar->count(); ++i) 
    {
        savedFiles.append(sidebar->item(i)->data(Qt::UserRole).toString());
    }
    QSettings settings("MyApp", "TextViewer");
    settings.setValue("recentFiles", savedFiles);
}

void TextViewer::loadSelectedFile() {
    QListWidgetItem *item = sidebar->currentItem();
    if (!item) return;

    QString filePath = item->data(Qt::UserRole).toString();
    QString ext = QFileInfo(filePath).suffix().toLower();

    if (ext == "pdf") 
        loadPdf(filePath);
    else
    {
        isPdf = false;
        pageStack->setCurrentIndex(0);

        // 2) Włącz przyciski zmiany czcionki:
        switchFontButton->setEnabled(true);
        increaseFontButton->setEnabled(true);
        decreaseFontButton->setEnabled(true);

        // 3) Odczytaj zawartość pliku .txt i wywołaj paginację:
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::warning(this, tr("Błąd"),
                                 tr("Nie można otworzyć pliku:\n%1")
                                 .arg(file.errorString()));
            return;
        }
        QTextStream in(&file);
        QString content = in.readAll();
        paginateText(content);
        updatePages();

        // 4) Zadbaj, żeby przyciski nawigacyjne odpowiadały tekstowi:
        //    – w zależności od currentPageIndex / pages.size().
        prevButton->setEnabled(currentPageIndex >= 2);
        nextButton->setEnabled(currentPageIndex + 2 < pages.size());
    }
}

void TextViewer::showContextMenu(const QPoint &pos) {
    QListWidgetItem *item = sidebar->itemAt(pos);
    if (!item) return;

    QMenu contextMenu;
    QAction *removeAction = contextMenu.addAction("Usuń z listy");

    QAction *selectedAction = contextMenu.exec(sidebar->viewport()->mapToGlobal(pos));
    if (selectedAction == removeAction) {
        QString removedFilePath = item->data(Qt::UserRole).toString();

        // jeśli aktualnie wyświetlany plik to ten usuwany — wyczyść widok
        if (!pages.isEmpty() && sidebar->currentItem() == item) {
            leftPage->clear();
            rightPage->clear();
            pages.clear();
            currentPageIndex = 0;
        }

        delete sidebar->takeItem(sidebar->row(item));

        // zaktualizuj listę zapamiętanych plików
        QSettings settings("MyApp", "TextViewer");
        QStringList savedFiles;
        for (int i = 0; i < sidebar->count(); ++i) {
            savedFiles.append(sidebar->item(i)->data(Qt::UserRole).toString());
        }
        settings.setValue("recentFiles", savedFiles);
    }
}

void TextViewer::paginateText(const QString &text) {
    pages.clear();

    QFont font(fontList[currentFontIndex], fontSize);
    QFontMetrics metrics(font);

    // Rzeczywista wysokość linii
    int lineHeight = metrics.lineSpacing();

    // Oblicz liczbę linii, które mieszczą się na stronie
    int maxLines = leftPage->viewport()->height() / lineHeight;

    // Zakładamy stałą szerokość, ale możesz dodać zaawansowane łamanie słów
    int avgCharsPerLine = leftPage->viewport()->width() / metrics.averageCharWidth();

    int charsPerPage = maxLines * avgCharsPerLine;

    int pos = 0;
    while (pos < text.length()) {
        pages.append(text.mid(pos, charsPerPage));
        pos += charsPerPage;
    }

    currentPageIndex = 0;
}

void TextViewer::updatePages() {
    if (pages.isEmpty()) {
        leftPage->clear();
        rightPage->clear();
        return;
    }

    leftPage->setFont(QFont(fontList[currentFontIndex], fontSize));
    rightPage->setFont(QFont(fontList[currentFontIndex], fontSize));
    leftPage->setText(pages.value(currentPageIndex));
    rightPage->setText(pages.value(currentPageIndex + 1));

    prevButton->setEnabled(currentPageIndex > 0);
    nextButton->setEnabled(currentPageIndex + 2 < pages.size());
}

void TextViewer::nextPage() {
    if (isPdf) 
    {
        if (currentPdfPage + 1 < totalPdfPages) 
        {
            currentPdfPage++;
            renderPdfPage(currentPdfPage);
            prevButton->setEnabled(true);
            nextButton->setEnabled(currentPdfPage + 1 < totalPdfPages);
        }
    } 
    else 
    {
        // stary kod paginacji tekstu:
        if (currentPageIndex + 2 < pages.size()) 
        {
            currentPageIndex += 2;
            updatePages();
        }
    }
}

void TextViewer::previousPage() {
    if (isPdf) 
    {
        if (currentPdfPage - 1 >= 0) 
        {
            currentPdfPage--;
            renderPdfPage(currentPdfPage);
            nextButton->setEnabled(true);
            prevButton->setEnabled(currentPdfPage > 0);
        }
    } 
    else 
    {
        // stary kod paginacji tekstu:
        if (currentPageIndex - 2 >= 0) 
        {
            currentPageIndex -= 2;
            updatePages();
        }
    }
}

void TextViewer::switchFont() {
    currentFontIndex = (currentFontIndex + 1) % fontList.size();
    updatePages();
}

void TextViewer::increaseFontSize() {
    fontSize += 2;
    if (!pages.isEmpty())
    {
        paginateText(pages.join(""));
        updatePages();
    }
}

void TextViewer::decreaseFontSize() {
    if (fontSize > 6) {
        fontSize -= 2;
        if (!pages.isEmpty()) 
        {
            paginateText(pages.join(""));
            updatePages();
        }
    }
}

bool TextViewer::eventFilter(QObject *obj, QEvent *event) 
{
    if ((obj == leftPage || obj == rightPage) && event->type() == QEvent::Wheel) 
    {
        return true;  // zignoruj przewijanie
    }
    return QMainWindow::eventFilter(obj, event);
}

void TextViewer::renderPdfPage(int pageIndex)
{
    if (!pdfDocument || pageIndex < 0 || pageIndex >= totalPdfPages)
        return;

    // 1) Pobierz oryginalne wymiary strony w punktach (1 pt = 1/72 cala)
    QSizeF pagePts = pdfDocument->pagePointSize(pageIndex);
    // Jeśli nie udało się pobrać, przyjmijmy domyślnie prosty stosunek 1:1
    if (pagePts.isEmpty())
        pagePts = QSizeF(1.0, 1.0);

    qreal pageRatio = pagePts.width() / pagePts.height();  // stosunek szerokość/wysokość

    // 2) Pobierz rozmiar viewportu (prawego panelu, w pikselach)
    QSize viewportSize = pdfScrollArea->viewport()->size();
    if (viewportSize.width() <= 0 || viewportSize.height() <= 0) {
        // Gdy viewport jest jeszcze niezainicjalizowany (np. na starcie),
        // dajmy fallback 800×600
        viewportSize = QSize(800, 600);
    }

    qreal viewRatio = viewportSize.width() / (qreal)viewportSize.height();

    // 3) Oblicz docelowy rozmiar renderu (w pikselach) tak, aby zachować proporcje:
    //    - jeśli strona jest „szersza” (wyższy pageRatio niż viewRatio), to dopasujmy szerokość,
    //      a wysokość obliczymy z proporcji;
    //    - w przeciwnym wypadku dopasujmy wysokość, a szerokość obliczymy z proporcji.
    int renderW, renderH;
    const int scaleFactor = 2; // renderujmy 2× większy obraz, żeby późniejsze skalowanie było ładne

    if (pageRatio > viewRatio) {
        // strona jest proporcjonalnie szersza niż viewport: maksymalna szerokość = viewport.width() * scaleFactor
        renderW = viewportSize.width() * scaleFactor;
        renderH = qRound(renderW / pageRatio);
    } else {
        // strona jest proporcjonalnie „wyższa” lub równa niż viewport: maksymalna wysokość = viewport.height() * scaleFactor
        renderH = viewportSize.height() * scaleFactor;
        renderW = qRound(renderH * pageRatio);
    }

    QSize renderSize(renderW, renderH);

    // 4) Ustaw opcje renderowania (domyślnie brak flag „aliased”)
    QPdfDocumentRenderOptions options;
    options.setRenderFlags(
        QPdfDocumentRenderOptions::RenderFlags(QPdfDocumentRenderOptions::RenderFlag::None)
    );

    // 5) Wyrenderuj stronę do QImage o podanym rozmiarze
    QImage image = pdfDocument->render(pageIndex, renderSize, options);
    if (image.isNull()) {
        QMessageBox::warning(this, tr("Błąd"),
                             tr("Nie udało się wyrenderować strony PDF."));
        return;
    }

    // 6) Teraz przeskaluj obraz do rzeczywistego viewportu, zachowując proporcje
    //    (na wypadek gdyby viewport miał inny stosunek niż renderSize):
    QPixmap pix = QPixmap::fromImage(image);
    QPixmap scaled = pix.scaled(viewportSize,
                                Qt::KeepAspectRatio,
                                Qt::SmoothTransformation);

    // 7) Podmień pixmapę w QLabel na przeskalowany i ustaw rozmiar etykiety
    pdfLabel->setPixmap(scaled);
    pdfLabel->resize(scaled.size());
}

void TextViewer::loadPdf(const QString &fileName)
{
    // 1) Sprawdź, czy plik jest już na liście – jeśli tak, zaznacz go i break
    for (int i = 0; i < sidebar->count(); ++i) {
        QListWidgetItem *it = sidebar->item(i);
        if (it->data(Qt::UserRole).toString() == fileName) {
            sidebar->setCurrentItem(it);
            break;
        }
    }

    // 2) Jeśli nie ma – dodaj do sidebar i zapisz w QSettings
    bool alreadyInList = false;
    for (int i = 0; i < sidebar->count(); ++i) {
        if (sidebar->item(i)->data(Qt::UserRole).toString() == fileName) {
            alreadyInList = true;
            break;
        }
    }
    if (!alreadyInList) {
        QListWidgetItem *newItem =
            new QListWidgetItem(QFileInfo(fileName).fileName());
        newItem->setData(Qt::UserRole, fileName);
        sidebar->addItem(newItem);

        // Zapisz w QSettings
        QStringList saved = {};
        for (int i = 0; i < sidebar->count(); ++i) {
            saved.append(sidebar->item(i)->data(Qt::UserRole).toString());
        }
        QSettings settings("MyApp", "TextViewer");
        settings.setValue("recentFiles", saved);
    }

    // 3) Zamyknij poprzedni PDF (jeśli był otwarty)
    pdfDocument->close();

    // 4) Załaduj nowy PDF
    QPdfDocument::Error status = pdfDocument->load(fileName);
    if (status != QPdfDocument::Error::None) {
        QString msg;
        switch (status) {
            case QPdfDocument::Error::FileNotFound:
                msg = tr("Plik PDF nie istnieje lub nie można go otworzyć.");
                break;
            case QPdfDocument::Error::InvalidFileFormat:
                msg = tr("Format pliku PDF nie jest obsługiwany.");
                break;
            default:
                msg = tr("Wystąpił błąd podczas otwierania pliku PDF.");
        }
        QMessageBox::warning(this, tr("Błąd"), msg);
        return;
    }

    // 5) Zlicz liczbę stron i sprawdź poprawność
    totalPdfPages = pdfDocument->pageCount();
    if (totalPdfPages <= 0) {
        QMessageBox::warning(this, tr("Błąd"),
                             tr("Dokument PDF nie zawiera żadnych stron."));
        return;
    }
    currentPdfPage = 0;
    isPdf = true;

    // 6) Przełącz na widok PDF wewnątrz pageStack
    pageStack->setCurrentIndex(1);
    pdfScrollArea->setVisible(true);

    // 7) Wyrenderuj pierwszą stronę
    renderPdfPage(currentPdfPage);

    // 8) Ustaw przyciski nawigacji
    prevButton->setEnabled(false);
    nextButton->setEnabled(totalPdfPages > 1);

    // 9) Wyłącz (na czas oglądania PDF) przyciski zmiany czcionki
    switchFontButton->setEnabled(false);
    increaseFontButton->setEnabled(false);
    decreaseFontButton->setEnabled(false);
}

void TextViewer::showMainMenu()
{
    // Jeśli został ustawiony rodzic (MainWindow), to przywróć jego widoczność,
    // a siebie zamknij. Jeżeli TextViewer miał innego rodzica, to też go pokażemy.
    if (QWidget *parent = qobject_cast<QWidget*>(this->parentWidget())) {
        parent->show();
    }
    // Zamknij bieżące okno TextViewer – użytkownik wróci do MainWindow
    this->close();
}