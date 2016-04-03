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

// Detector includes
#include "math_utilities.h"

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
  timer_.start();

  img_ = QImage(file_);
  if (img_.width() > imgSize_.width() || img_.height() > imgSize_.height()) {
    img_ = img_.scaled(imgSize_, Qt::KeepAspectRatio);
  }
  findVoting_ = QImage();

  issueMessage(QString("Loaded image with size: (%1, %2)").arg(img_.width()).arg(img_.height()));

  issueTimingMessage("Load image");
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
  int width(sobelAngles_.xSize());
  int height(sobelAngles_.ySize());

  QImage angleImage(width, height, QImage::Format_RGB32);
  int color;

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      color = sobelAngles_.get(x, y);
      angleImage.setPixel(x, y, qRgb(color, color, color));
    }
  }
  return QPixmap::fromImage(angleImage);
}

QPixmap Detector::getObjectHeatmapPixmap()
{
  return QPixmap::fromImage(findVoting_);
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

  if (yStart >= yStop || xStart >= xStop) {
    // Cannot do operations on negative rectangle
    return QColor();
  }

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

void Detector::blurred()
{
  timer_.start();
  QGraphicsBlurEffect *blur = new QGraphicsBlurEffect;
  blur->setBlurRadius(2);

  QGraphicsScene scene;
  QGraphicsPixmapItem item;

  item.setPixmap(getPixmap());
  item.setGraphicsEffect(blur);

  scene.addItem(&item);
  QPainter ptr(&img_);
  scene.render(&ptr, QRectF(), img_.rect());
  findVoting_ = QImage();
  issueTimingMessage("Blur");
}

void Detector::sobelEdges(int lowerEdgeThreshold)
{
  timer_.start();
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

  int width(img_.width());
  int height(img_.height());

  if (!sobelAngles_.init(width, height)) {
    // Failed to allocate memory, abort nicely
    issueMessage("Failed to allocate memory for the sobelAngles_ in Detector::sobelEdges.");
    timer_.invalidate();
    return;
  }

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
        sum = qAbs(sumX) + qAbs(sumY);
        sum = qMin(sum, 255);
        double phi = atan2upperHalfPlane(sumY, sumX);
        // Angle is between 0 and 180 degrees, where 0 is E/W, 45 is NE/SW and 90 is N/S
        angle = qRound(qRadiansToDegrees(phi));
      }
      if (sum < lowerEdgeThreshold) {
        sum = 0;
      }
      res.setPixel(x, y, qRgb(sum, sum, sum));
      sobelAngles_.set(x, y, angle);
    }
  }
  img_ = res;
  findVoting_ = QImage();
  issueTimingMessage("Edge detection");
}

void Detector::generateRTable()
{
  timer_.start();
  rTable_.clear();

  int width(img_.width());
  int height(img_.height());

  // Assume the feature shape lies in the middle of the image
  int xc = width/2;
  int yc = height/2;

  int pixelColor;
  int angleOfEdge;
  double angleToEdge;
  double distance;

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      // Skip outermost border since we did so for the preparatory steps
      if( y <= 0 || y >= height - 1 || x <= 0 || x >= width - 1 ) {
        continue;
      } else {
        pixelColor = qGray(img_.pixel(x, y));
        if (pixelColor <= 0) {
          // Black, not an edge
          continue;
        }
        // Not black, add an item into the R-table
        angleOfEdge = qGray(sobelAngles_.get(x, y));
        distance = qSqrt(qPow(x - xc, 2) + qPow(y - yc, 2));
        angleToEdge = atan2Positive(y - yc, x - xc);
        rTable_.insert(angleOfEdge, QPair<double, double>(angleToEdge, distance));
      }
    }
  }
//  qDebug() << rTable_;
  findVoting_ = QImage();
  trainingSize_ = img_.size();
  issueTimingMessage("R-table generation");
}

void Detector::findObject(bool createImage)
{
  timer_.start();
  if (!findVoting_.isNull()) {
    // Already have looked for the object.
    return;
  }

  int width(img_.width());
  int height(img_.height());

  Array2D accumulator(width, height);
  if (!accumulator.init()) {
    // Failed to allocate memory, abort nicely
    issueMessage("Failed to allocate memory for the accumulator in Detector::findObject.");
    timer_.invalidate();
    return;
  }

  int pixelColor;
  int angle;

  int xc, yc;

  QPair<double, double> v;

  issuePartialTimingMessage("--Allocated datastructures");

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      // Skip outermost border since we did so for the preparatory steps
      if( y <= 0 || y >= height - 1 || x <= 0 || x >= width - 1 ) {
        continue;
      } else {
        pixelColor = qGray(img_.pixel(x, y));
        if (pixelColor <= 0) {
          // Black, not an edge
          continue;
        }
        // Not black, check the R-table
        angle = qGray(sobelAngles_.get(x, y));
        QList<QPair<double, double> > values = rTable_.values(angle);
        for (int i = 0; i < values.size(); ++i) {
          v = values.at(i);
          xc = qRound(x + v.second * cos(v.first));
          yc = qRound(y + v.second * sin(v.first));
          if (xc >= 0 && xc < width - 1 && yc >= 0 && yc < height - 1) {
            accumulator.increment(xc, yc);
          }
        }
      }
    }
  }

  issuePartialTimingMessage("--Voted");

  int max(0);
  int xmax, ymax;
