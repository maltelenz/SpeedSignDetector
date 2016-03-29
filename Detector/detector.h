#ifndef DETECTOR_H
#define DETECTOR_H

// Qt Includes
#include <QString>
#include <QImage>
#include <QMultiMap>

class Detector
{

public:
  Detector();
  Detector(QString file);

  void loadImage();
  void loadImage(QString file);

  QPixmap getPixmap();
  QPixmap getSobelAnglePixmap();
  QPixmap getObjectHeatmapPixmap();

  QRect getImageSize();

  QImage averageLines();
  QColor averageSection(int xStart, int yStart, int xStop, int yStop);
  void blurred();
  void sobelEdges();
  void edgeThinning();

  void generateRTable();
  void findObject();

private:
  QString file_;
  QImage img_;
  QImage sobelAngles_;
  QImage findVoting_;
  QMultiMap<int, QPair<double, double> > rTable_;

  const QSize imgSize_;

  static const int LOWER_EDGE_THRESHOLD_ = 100;

  int interpolate(int a, int b, int progress);
};

#endif // DETECTOR_H
