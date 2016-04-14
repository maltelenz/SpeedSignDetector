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

  void setMode(QString mode);
  void setTrainingDirectory(QString trainingDirectory);
  void setTargetFile(QString targetFile);
  void setResultFile(QString resultFile);

  void setColorElimination(bool colorElimination);
  void setVerbose(bool verbose);

private:
  void loadTrainingImage(QString file);
  void detectInImage(QString rFile, QString file);

public slots:
    void run();

    void on_issueMessage(QString message);
    void on_issueVerboseMessage(QString message);
    void on_issueTiming(QString message);
    void on_speedFound(QRect position, double confidence, Detector::Speed speed);

signals:
    void finished();

private:
  QString mode_;
  QString trainingDirectory_;
  QString targetFile_;
  QString resultFile_;

  bool colorElimination_;
  bool verbose_;

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
