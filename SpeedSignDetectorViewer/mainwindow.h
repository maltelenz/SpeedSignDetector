#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Qt includes
#include <QMainWindow>
#include <QGraphicsScene>

// SpeedSignDetectorViewer includes
#include "imagescene.h"

// Detector includes
#include "detector.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

private slots:
  void on_actionLoad_Image_triggered();

  void on_actionMean_Lines_toggled(bool on);

  void on_actionPainter_triggered(bool on);

  void on_selectionReleased(QRectF rectf);

private:
  Ui::MainWindow *ui;

  ImageScene scene_;

  Detector detector_;
};

#endif // MAINWINDOW_H
