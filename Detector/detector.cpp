#include "detector.h"

//Qt includes
#include <QPixmap>
#include <QRgb>
#include <QGraphicsBlurEffect>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QPainter>

Detector::Detector() :
  imgSize_(700, 700)
{
}

Detector::Detector(QString file) :
  file_(file)
{
}

void Detector::loadImage()
{
  img_ = QImage(file_);
  img_ = img_.scaled(imgSize_, Qt::KeepAspectRatio);
}

void Detector::loadImage(QString file)
{
  file_ = file;
  loadImage();
}

QPixmap Detector::getPixmap()
{
  return QPixmap::fromImage(img_);
}

QRect Detector::getImageSize()
{
  return img_.rect();
}

QColor Detector::averageSection(int xStart, int yStart, int xStop, int yStop) {
  int red(0);
  int green(0);
  int blue(0);

  for (int lineNumber = yStart; lineNumber <= yStop; lineNumber++) {
    const uchar* byte = img_.constScanLine(lineNumber);
    for (int columnNumber = xStart; columnNumber < xStop; columnNumber++) {
      const QRgb* rgb = (const QRgb*) byte + columnNumber;
      red += qRed(*rgb);
      green += qGreen(*rgb);
      blue += qBlue(*rgb);
    }
  }
  int numberPixels = (yStop - yStart) * (xStop - xStart);
  red = red/numberPixels;
  green = green/numberPixels;
  blue = blue/numberPixels;

  return QColor(red, green, blue);
}

QImage Detector::averageLines()
{
  QImage meanImg(1, img_.height(), QImage::Format_RGB32);

  int bytesPerLine(img_.bytesPerLine());
  int lines(img_.height());
  for (int lineNumber = 0; lineNumber <= lines; lineNumber++) {
    const uchar* byte = img_.constScanLine(lineNumber);
    int red(0);
    int green(0);
    int blue(0);
    for (int columnNumber = 0; columnNumber < bytesPerLine; columnNumber++) {
      const QRgb* rgb = (const QRgb*) byte + columnNumber;
      red += qRed(*rgb);
      green += qGreen(*rgb);
      blue += qBlue(*rgb);
    }
    red = red/bytesPerLine;
    green = green/bytesPerLine;
    blue = blue/bytesPerLine;
    QRgb* rowLine = (QRgb*) meanImg.scanLine(lineNumber);
    *rowLine = qRgb(red, green, blue);
  }
  meanImg = meanImg.scaled(img_.width(), img_.height(), Qt::IgnoreAspectRatio);

  return meanImg;
}

QImage Detector::blurred()
{
  QGraphicsBlurEffect *blur = new QGraphicsBlurEffect;
  blur->setBlurRadius(5);

  QGraphicsScene scene;
  QGraphicsPixmapItem item;

  item.setPixmap(getPixmap());
  item.setGraphicsEffect(blur);

  scene.addItem(&item);
  QImage res(img_.size(), QImage::Format_ARGB32);
  QPainter ptr(&res);
  scene.render(&ptr, QRectF(), img_.rect());
  return res;
}
