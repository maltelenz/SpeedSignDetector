#ifndef DETECTOR_H
#define DETECTOR_H

// Qt Includes
#include <QString>
#include <QImage>
#include <QMultiMap>
#include <QElapsedTimer>

// Detector Includes
#include "arrays.h"
#include "detection.h"

class Detector : public QObject
{
  Q_OBJECT

public:
  Detector();
  Detector(QString file);

  enum Speed { NoSpeed, Thirty, Forty, Fifty, Sixty, Seventy, Eighty, Ninety, Hundred, HundredTen, HundredTwenty };
  void initialize();

  void loadImage();
  void loadImage(QString file);

  QPixmap getPixmap();
  QPixmap getSobelAnglePixmap();

  QRect getImageSize();

  void blurred();
  void sobelEdges();
  void edgeThinning();

  void train(QString trainingFolder);
  void detect(bool colorElimination);

  void generateRTable(Speed speed);

  QList<Detection> findNoSpeedObject(int numberObjects = 10);
  QMap<Speed, double> detectSpeed(Detection detection);

  void eliminateColors(double greenfactor, double bluefactor);

  QRgb getColor(QPoint point);

signals:
  void issueMessage(QString message);
  void issueVerboseMessage(QString message);
  void issueTiming(QString message);
  void itemFound(QRect position, int confidence, int order);
  void speedFound(QRect position, double confidence, Detector::Speed speed);


private:
  int interpolate(int a, int b, int progress);
  void issueTimingMessage(QString message);
  void issuePartialTimingMessage(QString message);

  void checkNeighborPixel(bool isEdge, bool *currentlyEdge, int *n, int *s);
  QList<Detection> findObject(int numberObjects, double scalingMin, double scalingMax, int nScalings, QMultiMap<int, QPair<double, double> > rTable, QRect detectionArea);

public:
  QMap<Speed, QString> speeds_;

private:
  QString file_;
  QImage img_;
  Array2D sobelAngles_;
  QMap<Speed, QMultiMap<int, QPair<double, double> > > rTables_;
  QMap<Speed, QSize> trainingSize_;

  int edgeThreshold_;

  QSize imgSize_;

  double signMaxSize_;
  double signMinSize_;
  double numberScalings_;

  QElapsedTimer timer_;
};

#endif // DETECTOR_H
