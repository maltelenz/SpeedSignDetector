#ifndef DETECTORTASK_H
#define DETECTORTASK_H

// Qt Includes
#include <QObject>
#include <QTextStream>
#include <QGraphicsScene>
#include <QColor>

// SpeedSignDetector Includes
#include "detector.h"

class DetectorTask : public QObject
{
  Q_OBJECT
public:
  explicit DetectorTask(QObject *parent = 0);

  void setTrainingFile(QString trainingFile);
  void setTargetFile(QString targetFile);
  void setResultFile(QString resultFile);

public slots:
    void run();

    void on_issueMessage(QString message);
    void on_issueTiming(QString message);
    void on_itemFound(QRect position, int confidence, int order);

signals:
    void finished();

private:
  QString trainingFile_;
  QString targetFile_;
  QString resultFile_;
  int edgeThreshold_;

  QGraphicsScene scene_;

  QColor detectionColor1_;
  QColor detectionColor2_;
  QColor detectionColor3_;
  QColor detectionColor4_;
  QColor detectionColor5_;

  Detector detector_;
  QTextStream out_;
};

#endif // DETECTORTASK_H
