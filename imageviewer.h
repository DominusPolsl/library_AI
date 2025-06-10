#pragma once

#include <QWidget>
#include <QStringList>
#include <QLabel>
#include <QScrollArea>
#include <QPushButton>
#include <QImage>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QScrollBar>

class ImageViewer : public QWidget
{
    Q_OBJECT

public:
    explicit ImageViewer(const QStringList &recentImages, QWidget *parent = nullptr);
    // Panowanie (strzałki):
    void panImage(int dx, int dy);
    // Pomocnicze: zoom w centrum widoku
    void zoomInAtCenter();
    void zoomOutAtCenter();
    ~ImageViewer() override;

signals:
    void returnToMainMenuClicked();
    void fileAdded(const QString &path);
    void fileRemoved(const QString &path);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private slots:
    void openImage();
    void clearImage();
    void onBackButtonClicked();

private:
    void updateImageDisplay();

    // Pełna funkcja zoomująca wokół punktu viewportu
    void zoom(double factor, const QPointF &viewportAnchor);

    QLabel      *imageLabel;
    QScrollArea *scrollArea;
    QPushButton *openButton;
    QPushButton *clearButton;
    QPushButton *backButton;

    QStringList  rememberedImages;
    QImage       currentImage;      // oryginalny obraz
    QString      lastLoadedPath;

    // by domyślnie zmieścić oryginał w ~90% viewportu
    double       fitFactor;
    // dodatkowy zoom zadany przez użytkownika
    double       userScale;
};