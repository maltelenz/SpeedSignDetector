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
#include "detection.h"

Detector::Detector() :
  imgSize_(700, 700),
  SCALING_MAX_(0.9),
  SCALING_MIN_(0.1),
  SCALING_STEP_(0.05)
{
}

Detector::Detector(QString file) :
  imgSize_(700, 700),
  SCALING_MAX_(0.9),
  SCALING_MIN_(0.1),
  SCALING_STEP_(0.05),
  file_(file)
{
}

void Detector::loadImage()
{
  timer_.start();

  img_ = QImage(file_);
  if (img_.width() > imgSize_.width() || img_.height() > imgSize_.height()) {
    img_ = img_.scaled(imgSize_, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  }
  findVoting_ = QImage();

  issueVerboseMessage(QString("Loaded image with size: (%1, %2)").arg(img_.width()).arg(img_.height()));

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
  blur->setBlurRadius(1.1);

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

void Detector::generateRTable(bool clearFirst)
{
  timer_.start();
  QMultiMap<int, QPair<double, double> > rTable;

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
        rTable.insert(angleOfEdge, QPair<double, double>(angleToEdge, distance));
      }
    }
  }
//  qDebug() << rTable;

  if (clearFirst) {
    rTables_.clear();
    trainingSources_.clear();
  }
  rTables_.append(rTable);
  trainingSources_.append(file_);

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

  QMultiMap<int, QPair<double, double> > rTable;
  foreach (rTable, rTables_) {
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
          QList<QPair<double, double> > values = rTable.values(angle);
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
              max,
              1
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
  }

  issueTimingMessage("Object detection");
}


void Detector::findScaledObject(bool createImage, int numberObjects)
{
  timer_.start();
  if (!findVoting_.isNull()) {
    // Already have looked for the object.
    return;
  }

  int width(img_.width());
  int height(img_.height());
  int numberScalingSteps(qCeil((SCALING_MAX_ - SCALING_MIN_) / SCALING_STEP_));

  issueVerboseMessage(QString("Finding objects with %1 scalings from %2 to %3.").arg(
                 numberScalingSteps).arg(
                 SCALING_MIN_).arg(
                 SCALING_MAX_));

  QMultiMap<int, QPair<double, double> > rTable;
  for (int ri = 0; ri < rTables_.length(); ++ri) {
    rTable = rTables_.at(ri);

    Array3D accumulator(width, height, numberScalingSteps);
    if (!accumulator.init()) {
      // Failed to allocate memory, abort nicely
      issueMessage("Failed to allocate memory for the accumulator in Detector::findScaledObject.");
      timer_.invalidate();
      return;
    }
    int pixelColor;
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
          pixelColor = qGray(line[x]);
          if (pixelColor <= 0) {
            // Black, not an edge
            continue;
          }
          // Not black, check the R-table
          angle = qGray(sobelAngles_.get(x, y));
          foreach (v, rTable.values(angle)) {
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

    QList<Detection> maxList;
    for (int i = 0; i < numberObjects; ++i) {
      maxList.append(Detection());
    }
    int val;
    Detection smallest;
    int foundWidth, foundHeight;
    QRect foundRect;
  //  QStringList dump;
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        for (int s = 0; s < numberScalingSteps; ++s) {
          val = accumulator.get(x, y, s);
          if (val > smallest.confidence_) {
            maxList.removeOne(smallest);
            foundWidth = (SCALING_MIN_ + s * SCALING_STEP_) * trainingSize_.width();
            foundHeight = (SCALING_MIN_ + s * SCALING_STEP_) * trainingSize_.height();
            foundRect = QRect(
                x - foundWidth / 2,
                y - foundHeight / 2,
                foundWidth,
                foundHeight
              ),
            maxList.append(Detection(foundRect, val));
            qSort(maxList);
            smallest = maxList.at(0);
          }
  //      dump << QString::number(accumulator[x][y][s]);
        }
      }
    }
  //  qDebug() << dump;
  //  qDebug() << maxList;
    issuePartialTimingMessage("Isolated max");

    issueMessage(QString("Using training data from: %1").arg(trainingSources_.at(ri)));

    Detection d;
    for (int i = 0; i < numberObjects; ++i) {
      d = maxList.at(i);

      QString msg1(QString("Found object with confidence: %1").arg(d.confidence_));
      QString msg2(QString("-- At: (%1, %2), %3x%4").arg(
                     d.box_.center().x()).arg(
                     d.box_.center().y()).arg(
                     d.box_.width()).arg(
                     d.box_.height()));
      if (i == numberObjects - 1) {
        issueMessage(msg1);
        issueMessage(msg2);
      } else {
        issueVerboseMessage(msg1);
        issueVerboseMessage(msg2);
      }
      emit itemFound(d.box_, d.confidence_, numberObjects - i);
    }

    if (createImage) {
      findVoting_ = QImage(width, height, QImage::Format_RGB32);
      int color;
      int max = maxList.last().confidence_;
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
  }

  issueTimingMessage("Object detection");
}

