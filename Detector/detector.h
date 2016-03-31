#ifndef DETECTOR_H
#define DETECTOR_H

// Qt Includes
#include <QString>
#include <QImage>
#include <QMultiMap>
#include <QElapsedTimer>

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
  void findScaledObject(bool createImage = false);

signals:
  void issueMessage(QString message);
  void issueTiming(QString message);
  void itemFound(QRect position, int confidence);


private:
  int interpolate(int a, int b, int progress);
  int offset(int x, int y, int z, int xSize, int ySize);
  int offset(int x, int y, int xSize);
  void issueTimingMessage(QString message);
  void issuePartialTimingMessage(QString message);

private:
  QString file_;
  QImage img_;
  QImage sobelAngles_;
  QImage findVoting_;
  QMultiMap<int, QPair<double, double> > rTable_;
  QSize trainingSize_;

  const QSize imgSize_;

  static const int LOWER_EDGE_THRESHOLD_ = 100;
  static const double SCALING_MAX_ = 0.7;
  static const double SCALING_MIN_ = 0.1;
  static const double SCALING_STEP_ = 0.05;

  QElapsedTimer timer_;
};

#endif // DETECTOR_H