//  QStringList dump;
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      if (accumulator.get(x, y) > max) {
        max = accumulator.get(x, y);
        xmax = x;
        ymax = y;
      }
      //dump << QString::number(accumulator[offset(x, y, width)]);
    }
    //dump << "\n";
  }
  //qDebug() << dump;

  issueMessage(
        QString("Found max: %1").arg(QString::number(max)));
  issueMessage(
        QString("At (x, y): (%1, %2)").arg(
          QString::number(xmax)).arg(
          QString::number(ymax)));

  emit itemFound(QRect(
              xmax - trainingSize_.width() / 2,
              ymax - trainingSize_.height() / 2,
              trainingSize_.width(),
              trainingSize_.height()
            ),
            max
          );

  issuePartialTimingMessage("--Located max");

  if (createImage) {
    findVoting_ = QImage(width, height, QImage::Format_RGB32);
    int color;
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        color = qRound((double)accumulator.get(x, y)/max * 255);
        findVoting_.setPixel(x, y, qRgb(color, color, color));
      }
    }
    issuePartialTimingMessage("--Created image");
  }

  issueTimingMessage("Object detection");
}


void Detector::findScaledObject(bool createImage)
{
  timer_.start();
  if (!findVoting_.isNull()) {
    // Already have looked for the object.
    return;
  }

  int width(img_.width());
  int height(img_.height());
  int numberScalingSteps(qCeil((SCALING_MAX_ - SCALING_MIN_) / SCALING_STEP_));

  issueMessage(QString("Finding objects with %1 scalings from %2 to %3.").arg(
                 numberScalingSteps).arg(
                 SCALING_MIN_).arg(
                 SCALING_MAX_));

  Array3D accumulator(width, height, numberScalingSteps);
  if (!accumulator.init()) {
    // Failed to allocate memory, abort nicely
    issueMessage("Failed to allocate memory for the accumulator in Detector::findScaledObject.");
    timer_.invalidate();
    return;
  }
  QRgb pixelColor;
  int angle;

  double xcp, ycp;
  int xc, yc;

  QPair<double, double> v;
  issuePartialTimingMessage("Allocated datastructures");

  for (int y = 0; y < height; ++y) {
    const QRgb* line = (const QRgb *)img_.constScanLine(y);
    for (int x = 0; x < width; ++x) {
      if( y <= 0 || y >= height - 1 || x <= 0 || x >= width - 1 ) {
        continue;
      } else {
        pixelColor = line[x];
        if (qGray(pixelColor) == 0) {
          // Black, not an edge
          continue;
        }
        // Not black, check the R-table
        angle = qGray(sobelAngles_.get(x, y));
        foreach (v, rTable_.values(angle)) {
          xcp = v.second * cos(v.first);
          ycp = v.second * sin(v.first);
          for (int s = 0; s < numberScalingSteps; ++s) {

            xc = qRound(x + xcp * (SCALING_MIN_ + s * SCALING_STEP_));
            yc = qRound(y + ycp * (SCALING_MIN_ + s * SCALING_STEP_));
            if (xc >= 0 && xc < width - 1 && yc >= 0 && yc < height - 1) {
              accumulator.increment(xc, yc, s);
            }
          }
        }
      }
    }
  }

  issuePartialTimingMessage("Voted");

  int max(0);
  int xmax, ymax;
  double smax;
//  QStringList dump;
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      for (int s = 0; s < numberScalingSteps; ++s) {
        if (accumulator.get(x, y, s) > max) {
          max = accumulator.get(x, y, s);
          xmax = x;
          ymax = y;
          smax = (SCALING_MIN_ + s * SCALING_STEP_);
        }
//        dump << QString::number(accumulator[x][y][s]);
      }
    }
  }
//  qDebug() << dump;
  issuePartialTimingMessage("Isolated max");

  issueMessage(
        QString("Found max: %1").arg(QString::number(max)));
  issueMessage(
        QString("At (x, y, s): (%1, %2, %3)").arg(
          QString::number(xmax)).arg(
          QString::number(ymax)).arg(
          QString::number(smax)));

  int foundWidth(smax * trainingSize_.width());
  int foundHeight(smax * trainingSize_.height());
  emit itemFound(QRect(
              xmax - foundWidth / 2,
              ymax - foundHeight / 2,
              foundWidth,
              foundHeight
            ),
            max
          );

  if (createImage) {
    findVoting_ = QImage(width, height, QImage::Format_RGB32);
    int color;
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        color = 0;
        for (int s = 0; s < numberScalingSteps; ++s) {
          color = qMax(color, qRound((double)accumulator.get(x, y, s)/max * 255));
        }
        findVoting_.setPixel(x, y, qRgb(color, color, color));
      }
    }
    issuePartialTimingMessage("Built result image");
  }

  issueTimingMessage("Object detection");
}


void Detector::edgeThinning()
{
  timer_.start();
  int width(img_.width());
  int height(img_.height());

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

      angle = qGray(sobelAngles_.get(x, y));
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
  issueTimingMessage("Edge thinning");
}

int Detector::interpolate(int a, int b, int progress)
{
  return a + (a - b) * ((float) progress / 45);
}

void Detector::issueTimingMessage(QString message)
{
  issueTiming(QString("%1: %2 ms").arg(message).arg(QString::number(timer_.elapsed())));
  timer_.invalidate();
}

void Detector::issuePartialTimingMessage(QString message)
{
  issueTiming(QString("-- %1: %2 ms").arg(message).arg(QString::number(timer_.elapsed())));
}
