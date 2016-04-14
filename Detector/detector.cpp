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

Detector::Detector()
{
}

Detector::Detector(QString file) :
  file_(file)
{
}

void Detector::initialize()
{
  edgeThreshold_ = 50;
  harrisThreshold_ = 10000000;
  imgSize_ = QSize(600, 600);
  signMaxSize_ = qRound(imgSize_.width() * 0.1);
  signMinSize_ = qRound(imgSize_.width() * 0.03);
  numberScalings_ = 20;

  speeds_.insert(NoSpeed, "nospeed");
  speeds_.insert(Thirty, "30");
  speeds_.insert(Forty, "40");
  speeds_.insert(Fifty, "50");
  speeds_.insert(Sixty, "60");
  speeds_.insert(Seventy, "70");
  speeds_.insert(Eighty, "80");
  speeds_.insert(Ninety, "90");
  speeds_.insert(Hundred, "100");
  speeds_.insert(HundredTen, "110");
  speeds_.insert(HundredTwenty, "120");
}

void Detector::setEdgeThreshold(double threshold)
{
  edgeThreshold_ = threshold;
}

void Detector::setHarrisThreshold(double threshold)
{
  harrisThreshold_ = threshold;
}

void Detector::loadImage()
{
  timer_.start();

  img_ = QImage(file_);
  if (img_.width() > imgSize_.width() || img_.height() > imgSize_.height()) {
    img_ = img_.scaled(imgSize_, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  }

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

QRect Detector::getImageSize()
{
  return img_.rect();
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
  issueTimingMessage("Blur");
}

void Detector::sobelEdges()
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
      if (sum < edgeThreshold_) {
        sum = 0;
      }
      res.setPixel(x, y, qRgb(sum, sum, sum));
      sobelAngles_.set(x, y, angle);
    }
  }
  img_ = res;
  issueTimingMessage("Edge detection");
}

void Detector::harrisCorners()
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
    issueMessage("Failed to allocate memory for the harrisCorners_ in Detector::harrisCorners.");
    timer_.invalidate();
    return;
  }

  int i, j;
  long sumX, sumY;
  int sum;
  uint color;

  double phi;
  int angle;

  double r;
  double ix2, iy2, ixy, sx2, sy2, sxy;
  double detH;
  double kTraceH2;
  double k(0.05);

  issueVerboseMessage(QString("Harris threshold is: %1").arg(harrisThreshold_));

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      // Make outermost border black, so we can have an easier/faster for loop below
      if( y <= 0 || y >= height - 1 || x <= 0 || x >= width - 1 ) {
        r = 0;
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
        // Compute angle/gradient
        sum = qAbs(sumX) + qAbs(sumY);
        sum = qMin(sum, 255);
        phi = atan2upperHalfPlane(sumY, sumX);
        // Angle is between 0 and 180 degrees, where 0 is E/W, 45 is NE/SW and 90 is N/S
        angle = qRound(qRadiansToDegrees(phi));

        // Harris corner algorithm description from
        // http://www.cse.psu.edu/~rtc12/CSE486/lecture06.pdf
        ix2 = sumX * sumX;
        iy2 = sumY * sumY;
        ixy = sumX * sumY;
        // Using simplified version without windowing
        sx2 = ix2;
        sy2 = iy2;
        sxy = ixy;
        detH = sx2 * sy2 - sxy * sxy;
        kTraceH2 = k * (sx2 + sy2) * (sx2 + sy2);
        r = qAbs(qRound(detH - kTraceH2));
        if (r < harrisThreshold_) {
          r = 0;
        } else {
          r = 255;
        }
      }
      res.setPixel(x, y, qRgb(r, r, r));
      sobelAngles_.set(x, y, angle);
    }
  }
  img_ = res;
  issueTimingMessage("Harris corners");
}

void Detector::train(QString trainingFolder) {
  issueVerboseMessage("Training...");

  rTables_.clear();
  QString imgFilePath;
  foreach (Speed s, speeds_.keys()) {
    imgFilePath = trainingFolder + "training-" + speeds_.value(s) + ".png";
    issueMessage(QString("Loading training image %1").arg(imgFilePath));
    loadImage(imgFilePath);
    sobelEdges();
    edgeThinning();
    generateRTable(s);
  }
}

