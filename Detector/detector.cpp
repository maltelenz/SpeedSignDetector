#include "detector.h"

//Qt includes
#include <QPixmap>

Detector::Detector()
{
}

Detector::Detector(QString file) :
  file_(file)
{
}

void Detector::loadImage()
{
  img_ = QImage(file_);
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
