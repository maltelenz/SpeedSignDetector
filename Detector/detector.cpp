#include "detector.h"

//Qt includes
#include <QPixmap>
#include <QRgb>

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