void Detector::trainHarris(QString trainingFolder) {
  issueVerboseMessage("Training using Harris corners...");

  rTables_.clear();
  QString imgFilePath;
  foreach (Speed s, speeds_.keys()) {
    imgFilePath = trainingFolder + "training-" + speeds_.value(s) + ".png";
    issueMessage(QString("Loading training image %1").arg(imgFilePath));
    loadImage(imgFilePath);
    harrisCorners();
    generateRTable(s);
  }
}

void Detector::detect(bool colorElimination)
{
  issueVerboseMessage("Detecting...");
  if (colorElimination) {
    issueVerboseMessage("Eliminating colors...");
    eliminateColors(1, 1.2);
  }
  sobelEdges();
  edgeThinning();
  QList<Detection> noSpeedDetections = findNoSpeedObject(1);
  foreach (Detection d, noSpeedDetections) {
    detectSpeed(d);
  }
}

void Detector::detectHarris(bool colorElimination)
{
  issueVerboseMessage("Detecting...");
  if (colorElimination) {
    issueVerboseMessage("Eliminating colors...");
    eliminateColors(1, 1.2);
  }
  harrisCorners();
  QList<Detection> noSpeedDetections = findNoSpeedObject(1);
  foreach (Detection d, noSpeedDetections) {
    detectSpeed(d);
  }
}

void Detector::generateRTable(Speed speed)
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

  rTables_.insert(speed, rTable);

  trainingSize_.insert(speed, img_.size());
  issueTimingMessage("R-table generation");
}

QMap<Detector::Speed, double> Detector::detectSpeed(Detection detection)
{
  timer_.start();

  double lowerScalingFactor(0.8);
  double upperScalingFactor(1.2);
  int numberScalings(10);

  int width(img_.width());
  int height(img_.height());

  int ymin(qMax(detection.box_.top(), 0));
  int ymax(qMin(detection.box_.bottom(), height));
  int xmin(qMax(detection.box_.left(), 0));
  int xmax(qMin(detection.box_.right(), width));

  issueVerboseMessage(QString("Looking at (%1,%2) -> (%3,%4) for speed.").arg(xmin).arg(ymin).arg(xmax).arg(ymax));

  QMap<Detector::Speed, double> maxMap;
  Speed maxSpeed;
  double maxConfidence(0);

  QRect enlargedBox(
        detection.box_.left() - (upperScalingFactor - 1) * detection.box_.width(),
        detection.box_.top() - (upperScalingFactor - 1) * detection.box_.height(),
        upperScalingFactor * detection.box_.width(),
        upperScalingFactor * detection.box_.height());

  QMultiMap<int, QPair<double, double> > rTable;
  foreach (Speed speed, rTables_.keys()) {
    if (speed == NoSpeed) {
      // Do not detect empty signs.
      continue;
    }
    rTable = rTables_.value(speed);

    double detectedScaling((double)detection.box_.width()/trainingSize_.value(speed).width());

    issueVerboseMessage(QString("Looking for speed %1.").arg(speeds_.value(speed)));

    QList<Detection> maxList = findObject(
          1,
          lowerScalingFactor * detectedScaling,
          upperScalingFactor * detectedScaling,
          numberScalings,
          rTables_.value(speed),
          enlargedBox);

    double confidence((double)maxList.first().confidence_/rTable.size());
    issuePartialTimingMessage("Isolated max");

    issueMessage(QString("Found %1 with value %2 and confidence %3 at (%4,%5)").arg(
                   speeds_.value(speed)).arg(
                   maxList.first().confidence_).arg(
                   confidence).arg(
                   maxList.first().box_.center().x()).arg(
                   maxList.first().box_.center().y()));
    maxMap.insert(speed, confidence);
    if (confidence > maxConfidence) {
      maxConfidence = confidence;
      maxSpeed = speed;
    }
    emit itemFound(detection.box_, confidence, 1);
  }

  emit speedFound(detection.box_, maxConfidence, maxSpeed);

  issueTimingMessage("Speed detection");
  return maxMap;
}

