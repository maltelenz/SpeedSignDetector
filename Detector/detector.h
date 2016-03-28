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

  double positiveMod(double x, double y);
};

#endif // DETECTOR_H
