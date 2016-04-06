#ifndef DETECTOR_H
#define DETECTOR_H

// Qt Includes
#include <QString>
#include <QImage>
#include <QMultiMap>
#include <QElapsedTimer>

// Detector Includes
#include "arrays.h"

class Detector : public QObject
{
  Q_OBJECT

public:
  Detector();
  Detector(QString file);

  void loadImage();
  void loadImage(QString file);

  QPixmap getPixmap();
  QPixmap getSobelAnglePixmap();
  QPixmap getObjectHeatmapPixmap();

  QRect getImageSize();

  QColor averageSection(int xStart, int yStart, int xStop, int yStop);
  void blurred();
  void sobelEdges(int lowerEdgeThreshold);
  void edgeThinning();

  void generateRTable();
  void findObject(bool createImage = false);
  void findScaledObject(bool createImage = false, int numberObjects = 10);

signals:
  void issueMessage(QString message);
  void issueTiming(QString message);
  void itemFound(QRect position, int confidence, int order);


private:
  int interpolate(int a, int b, int progress);
  void issueTimingMessage(QString message);
  void issuePartialTimingMessage(QString message);

private:
  QString file_;
  QImage img_;
  Array2D sobelAngles_;
  QImage findVoting_;
  QMultiMap<int, QPair<double, double> > rTable_;
  QSize trainingSize_;

  const QSize imgSize_;

  static const double SCALING_MAX_ = 0.9;
  static const double SCALING_MIN_ = 0.1;
  static const double SCALING_STEP_ = 0.05;

  QElapsedTimer timer_;
  void checkNeighborPixel(bool isEdge, bool *currentlyEdge, int *n, int *s);
};

#endif // DETECTOR_H
