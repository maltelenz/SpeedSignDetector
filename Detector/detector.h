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
  QRect getImageSize();

  QImage averageLines();
  QColor averageSection(int xStart, int yStart, int xStop, int yStop);
  QImage blurred();

private:
  QString file_;
  QImage img_;
  const QSize imgSize_;

};

#endif // DETECTOR_H