void Detector::eliminateColors(double greenfactor, double bluefactor)
{
  timer_.start();

  int width(img_.width());
  int height(img_.height());

  QImage res(img_);

  int i,j;
  QRgb color;
  int red, green, blue;
  int removalVote;

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      // Ignore outermost border, so we can have an easier/faster checking below
      if( y <= 0 || y >= height - 1 || x <= 0 || x >= width - 1 ) {
        continue;
      }
      removalVote = 0;
      for ( i = -1; i <= 1; i++) {
        for (j = -1; j <= 1; j++) {
          color = img_.pixel(x + i, y + j);
          red = qRed(color);
          green = qGreen(color);
          blue = qBlue(color);
          if (red < green * greenfactor || red < blue * bluefactor) {
            removalVote++;
          }
        }
      }
      if (removalVote > 3) {
        res.setPixel(x, y, qRgb(0, 0, 0));
      }
    }
  }
  img_ = res;
  issueTimingMessage("Color elimination");
}

QRgb Detector::getColor(QPoint point)
{
  return img_.pixel(point);
}


void Detector::checkNeighborPixel(bool isEdge, bool* currentlyEdge, int* n, int* s)
{
  if (isEdge) {
    (*n)++;
    if (!(*currentlyEdge)) {
      (*s)++;
      (*currentlyEdge) = true;
    }
  } else if ((*currentlyEdge)) {
    (*s)++;
    (*currentlyEdge) = false;
  }
}

void Detector::edgeThinning()
{
  timer_.start();
  int width(img_.width());
  int height(img_.height());

  int n;
  int s;
  bool currentlyEdge;
  int pixelColor;
  bool neighbors[8];
  bool changed = true;


  while (changed) {
    changed = false;
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

        n = 0;
        s = 0;
        neighbors[0] = qGray(img_.pixel(x, y - 1)) > 0;
        neighbors[1] = qGray(img_.pixel(x + 1, y - 1)) > 0;
        neighbors[2] = qGray(img_.pixel(x + 1, y)) > 0;
        neighbors[3] = qGray(img_.pixel(x + 1, y + 1)) > 0;
        neighbors[4] = qGray(img_.pixel(x, y + 1)) > 0;
        neighbors[5] = qGray(img_.pixel(x - 1, y + 1)) > 0;
        neighbors[6] = qGray(img_.pixel(x - 1, y)) > 0;
        neighbors[7] = qGray(img_.pixel(x - 1, y - 1)) > 0;

        currentlyEdge = false;
        if (neighbors[0] > 0) {
          n++;
          currentlyEdge = true;
        }
        checkNeighborPixel(neighbors[1], &currentlyEdge, &n, &s);
        checkNeighborPixel(neighbors[2], &currentlyEdge, &n, &s);
        checkNeighborPixel(neighbors[3], &currentlyEdge, &n, &s);
        checkNeighborPixel(neighbors[4], &currentlyEdge, &n, &s);
        checkNeighborPixel(neighbors[5], &currentlyEdge, &n, &s);
        checkNeighborPixel(neighbors[6], &currentlyEdge, &n, &s);
        checkNeighborPixel(neighbors[7], &currentlyEdge, &n, &s);

        if (
            n == 0 || // Standalone pixel is removed
            (n > 1 && // End of a line is kept
            n <= 6 && // Interior points are kept
            s < 3) // Bridge pixels are kept
        ) {
          // Kill pixel
          img_.setPixel(x, y, qRgb(0, 0, 0));
          changed = true;
        }
      }
    }
  }

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
