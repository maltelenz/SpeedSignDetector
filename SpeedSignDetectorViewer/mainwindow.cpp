#include "mainwindow.h"
#include "ui_mainwindow.h"

// Qt Includes
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  scene_.addText("Open an image...");

  ui->graphicsView->setScene(&scene_);

}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::on_actionLoad_Image_triggered()
{
  QString fileName(
        QFileDialog::getOpenFileName(this,
           tr("Open Image"), QString(), tr("Images (*.png *.jpg)")));
  detector_.loadImage(fileName);
  scene_.clear();
  scene_.addPixmap(detector_.getPixmap());
  scene_.setSceneRect(detector_.getImageSize());

  ui->actionMean_Lines->setEnabled(true);
  ui->actionPainter->setEnabled(true);
  ui->actionReset->setEnabled(true);
  ui->actionBlur->setEnabled(true);

  connect(&scene_, SIGNAL(mouseReleased(QRectF)), this, SLOT(on_selectionReleased(QRectF)));
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

void MainWindow::on_actionReset_triggered()
{
  scene_.clear();
  scene_.addPixmap(detector_.getPixmap());
}

void MainWindow::on_actionBlur_triggered()
{
  scene_.clear();
  scene_.addPixmap(QPixmap::fromImage(detector_.blurred()));
}
