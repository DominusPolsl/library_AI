#include "imageviewer.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QPalette>


ImageViewer::ImageViewer(const QStringList &recentImages, QWidget *parent)
    : QWidget(parent),
      imageLabel(new QLabel),
      scrollArea(new QScrollArea),
      openButton(new QPushButton(tr("Open Image"))),
      clearButton(new QPushButton(tr("Clear"))),
      backButton(new QPushButton(tr("Back"))),
      rememberedImages(recentImages),
      currentImage(),
      lastLoadedPath(),
      fitFactor(1.0),     // współczynnik dopasowania obrazu do okna
      userScale(1.0)      // dodatkowy zoom od użytkownika
{
    // QLabel — do wyświetlania obrazu (bez wymuszania rozmiaru)
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    // ScrollArea — zawiera QLabel, bez pasków przewijania
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    scrollArea->setWidgetResizable(false);
    scrollArea->setAlignment(Qt::AlignCenter);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Połączenia przycisków z funkcjami
    connect(openButton, &QPushButton::clicked, this, &ImageViewer::openImage);
    connect(clearButton, &QPushButton::clicked, this, &ImageViewer::clearImage);
    connect(backButton, &QPushButton::clicked, this, &ImageViewer::onBackButtonClicked);

    // Układ przycisków w poziomie
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(openButton);
    buttonLayout->addWidget(clearButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(backButton);

    // Główny layout — obraz + przyciski
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(scrollArea);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);

    // Tytuł okna i domyślny rozmiar
    setWindowTitle(tr("Image Viewer"));
    resize(800, 600);
}

// Destruktor — nie trzeba ręcznie usuwać wskaźników (Qt robi to automatycznie)
ImageViewer::~ImageViewer()
{ }

// Otwiera i ładuje plik obrazu
void ImageViewer::openImage()
{
    try
    {
        // 1) Okno dialogowe do wyboru obrazu
        QString fileName = QFileDialog::getOpenFileName(
            this,
            tr("Select image file"),
            QString(),
            tr("Images (*.png *.jpg *.jpeg *.bmp *.gif);;All files (*)"));

        if (fileName.isEmpty())
            return;

        // 2) Próba wczytania obrazu
        QImage img;
        if (!img.load(fileName)) {
            QMessageBox::warning(
                this,
                tr("Image Viewer"),
                tr("Failed to open image:\n%1").arg(fileName)
            );
        }

        currentImage = img;
        lastLoadedPath = fileName;

        // 3) Obliczenie fitFactor — maks. 90% rozmiaru viewportu
        QSize vpSz = scrollArea->viewport()->size();
        if (!currentImage.isNull() && vpSz.width() > 0 && vpSz.height() > 0) {
            QSize maxSz(int(vpSz.width()  * 0.9), int(vpSz.height() * 0.9));
            QSize origSz = currentImage.size();
            double fx = double(maxSz.width())  / origSz.width();
            double fy = double(maxSz.height()) / origSz.height();
            fitFactor = qMin(fx, fy);
            if (fitFactor > 1.0) {
                fitFactor = 1.0; // nie skaluj w górę jeśli nie trzeba
            }
        } else {
            fitFactor = 1.0;
        }

        // 4) Reset zoomu użytkownika
        userScale = 1.0;

        // 5) Odśwież widok obrazu
        updateImageDisplay();

        // 6) Powiadom MainWindow o nowym pliku
        emit fileAdded(fileName);
    }
    catch (const std::exception &e) {
        qDebug() << "Unexpected error:" << e.what();
    }
}

//  Czyści obraz i resetuje stan
void ImageViewer::clearImage()
{
    if (!currentImage.isNull() && !lastLoadedPath.isEmpty()) {
        emit fileRemoved(lastLoadedPath); // powiadom MainWindow
    }

    imageLabel->clear();           // usuń obraz z QLabel
    currentImage = QImage();       // zresetuj obraz
    lastLoadedPath.clear();        // usuń ścieżkę
    fitFactor = 1.0;               // reset dopasowania
    userScale = 1.0;               // reset zoomu
}

// Sygnał powrotu do głównego menu
void ImageViewer::onBackButtonClicked()
{
    emit returnToMainMenuClicked();
}


void ImageViewer::updateImageDisplay()
{
    if (currentImage.isNull()) {
        imageLabel->clear();
        return;
    }

    // Tworzenie pixmapy i przeskalowanie w oparciu o fitFactor * userScale
    QPixmap pixmap = QPixmap::fromImage(currentImage);
    double totalScale = fitFactor * userScale;
    if (totalScale <= 0.0) {
        totalScale = 1.0;
    }

    QSize newSize(
        int(pixmap.width()  * totalScale),
        int(pixmap.height() * totalScale)
    );
    pixmap = pixmap.scaled(
        newSize,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
    );

    imageLabel->setPixmap(pixmap);
    imageLabel->adjustSize();
}

void ImageViewer::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    // Gdy zmienia się wielkość okna, ponownie odbywa się obliczanie fitFactor
    // (by oryginał maks. 90% viewportu), zachowując userScale.
    if (!currentImage.isNull()) {
        QSize vpSz = scrollArea->viewport()->size();
        if (vpSz.width() > 0 && vpSz.height() > 0) {
            QSize maxSz(int(vpSz.width()  * 0.9),
                        int(vpSz.height() * 0.9));
            QSize origSz = currentImage.size();
            double fx = double(maxSz.width())  / origSz.width();
            double fy = double(maxSz.height()) / origSz.height();
            fitFactor = qMin(qMin(fx, fy), 1.0);
        }
        updateImageDisplay();
    }
}

