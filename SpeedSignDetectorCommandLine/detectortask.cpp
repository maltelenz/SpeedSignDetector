#include "detectortask.h"

#include <QDebug>
#include <QPainter>
#include <QGraphicsTextItem>
#include <QDir>
#include <QFileInfo>

DetectorTask::DetectorTask(QObject *parent) :
  out_(stdout),
  edgeThreshold_(100),
  detectionColor1_(0, 171, 0),
  detectionColor2_(255, 255, 84),
  detectionColor3_(250, 122, 42),
  detectionColor4_(250, 53, 42),
  detectionColor5_(115, 119, 255)
{
  connect(&detector_, SIGNAL(issueMessage(QString)), this, SLOT(on_issueMessage(QString)));
  connect(&detector_, SIGNAL(issueTiming(QString)), this, SLOT(on_issueTiming(QString)));
  connect(&detector_, SIGNAL(itemFound(QRect, int, int)), this, SLOT(on_itemFound(QRect, int, int)));
}

void DetectorTask::setTrainingFile(QString trainingFile)
{
  trainingFile_ = trainingFile;
}

void DetectorTask::setTargetFile(QString targetFile)
{
  targetFile_ = targetFile;
}

void DetectorTask::setResultFile(QString resultFile)
{
  resultFile_ = resultFile;
}

void DetectorTask::loadTrainingImage(QString file)
{
  out_ << endl << QString("Loading training image from %1").arg(file) << endl;
  detector_.loadImage(file);
  out_ << "Training..." << endl;
  detector_.sobelEdges(edgeThreshold_);
  detector_.generateRTable();
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

  out_ << "Detecting..." << endl;
  detector_.sobelEdges(edgeThreshold_);
  detector_.findScaledObject();

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
  loadTrainingImage(trainingFile_);

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

void DetectorTask::on_issueTiming(QString message)
{
  out_ << "Timing: " << message << endl;
}

void DetectorTask::on_itemFound(QRect position, int confidence, int order)
{
  if (resultFile_.isEmpty()) {
    // Ignore items found, a separate message is already issued by the Detector.
    return;
  }
  QColor c;
  switch (order) {
  case 1:
    c = detectionColor1_;
    break;
  case 2:
    c = detectionColor2_;
    break;
  case 3:
    c = detectionColor3_;
    break;
  case 4:
    c = detectionColor4_;
    break;
  default:
    c = detectionColor5_;
    break;
  }
  scene_.addRect(position, QPen(c));
  QGraphicsTextItem* ctext = scene_.addText(QString::number(confidence));
  ctext->setPos(position.topLeft());
  ctext->setDefaultTextColor(c);
  QFont f;
  f.setBold(true);
  ctext->setFont(f);
}
