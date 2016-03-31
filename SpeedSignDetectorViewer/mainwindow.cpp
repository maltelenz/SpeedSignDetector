#include "mainwindow.h"
#include "ui_mainwindow.h"

// Qt Includes
#include <QFileDialog>
#include <QLabel>
#include <QDebug>
#include <QGraphicsTextItem>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  scene_.addText("Open an image...");

  ui->graphicsView->setScene(&scene_);

  statusLabel_ = new QLabel("");
  ui->statusBar->addPermanentWidget(statusLabel_);

  connect(&detector_, SIGNAL(issueMessage(QString)), this, SLOT(on_issueMessage(QString)));
  connect(&detector_, SIGNAL(issueTiming(QString)), this, SLOT(on_issueTiming(QString)));
  connect(&detector_, SIGNAL(itemFound(QRect, int)), this, SLOT(on_itemFound(QRect, int)));

  on_issueMessage(QString("Startup successful."));
  on_issueMessage(QString("Open an image to start."));
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

  ui->actionReset->setEnabled(true);
  ui->actionBlur->setEnabled(true);
  ui->actionEdges->setEnabled(true);
  ui->actionFind_Object->setChecked(false);

  connect(&scene_, SIGNAL(mouseReleased(QRectF)), this, SLOT(on_selectionReleased(QRectF)));
  connect(&scene_, SIGNAL(mouseMoved(QPointF)), this, SLOT(on_mouseMoved(QPointF)));
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
  QString txt = QString("(%1, %2)").arg(QString::number(point.x()), QString::number(point.y()));
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

void MainWindow::on_itemFound(QRect position, int confidence)
{
  QColor c(0, 150, 0);
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
  detector_.sobelEdges(ui->lowerThreshold->value());
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

void MainWindow::on_actionFind_Object_triggered()
{
  // Do edge detection
  on_actionEdges_triggered();
  // Find the object
  detector_.findObject();
}

void MainWindow::on_actionFind_Scaled_Objects_triggered()
{
  // Do edge detection
  on_actionEdges_triggered();
  // Find the object
  detector_.findScaledObject();
}
