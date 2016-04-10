#include "detectortask.h"

#include <QDebug>
#include <QPainter>
#include <QGraphicsTextItem>
#include <QDir>
#include <QFileInfo>

DetectorTask::DetectorTask(QObject *parent) :
  out_(stdout),
  detectionColor1_(0, 171, 0),
  detectionColor2_(255, 255, 84),
  detectionColor3_(250, 122, 42),
  detectionColor4_(250, 53, 42),
  detectionColor5_(115, 119, 255)
{
  detector_.initialize();
  connect(&detector_, SIGNAL(issueMessage(QString)), this, SLOT(on_issueMessage(QString)));
  connect(&detector_, SIGNAL(issueVerboseMessage(QString)), this, SLOT(on_issueVerboseMessage(QString)));
  connect(&detector_, SIGNAL(issueTiming(QString)), this, SLOT(on_issueTiming(QString)));
  connect(&detector_, SIGNAL(speedFound(QRect, double, Detector::Speed)), this, SLOT(on_speedFound(QRect, double, Detector::Speed)));
}

void DetectorTask::setTrainingDirectory(QString trainingDirectory)
{
  trainingDirectory_ = trainingDirectory;
}

void DetectorTask::setTargetFile(QString targetFile)
{
  targetFile_ = targetFile;
}

void DetectorTask::setResultFile(QString resultFile)
{
  resultFile_ = resultFile;
}

void DetectorTask::setColorElimination(bool colorElimination)
{
  colorElimination_ = colorElimination;
}

void DetectorTask::setVerbose(bool verbose)
{
  verbose_ = verbose;
}

void DetectorTask::detectInImage(QString rFile, QString file)
{
  out_ << endl << QString("Loading target image from %1").arg(file) << endl;
  detector_.loadImage(file);

  if (!rFile.isEmpty()) {
    scene_.clear();
    scene_.addPixmap(detector_.getPixmap());
    scene_.setSceneRect(detector_.getImageSize());
  }

  detector_.detect(colorElimination_);

  if (!rFile.isEmpty()) {
    out_ << endl << QString("Saving output image to %1").arg(rFile) << endl;
    QImage image(scene_.sceneRect().size().toSize(), QImage::Format_RGB32);
    QPainter painter(&image);
    scene_.render(&painter);
    image.save(rFile);
  }
}

void DetectorTask::run()
{
  detector_.train(trainingDirectory_);

  QStringList supportedImageFilter;
  supportedImageFilter << "*.jpg" << "*.JPG" << "*.JPEG" << "*.jpeg" << "*.png";

  QStringList targetFiles;
  QStringList resultFiles;
  if (QFileInfo(targetFile_).isDir()) {
    if (!QFileInfo(resultFile_).isDir()) {
      out_ << "Target file is a directory, but result file is not.";
      emit finished();
      return;
    }
    QFileInfoList fiList(QDir(targetFile_).entryInfoList(supportedImageFilter, QDir::Files));
    foreach (QFileInfo f, fiList) {
      targetFiles << f.filePath();
      resultFiles << resultFile_ + f.fileName();
    }
  } else {
    targetFiles << targetFile_;
    if (QFileInfo(resultFile_).isDir()) {
      resultFiles << resultFile_ + QFileInfo(targetFile_).fileName();
    } else {
      resultFiles << resultFile_;
    }
  }

  for (int i = 0; i < targetFiles.length(); ++i) {
    detectInImage(resultFiles.at(i), targetFiles.at(i));
  }

  emit finished();
}

void DetectorTask::on_issueMessage(QString message)
{
  out_ << message << endl;
}

void DetectorTask::on_issueVerboseMessage(QString message)
{
  if (verbose_) {
    on_issueMessage(message);
  }
}

void DetectorTask::on_issueTiming(QString message)
{
  if (verbose_) {
    out_ << "Timing: " << message << endl;
  }
}

void DetectorTask::on_speedFound(QRect position, double confidence, Detector::Speed speed)
{
  if (resultFile_.isEmpty()) {
    // Ignore items found, a separate message is already issued by the Detector.
    return;
  }
  scene_.addRect(position, QPen(detectionColor1_));
  QGraphicsTextItem* ctext = scene_.addText(QString::number(confidence));
  QGraphicsTextItem* stext = scene_.addText(detector_.speeds_.value(speed));
  ctext->setPos(position.topLeft());
  stext->setPos(position.bottomRight());
  ctext->setDefaultTextColor(Qt::white);
  stext->setDefaultTextColor(Qt::white);
}
