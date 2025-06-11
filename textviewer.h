#pragma once

#include <QWidget>                     // Klasa bazowa dla wszystkich komponentów GUI
#include <QPdfDocument>                // Klasa umożliwiająca wczytywanie dokumentów PDF
#include <QPdfDocumentRenderOptions>   // Dodatkowe opcje renderowania PDF
#include <QLabel>                      // Do wyświetlania obrazu strony PDF
#include <QPushButton>                 // Przycisk GUI
#include <QVBoxLayout>                 // Układ pionowy (layout)
#include <QHBoxLayout>                 // Układ poziomy (layout)


// Służy do przeglądania dokumentów PDF strona po stronie.
// Umożliwia otwieranie pliku, nawigację (następna/poprzednia strona) oraz powrót do menu.
class TextViewer : public QWidget {
    Q_OBJECT  // Umożliwia używanie sygnałów i slotów Qt

public:
    // Konstruktor — tworzy widżet PDF (opcjonalnie z rodzicem)
    explicit TextViewer(QWidget *parent = nullptr);

public slots:
    // Slot otwierający plik PDF z systemowego okna dialogowego
    void openPdf();

    // Slot przechodzący do następnej strony PDF
    void nextPage();

    // Slot przechodzący do poprzedniej strony PDF
    void prevPage();

private:
    // Wskaźnik do dokumentu PDF, który został wczytany
    QPdfDocument *pdfDoc;

    // Etykieta" (pageLabel) czyli miejsce w interfejsie, w którym użytkownik widzi aktualną stronę PDF jako obraz.
    QLabel *pageLabel;

    // Przycisk otwierający plik PDF
    QPushButton *openButton;

    // Przycisk do przejścia na następną stronę
    QPushButton *nextButton;

    // Przycisk do przejścia na poprzednią stronę
    QPushButton *prevButton;

    // Numer aktualnie wyświetlanej strony (indeksowana od 0)
    int currentPage;

    // Funkcja renderująca i wyświetlająca bieżącą stronę PDF
    void showPage();

signals:
    // Sygnał wysyłany, gdy użytkownik chce wrócić do menu głównego
    void backToMenuRequested();    
};
