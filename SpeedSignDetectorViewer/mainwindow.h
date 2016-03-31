#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Qt includes
#include <QMainWindow>
#include <QGraphicsScene>

// SpeedSignDetectorViewer includes
#include "imagescene.h"

// Detector includes
#include "detector.h"

// Forward declarations
class QLabel;

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
  void on_actionReset_triggered();
  void on_actionBlur_triggered();
  void on_actionEdges_triggered();
  void on_actionShowAngles_triggered(bool on);
  void on_actionEdge_Thinning_triggered();
  void on_actionQuit_triggered();
  void on_actionR_Table_triggered();
  void on_actionFind_Object_triggered(bool on);

  void on_selectionReleased(QRectF rectf);
  void on_mouseMoved(QPointF point);

  void on_actionFind_Scaled_Objects_triggered(bool on);

public slots:
  void on_issueMessage(QString message);
  void on_issueTiming(QString message);
  void on_itemFound(QRect position, int confidence);

private:
  Ui::MainWindow *ui;

  ImageScene scene_;
  QLabel* statusLabel_;

  Detector detector_;
  void refetchImage();
};

#endif // MAINWINDOW_H
