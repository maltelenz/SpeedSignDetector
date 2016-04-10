#include "mainwindow.h"
#include "ui_mainwindow.h"

// Qt Includes
#include <QFileDialog>
#include <QLabel>
#include <QDebug>
#include <QGraphicsTextItem>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  detectionColor1_(0, 171, 0),
  detectionColor2_(255, 255, 84),
  detectionColor3_(250, 122, 42),
  detectionColor4_(250, 53, 42),
  detectionColor5_(115, 119, 255)
{
  ui->setupUi(this);

  scene_.addText("Open an image...");

  ui->graphicsView->setScene(&scene_);

  statusLabel_ = new QLabel("");
  ui->statusBar->addPermanentWidget(statusLabel_);

  detector_.initialize();

  connect(&detector_, SIGNAL(issueMessage(QString)), this, SLOT(on_issueMessage(QString)));
  connect(&detector_, SIGNAL(issueVerboseMessage(QString)), this, SLOT(on_issueMessage(QString)));
  connect(&detector_, SIGNAL(issueTiming(QString)), this, SLOT(on_issueTiming(QString)));
  connect(&detector_, SIGNAL(itemFound(QRect, int, int)), this, SLOT(on_itemFound(QRect, int, int)));

  on_issueMessage(QString("Startup successful."));
  on_issueMessage(QString("Click Train to select the training directory."));
}

MainWindow::~MainWindow()
{
  delete ui;
  delete statusLabel_;
}

void MainWindow::on_actionLoad_Image_triggered()
{
  QString fileName(
        QFileDialog::getOpenFileName(this,
           tr("Open Image"), QString(), tr("Images (*.png *.jpg)")));
  if (fileName.isNull()) {
    // User pressed cancel
    return;
  }
  detector_.loadImage(fileName);
  scene_.clear();
  scene_.addPixmap(detector_.getPixmap());
  scene_.setSceneRect(detector_.getImageSize());

  detector_.detect(true);

  ui->actionReset->setEnabled(true);
  ui->actionBlur->setEnabled(true);
  ui->actionEdges->setEnabled(true);
  ui->actionEliminate_Colors->setEnabled(true);

  connect(&scene_, SIGNAL(mouseReleased(QRectF)), this, SLOT(on_selectionReleased(QRectF)));
  connect(&scene_, SIGNAL(mouseMoved(QPointF)), this, SLOT(on_mouseMoved(QPointF)));
}

void MainWindow::on_mouseMoved(QPointF point)
{
  QRgb pixelColor(detector_.getColor(point.toPoint()));
  QString txt = QString("(%1, %2) - (%3,%4,%5)").arg(
        QString::number(point.x())).arg(
        QString::number(point.y())).arg(
        QString::number(qRed(pixelColor))).arg(
        QString::number(qGreen(pixelColor))).arg(
        QString::number(qBlue(pixelColor)));
  statusLabel_->setText(txt);
}

void MainWindow::on_issueMessage(QString message)
{
  ui->messagesTextEdit->appendPlainText(message);
}

void MainWindow::on_issueTiming(QString message)
{
  ui->timingsTextEdit->appendPlainText(message);
}

void MainWindow::on_itemFound(QRect position, int confidence, int order)
{
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

void MainWindow::refetchImage()
{
  scene_.clear();
  scene_.addPixmap(detector_.getPixmap());
}

void MainWindow::on_actionReset_triggered()
{
  ui->actionShowAngles->setEnabled(false);
  ui->actionEdge_Thinning->setEnabled(false);
  detector_.loadImage();
  refetchImage();
}

void MainWindow::on_actionBlur_triggered()
{
  detector_.blurred();
  refetchImage();
}

void MainWindow::on_actionEdges_triggered()
{
  detector_.sobelEdges();
  ui->actionShowAngles->setEnabled(true);
  ui->actionEdge_Thinning->setEnabled(true);
  refetchImage();
}

void MainWindow::on_actionShowAngles_triggered(bool on)
{
  if (on) {
    scene_.clear();
    scene_.addPixmap(detector_.getSobelAnglePixmap());
  } else {
    refetchImage();
  }
}

void MainWindow::on_actionEdge_Thinning_triggered()
{
  detector_.edgeThinning();
  refetchImage();
}

void MainWindow::on_actionQuit_triggered()
{
  QCoreApplication::quit();
}

void MainWindow::on_actionTrain_triggered()
{
  QString dir(
        QFileDialog::getExistingDirectory(this,
           tr("Training Directory")));
  if (dir.isNull()) {
    // User pressed cancel
    return;
  }
  detector_.train(dir + "/");
}

void MainWindow::on_actionEliminate_Colors_triggered()
{
    detector_.eliminateColors(1, 1.2);
    refetchImage();
}
