#include "mainwindow.h"
#include "ui_mainwindow.h"

// Qt Includes
#include <QFileDialog>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  scene_.addText("Open an image...");

  ui->graphicsView->setScene(&scene_);

  statusLabel_ = new QLabel("");
  ui->statusBar->addPermanentWidget(statusLabel_);
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

  ui->actionMean_Lines->setEnabled(true);
  ui->actionPainter->setEnabled(true);
  ui->actionReset->setEnabled(true);
  ui->actionBlur->setEnabled(true);
  ui->actionEdges->setEnabled(true);

  connect(&scene_, SIGNAL(mouseReleased(QRectF)), this, SLOT(on_selectionReleased(QRectF)));
  connect(&scene_, SIGNAL(mouseMoved(QPointF)), this, SLOT(on_mouseMoved(QPointF)));
}

void MainWindow::on_actionMean_Lines_toggled(bool on)
{
  scene_.clear();
  if (on) {
    scene_.addPixmap(QPixmap::fromImage(detector_.averageLines()));
  } else {
    scene_.addPixmap(detector_.getPixmap());
  }
}

void MainWindow::on_actionPainter_triggered(bool on)
{
  if (!on) {
  } else {
  }
}

void MainWindow::on_selectionReleased(QRectF rectf)
{
  QRect nr(
        qMin(rectf.left(), rectf.right()),
        qMin(rectf.top(), rectf.bottom()),
        qAbs(rectf.width()),
        qAbs(rectf.height())
      );
  if (!detector_.getImageSize().contains(nr) || nr.width() == 0 || nr.height() == 0) {
    // Illegal rectangle, abort!
    return;
  }
  QColor mean(detector_.averageSection(nr.left(), nr.top(), nr.right(), nr.bottom()));
  scene_.addRect(nr, QPen(mean), QBrush(mean));
}

void MainWindow::on_mouseMoved(QPointF point)
{
  QString txt = QString("{%1, %2}").arg(QString::number(point.x()), QString::number(point.y()));
  statusLabel_->setText(txt);
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
  ui->actionR_Table->setEnabled(false);
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
  ui->actionR_Table->setEnabled(true);
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

void MainWindow::on_actionR_Table_triggered()
{
  detector_.generateRTable();
  ui->actionFind_Object->setEnabled(true);
}

void MainWindow::on_actionQuit_triggered()
{
  QCoreApplication::quit();
}

void MainWindow::on_actionFind_Object_triggered(bool on)
{
  detector_.findObject();
  if (on) {
    scene_.clear();
    scene_.addPixmap(detector_.getObjectHeatmapPixmap());
  } else {
    refetchImage();
  }
}
