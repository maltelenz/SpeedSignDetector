#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Qt includes
#include <QMainWindow>
#include <QGraphicsScene>

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

private:
  Ui::MainWindow *ui;

  QGraphicsScene scene_;

  Detector detector_;
};

#endif // MAINWINDOW_H
