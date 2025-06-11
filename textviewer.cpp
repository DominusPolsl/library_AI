#include "textviewer.h"         
#include <QFileDialog>          // Okno dialogowe do wyboru pliku
#include <QMessageBox>          // Komunikaty błędów
#include <QPixmap>              // Do konwersji obrazu PDF do wyświetlenia
#include <QDir>                 // Ścieżki katalogów

// Inicjalizacja interfejsu użytkownika i łączenie przycisków z odpowiednimi funkcjami
TextViewer::TextViewer(QWidget *parent)
    : QWidget(parent), currentPage(0)  // Ustawiamy aktualną stronę na 0
{
    // Tworzenie obiektu reprezentującego dokument PDF
    pdfDoc = new QPdfDocument(this);

    // Etykieta do wyświetlania strony jako obrazu
    pageLabel = new QLabel(this);
    pageLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter); // Wyśrodkowanie poziome, wyrównanie do góry
    pageLabel->setMinimumSize(400, 600);                      // Minimalny rozmiar widoku strony
    pageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding); // Automatyczne skalowanie

    // Tworzenie przycisków interfejsu
    openButton = new QPushButton("📁 File", this);      // Otwórz plik PDF
    prevButton = new QPushButton("←", this);           // Poprzednia strona
    nextButton = new QPushButton("→", this);           // Następna strona
    QPushButton *backButton = new QPushButton("Back to Menu", this); // Powrót do menu
    backButton->setFixedSize(120, 30);                 

     // Ustawienie szerokości przycisków nawigacyjnych (połowa standardowej szerokości)
    int halfWidth = 60;
    prevButton->setFixedWidth(halfWidth);
    nextButton->setFixedWidth(halfWidth);

    // === Rząd (poziomy układ) dla przycisków nawigacyjnych stron PDF ===
    QHBoxLayout *pageNavRow = new QHBoxLayout();      // Tworzymy układ poziomy
    pageNavRow->setSpacing(5);                        // Ustawiamy odstęp 5 pikseli między przyciskami
    // Dodawanie Przycisków zmiany strony
    pageNavRow->addWidget(prevButton);                
    pageNavRow->addWidget(nextButton);                
    // Wyrównanie przyciskó do lewej strony
    pageNavRow->setAlignment(Qt::AlignLeft);

    // Lewy panel boczny z przyciskami (wertykalnie ułożony)
    QVBoxLayout *leftBar = new QVBoxLayout();
    leftBar->setContentsMargins(10, 10, 10, 10);        // Marginesy wokół przycisków
    leftBar->setSpacing(10);                           // Odstępy między przyciskami
    leftBar->addWidget(backButton);                    
    leftBar->addWidget(openButton);                    
    leftBar->addLayout(pageNavRow);                                       
    leftBar->addStretch();                             // Wypełnienie pustej przestrzeni poniżej

    // Główna sekcja wyświetlania PDF
    QVBoxLayout *pdfLayout = new QVBoxLayout();        // Układ pionowy (tylko strona PDF)
    pdfLayout->setContentsMargins(0, 10, 10, 10);
    pdfLayout->setSpacing(0);
    pdfLayout->addWidget(pageLabel);                   

    // Główny układ aplikacji: poziomy podział — lewa kolumna + prawa sekcja
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);         // Bez zewnętrznych marginesów
    mainLayout->setSpacing(0);                          // Bez przerwy między kolumnami
    mainLayout->addLayout(leftBar);                     // Panel z przyciskami
    mainLayout->addLayout(pdfLayout, 1);                // PDF zajmuje resztę miejsca

    setLayout(mainLayout);                              // Ustawienie układu jako głównego

    // Połączenie sygnałów z funkcjami
    connect(backButton, &QPushButton::clicked, this, [this]() {
        emit backToMenuRequested();                     // Wysyłanie sygnału do Mainwindow, żeby wrócić do głównego menu
    });

    connect(openButton, &QPushButton::clicked, this, &TextViewer::openPdf);   // Podłączenie sygnału kliknięcia przycisku openButton do funkcji openPdf() w klasie TextViewer
    connect(prevButton, &QPushButton::clicked, this, &TextViewer::prevPage);  
    connect(nextButton, &QPushButton::clicked, this, &TextViewer::nextPage);  
}

// Funkcja otwierająca i wczytująca plik PDF
void TextViewer::openPdf() {
    // Okno dialogowe do wyboru pliku PDF
    QString fileName = QFileDialog::getOpenFileName(
        this, "Open PDF", QDir::homePath(), "PDF files (*.pdf)"
    );

    if (fileName.isEmpty()) return;  // Jeśli anulowano wybór, wtedy nic nie będzie zrobione

    // Próba załadowania pliku PDF
    QPdfDocument::Error err = pdfDoc->load(fileName);
    if (err != QPdfDocument::Error::None) {
        // Jeśli wystąpił błąd - wyświetli się komunikat
        QMessageBox::critical(this, "Error", "Failed to load PDF file.");
        return;
    }

    currentPage = 0;  // Resetujemy do pierwszej strony
    showPage();       // Wyświetlenie strony
}


// Funkcja renderująca aktualną stronę PDF i wyświetlająca ją jako obraz
void TextViewer::showPage() {
    if (!pdfDoc || pdfDoc->pageCount() <= 0) return;  // Koniec metody przy braku dokumentu

    QSize imageSize(1300, 1000);                      // Rozdzielczość renderowania strony
    QPdfDocumentRenderOptions options;                // Opcje renderowania (domyślne)

    // Renderowanie aktualnej strony jako obrazu
    QImage image = pdfDoc->render(currentPage, imageSize, options);

    if (image.isNull()) {
        // Jeśli nie udało się wyrenderować, to pakazuje się błąd
        QMessageBox::warning(this, "Error", "Unable to render PDF page.");
        return;
    }

    // Przekształcenie QImage na QPixmap i wyświetlenie w etykiecie
    pageLabel->setPixmap(QPixmap::fromImage(image));
}


// Przejście do następnej strony (jeśli nie jest koniec pliku)
void TextViewer::nextPage() {
    if (currentPage + 1 < pdfDoc->pageCount()) {
        ++currentPage;
        showPage();
    }
}

// Przejście do poprzedniej strony (jeśli nie jest to początek pliku) 
void TextViewer::prevPage() {
    if (currentPage > 0) {
        --currentPage;
        showPage();
    }
}
