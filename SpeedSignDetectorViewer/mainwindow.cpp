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
}