QList<Detection> Detector::findNoSpeedObject(int numberObjects)
{
  timer_.start();

  double scalingMin(signMinSize_/trainingSize_.value(NoSpeed).width());
  double scalingMax(signMaxSize_/trainingSize_.value(NoSpeed).width());

  QList<Detection> maxList = findObject(numberObjects, scalingMin, scalingMax, numberScalings_, rTables_.value(NoSpeed), img_.rect());

  issueTimingMessage("Sign detection");
  return maxList;
}

QList<Detection> Detector::findObject(
      int numberObjects,
      double scalingMin,
      double scalingMax,
      int nScalings,
      QMultiMap<int, QPair<double, double> > rTable,
      QRect detectionArea
    )
{

  int width(img_.width());
  int height(img_.height());

  int ymin(qMax(detectionArea.top(), 0));
  int ymax(qMin(detectionArea.bottom(), height));
  int xmin(qMax(detectionArea.left(), 0));
  int xmax(qMin(detectionArea.right(), width));

  double scalingStep((scalingMax - scalingMin)/(nScalings - 1));

  Array3D accumulator(width, height, nScalings);
  if (!accumulator.init()) {
    // Failed to allocate memory, abort nicely
    issueMessage("Failed to allocate memory for the accumulator in Detector::findObject.");
    return QList<Detection>();
  }
  int pixelColor;
  int angle;

  double xcp, ycp;
  int xc, yc;

  QPair<double, double> v;
  issuePartialTimingMessage("Allocated datastructures");

  for (int y = ymin; y < ymax; ++y) {
    const QRgb* line = (const QRgb *)img_.constScanLine(y);
    for (int x = xmin; x < xmax; ++x) {
      if( y <= ymin || y >= ymax - 1 || x <= xmin || x >= xmax - 1 ) {
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
          for (int s = 0; s < nScalings; ++s) {
            xc = qRound(x + xcp * (scalingMin + s * scalingStep));
            yc = qRound(y + ycp * (scalingMin + s * scalingStep));
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
  for (int y = ymin; y < ymax; ++y) {
    for (int x = xmin; x < xmax; ++x) {
      for (int s = 0; s < nScalings; ++s) {
        val = accumulator.get(x, y, s);
        if (val > smallest.confidence_) {
          maxList.removeOne(smallest);
          foundWidth = (scalingMin + s * scalingStep) * trainingSize_.value(NoSpeed).width();
          foundHeight = (scalingMin + s * scalingStep) * trainingSize_.value(NoSpeed).height();
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
  }

  return maxList;
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
  if (img_.valid(point)) {
    return img_.pixel(point);
  }
  return qRgb(0, 0, 0);
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

  QList<QPoint> toKill;
  while (changed) {
    changed = false;
    for (int direction = 0; direction < 7; direction+=2) {
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

          /*
           * This is the indexing of the neighbors
           *  -------------
           *  | 7 | 0 | 1 |
           *  -------------
           *  | 6 | X | 2 |
           *  -------------
           *  | 5 | 4 | 3 |
           *  -------------
           *
          */

          neighbors[0] = qGray(img_.pixel(x, y - 1)) > 0;
          neighbors[1] = qGray(img_.pixel(x + 1, y - 1)) > 0;
          neighbors[2] = qGray(img_.pixel(x + 1, y)) > 0;
          neighbors[3] = qGray(img_.pixel(x + 1, y + 1)) > 0;
          neighbors[4] = qGray(img_.pixel(x, y + 1)) > 0;
          neighbors[5] = qGray(img_.pixel(x - 1, y + 1)) > 0;
          neighbors[6] = qGray(img_.pixel(x - 1, y)) > 0;
          neighbors[7] = qGray(img_.pixel(x - 1, y - 1)) > 0;

          if (neighbors[direction] > 0) {
            // Not removing pixels in the current direction
            continue;
          }

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
            toKill.append(QPoint(x, y));
            changed = true;
          }
        }
      }
      foreach (QPoint p, toKill) {
        img_.setPixel(p.x(), p.y(), qRgb(0, 0, 0));
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
