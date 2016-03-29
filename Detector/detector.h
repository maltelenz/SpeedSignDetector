#ifndef DETECTOR_H
#define DETECTOR_H

// Qt Includes
#include <QString>
#include <QImage>

class Detector
{

public:
  Detector();
  Detector(QString file);

  void loadImage();
  void loadImage(QString file);

  QPixmap getPixmap();
  QPixmap getSobelAnglePixmap();

  QRect getImageSize();

  QImage averageLines();
  QColor averageSection(int xStart, int yStart, int xStop, int yStop);
  void blurred();
  void sobelEdges();
  void edgeThinning();

private:
  QString file_;
  QImage img_;
  QImage sobelAngles_;
  const QSize imgSize_;
  static const int LOWER_EDGE_THRESHOLD_ = 100;

  int interpolate(int a, int b, int progress);
};

#endif // DETECTOR_H
