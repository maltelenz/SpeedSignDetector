#include "detector.h"

//Qt includes
#include <QPixmap>
#include <QRgb>
#include <QGraphicsBlurEffect>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QPainter>
#include <qmath.h>
#include <QDebug>

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

QPixmap Detector::getSobelAnglePixmap()
{
  return QPixmap::fromImage(sobelAngles_);
}

QRect Detector::getImageSize()
{
  return img_.rect();
}

QColor Detector::averageSection(int xStart, int yStart, int xStop, int yStop)
{
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

void Detector::blurred()
{
  QGraphicsBlurEffect *blur = new QGraphicsBlurEffect;
  blur->setBlurRadius(2);

  QGraphicsScene scene;
  QGraphicsPixmapItem item;

  item.setPixmap(getPixmap());
  item.setGraphicsEffect(blur);

  scene.addItem(&item);
  QPainter ptr(&img_);
  scene.render(&ptr, QRectF(), img_.rect());
}

void Detector::sobelEdges()
{
  // Sobel masks
  int gX[3][3] = {
      {-1, 0, 1},
      {-2, 0, 2},
      {-1, 0, 1}
    };

  int gY[3][3] = {
    {1, 2, 1},
    {0, 0, 0},
    {-1, -2, -1}
  };

  QImage res(img_.size(), QImage::Format_RGB32);
  sobelAngles_ = QImage(img_.size(), QImage::Format_Grayscale8);

  int width = img_.width();
  int height = img_.height();

  int i, j;
  long sumX, sumY;
  int sum;
  int angle;
  uint color;

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      // Make outermost border black, so we can have an easier/faster for loop below
      if( y <= 0 || y >= height - 1 || x <= 0 || x >= width - 1 ) {
        sum = 0;
        angle = 0;
      } else {
        sumX = 0;
        sumY = 0;
        for ( i = -1; i <= 1; i++) {
          for (j = -1; j <= 1; j++) {
           color = img_.pixel(x + i, y + j);
           sumX += qGray(color) * gX[i + 1][j + 1];
           sumY += qGray(color) * gY[i + 1][j + 1];
          }
        }
        sum = abs(sumX) + abs(sumY);
        sum = qMin(sum, 255);
        double phi = atan2(sumY, sumX);
        while (phi < 0) {
          phi += M_PI;
        }
        // Angle is between 0 and 180 degrees, where 0 is E/W, 45 is NE/SW and 90 is N/S
        angle = qRound(qRadiansToDegrees(phi));
      }
      res.setPixel(x, y, qRgb(sum, sum, sum));
      sobelAngles_.setPixel(x, y, qRgb(angle, angle, angle));
    }
  }
  img_ = res;
}

void Detector::edgeThinning()
{

  int width = img_.width();
  int height = img_.height();

  int c1, c2, c3, c4;
  int cP, cN;
  int pixelColor;
  int angle;

  QImage res(img_);

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      // Ignore outermost border, so we can have an easier/faster checking below
      if( y <= 0 || y >= height - 1 || x <= 0 || x >= width - 1 ) {
        continue;
      }
      pixelColor = qGray(img_.pixel(x, y));
      if (pixelColor <= 0) {
        // Already black, needs no thinning
        continue;
      }

      angle = qGray(sobelAngles_.pixel(x, y));
      if (angle <= 45) {
        c1 = qGray(img_.pixel(x, y - 1));
        c2 = qGray(img_.pixel(x, y + 1));
        c3 = qGray(img_.pixel(x - 1, y - 1));
        c4 = qGray(img_.pixel(x + 1, y + 1));
      } else if (angle <= 90) {
        c1 = qGray(img_.pixel(x + 1, y));
        c2 = qGray(img_.pixel(x - 1, y));
        c3 = qGray(img_.pixel(x + 1, y - 1));
        c4 = qGray(img_.pixel(x - 1, y + 1));
      } else if (angle <= 135) {
        c1 = qGray(img_.pixel(x - 1, y));
        c2 = qGray(img_.pixel(x + 1, y));
        c3 = qGray(img_.pixel(x - 1, y + 1));
        c4 = qGray(img_.pixel(x + 1, y - 1));
      } else /* if (angle < 180) */ {
        c1 = qGray(img_.pixel(x, y + 1));
        c2 = qGray(img_.pixel(x, y - 1));
        c3 = qGray(img_.pixel(x + 1, y + 1));
        c4 = qGray(img_.pixel(x - 1, y - 1));
      }

      cP = interpolate(c1, c3, angle % 45);
      cN = interpolate(c2, c4, angle % 45);

      if (pixelColor < cP || pixelColor < cN) {
        // The current pixel is weaker than its surroundings, kill it.
        res.setPixel(x, y, qRgb(0, 0, 0));
      }
    }
  }
  img_ = res;
}

int Detector::interpolate(int a, int b, int progress)
{
  return a + (a - b) * ((float) progress / 45);
}