void ImageViewer::keyPressEvent(QKeyEvent *event)
{
    // 1) Ctrl + [+] lub Ctrl + [-] → zoom w centrum
    if (event->modifiers() & Qt::ControlModifier) {
        switch (event->key()) {
            case Qt::Key_Plus:
            case Qt::Key_Equal:  // '+' to Shift+'='
                zoomInAtCenter();
                return;
            case Qt::Key_Minus:
            case Qt::Key_Underscore:
                zoomOutAtCenter();
                return;
            default:
                break;
        }
    }

    // 2) Bez Ctrl: strzałki -> pan
    if (!(event->modifiers() & Qt::ControlModifier)) {
        constexpr int step = 20; // krok przesunięcia
        switch (event->key()) {
            case Qt::Key_Left:
                panImage(-step, 0);
                return;
            case Qt::Key_Right:
                panImage(+step, 0);
                return;
            case Qt::Key_Up:
                panImage(0, -step);
                return;
            case Qt::Key_Down:
                panImage(0, +step);
                return;
            default:
                break;
        }
    }

    QWidget::keyPressEvent(event);
}

void ImageViewer::wheelEvent(QWheelEvent *event)
{
    // 1) Ctrl + wheel -> zoom w centrum
    if (event->modifiers() & Qt::ControlModifier) {
        // punkt kotwiczenia = środek viewportu:
        QSize vpSz = scrollArea->viewport()->size();
        QPointF centerPt(vpSz.width() / 2.0, vpSz.height() / 2.0);

        if (event->angleDelta().y() > 0) {
            zoom(1.25, centerPt);
        } else if (event->angleDelta().y() < 0) {
            zoom(1.0 / 1.25, centerPt);
        }
        event->accept();
        return;
    }

    // 2) Bez Ctrl – ignorowanie przewijania (pan tylko strzałkami)
    event->ignore();
}

void ImageViewer::zoom(double factor, const QPointF &viewportAnchor)
{
    if (currentImage.isNull())
        return;

    // 1) Oblicz stare i nowe skalowanie
    double oldTotal = fitFactor * userScale;
    double newUserScale = userScale * factor;

    // Ogranicz nowy userScale
    if (newUserScale < 0.1)    newUserScale = 0.1;
    if (newUserScale > 10.0)   newUserScale = 10.0;

    double newTotal = fitFactor * newUserScale;
    if (newTotal <= 0.0)
        newTotal = 1.0;

    // 2) Pobierz bieżące wartości suwaków
    QScrollBar *hBar = scrollArea->horizontalScrollBar();
    QScrollBar *vBar = scrollArea->verticalScrollBar();
    int hOld = hBar->value();
    int vOld = vBar->value();

    // 3) Oblicz współrzędne punktu kotwiczenia w „przestrzeni oryginału”
    double fx = (hOld + viewportAnchor.x()) / oldTotal;
    double fy = (vOld + viewportAnchor.y()) / oldTotal;

    // 4) Zapisz newUserScale i przerysuj obraz
    userScale = newUserScale;
    updateImageDisplay();

    // 5) Oblicz nowe pozycje suwaków, aby ten sam „oryginalny piksel”
    //    wylądował w tym samym miejscu viewportAnchor po nowym przeskalowaniu:
    int hNew = int(fx * newTotal - viewportAnchor.x());
    int vNew = int(fy * newTotal - viewportAnchor.y());

    // 6) Ogranicz je do zakresu [min, max]
    hNew = qBound(hBar->minimum(), hNew, hBar->maximum());
    vNew = qBound(vBar->minimum(), vNew, vBar->maximum());

    // 7) Ustaw suwaki
    hBar->setValue(hNew);
    vBar->setValue(vNew);
}

// Powiększenie obrazu względem środka widocznego obszaru (viewportu)
void ImageViewer::zoomInAtCenter()
{
    // Obliczenie środku viewportu jako punkt odniesienia
    QSize vpSz = scrollArea->viewport()->size();
    QPointF centerPt(vpSz.width() / 2.0, vpSz.height() / 2.0);

    // Powiększenie obrazu o 25% względem środka
    zoom(1.25, centerPt);
}

// Pomniejszenie obrazu względem środka widocznego obszaru
void ImageViewer::zoomOutAtCenter()
{
    // Obliczenie środku viewportu
    QSize vpSz = scrollArea->viewport()->size();
    QPointF centerPt(vpSz.width() / 2.0, vpSz.height() / 2.0);

    // Pomniejszenie obrazu o 20% względem środka
    zoom(1.0 / 1.25, centerPt);
}

// Przesuwanie (pan) obrazu o zadany wektor (dx, dy)
void ImageViewer::panImage(int dx, int dy)
{
    QScrollBar *hBar = scrollArea->horizontalScrollBar(); // pasek poziomy
    QScrollBar *vBar = scrollArea->verticalScrollBar();   // pasek pionowy

    // Obliczenie nowej pozycji pasków
    int newH = hBar->value() + dx;
    int newV = vBar->value() + dy;

    // Ograniczenie nowej wartości do dostępnego zakresu suwaków
    newH = qBound(hBar->minimum(), newH, hBar->maximum());
    newV = qBound(vBar->minimum(), newV, vBar->maximum());

    // Ustawienie nowych pozycji suwaków, co powoduje przesunięcie obrazu
    hBar->setValue(newH);
    vBar->setValue(newV);
}
