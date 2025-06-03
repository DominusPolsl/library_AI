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
      openButton(new QPushButton(tr("Otwórz obraz"))),
      clearButton(new QPushButton(tr("Wyczyść"))),
      backButton(new QPushButton(tr("Powrót"))),
      rememberedImages(recentImages),
      currentImage(),
      lastLoadedPath(),
      fitFactor(1.0),
      userScale(1.0)
{
    // 1) QLabel (bez automatycznego rozciągania)
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    // 2) QScrollArea: wyśrodkuj, ukryj paski przewijania
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    scrollArea->setWidgetResizable(false);
    scrollArea->setAlignment(Qt::AlignCenter);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 3) Podłącz przyciski
    connect(openButton, &QPushButton::clicked,
            this,       &ImageViewer::openImage);
    connect(clearButton, &QPushButton::clicked,
            this,        &ImageViewer::clearImage);
    connect(backButton, &QPushButton::clicked,
            this,      &ImageViewer::onBackButtonClicked);

    // 4) Layout przycisków
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(openButton);
    buttonLayout->addWidget(clearButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(backButton);

    // 5) Główny layout: obraz + przyciski
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(scrollArea);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);

    setWindowTitle(tr("Image Viewer"));
    resize(800, 600);
}

ImageViewer::~ImageViewer()
{
    // Qt sam usuwa wszystkie widgety-dzieci
}

void ImageViewer::openImage()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Wybierz plik obrazu"),
        QString(),
        tr("Obrazy (*.png *.jpg *.jpeg *.bmp *.gif);;Wszystkie pliki (*)")
    );
    if (fileName.isEmpty())
        return;

    QImage img;
    if (!img.load(fileName)) {
        QMessageBox::warning(
            this,
            tr("Image Viewer"),
            tr("Nie udało się wczytać obrazu:\n%1").arg(fileName)
        );
        return;
    }

    // 1) Zachowaj oryginał i ścieżkę
    currentImage = img;
    lastLoadedPath = fileName;

    // 2) Oblicz fitFactor: max. 90% viewportu
    QSize vpSz = scrollArea->viewport()->size();
    if (!currentImage.isNull() && vpSz.width() > 0 && vpSz.height() > 0) {
        QSize maxSz(int(vpSz.width()  * 0.9),
                    int(vpSz.height() * 0.9));
        QSize origSz = currentImage.size();
        double fx = double(maxSz.width())  / origSz.width();
        double fy = double(maxSz.height()) / origSz.height();
        fitFactor = qMin(fx, fy);
        if (fitFactor > 1.0) {
            fitFactor = 1.0; // nie powiększaj, jeśli oryginał jest mniejszy
        }
    } else {
        fitFactor = 1.0;
    }

    // 3) Reset dodatkowego zoomu użytkownika
    userScale = 1.0;

    // 4) Wyświetl obraz (fitFactor * userScale)
    updateImageDisplay();

    // 5) Powiadom MainWindow o nowej ścieżce
    emit fileAdded(fileName);
}

void ImageViewer::clearImage()
{
    if (!currentImage.isNull() && !lastLoadedPath.isEmpty()) {
        emit fileRemoved(lastLoadedPath);
    }
    imageLabel->clear();
    currentImage = QImage();
    lastLoadedPath.clear();
    fitFactor = 1.0;
    userScale = 1.0;
}

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

    // Utwórz pixmapę i przeskaluj w oparciu o fitFactor * userScale
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

    // Gdy zmienia się wielkość okna, ponownie obliczamy fitFactor
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

    // 2) Bez Ctrl – ignorujemy przewijanie (pan tylko strzałkami)
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
    //    fx = (hOld + viewportAnchor.x) / oldTotal
    //    fy = (vOld + viewportAnchor.y) / oldTotal
    double fx = (hOld + viewportAnchor.x()) / oldTotal;
    double fy = (vOld + viewportAnchor.y()) / oldTotal;

    // 4) Zapisz newUserScale i przerysuj obraz
    userScale = newUserScale;
    updateImageDisplay();

    // 5) Oblicz nowe pozycje suwaków, aby ten sam „oryginalny piksel”
    //    wylądował w tym samym miejscu viewportAnchor po nowym przeskalowaniu:
    //    hNew = fx * newTotal - viewportAnchor.x
    //    vNew = fy * newTotal - viewportAnchor.y
    int hNew = int(fx * newTotal - viewportAnchor.x());
    int vNew = int(fy * newTotal - viewportAnchor.y());

    // 6) Ogranicz je do zakresu [min, max]
    hNew = qBound(hBar->minimum(), hNew, hBar->maximum());
    vNew = qBound(vBar->minimum(), vNew, vBar->maximum());

    // 7) Ustaw suwaki
    hBar->setValue(hNew);
    vBar->setValue(vNew);
}

void ImageViewer::zoomInAtCenter()
{
    // punkt kotwiczenia = środek viewportu
    QSize vpSz = scrollArea->viewport()->size();
    QPointF centerPt(vpSz.width() / 2.0, vpSz.height() / 2.0);
    zoom(1.25, centerPt);
}

void ImageViewer::zoomOutAtCenter()
{
    QSize vpSz = scrollArea->viewport()->size();
    QPointF centerPt(vpSz.width() / 2.0, vpSz.height() / 2.0);
    zoom(1.0 / 1.25, centerPt);
}

void ImageViewer::panImage(int dx, int dy)
{
    QScrollBar *hBar = scrollArea->horizontalScrollBar();
    QScrollBar *vBar = scrollArea->verticalScrollBar();

    int newH = hBar->value() + dx;
    int newV = vBar->value() + dy;

    newH = qBound(hBar->minimum(), newH, hBar->maximum());
    newV = qBound(vBar->minimum(), newV, vBar->maximum());

    hBar->setValue(newH);
    vBar->setValue(newV);
}
