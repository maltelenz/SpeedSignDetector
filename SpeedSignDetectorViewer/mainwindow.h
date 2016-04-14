#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Qt includes
#include <QMainWindow>
#include <QGraphicsScene>
#include <QColor>

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

  void on_mouseMoved(QPointF point);

  void on_actionTrain_triggered();

  void on_actionEliminate_Colors_triggered();

  void on_actionDetect_triggered();

  void on_actionHarris_Corners_triggered();

  void on_edgeThreshold_textChanged(const QString &value);
  void on_harrisThreshold_textChanged(const QString &value);

  void on_actionTrain_Harris_triggered();

  void on_actionDetect_Harris_triggered();

public slots:
  void on_issueMessage(QString message);
  void on_issueTiming(QString message);
  void on_itemFound(QRect position, int confidence, int order);

private:
  Ui::MainWindow *ui;

  ImageScene scene_;
  QLabel* statusLabel_;

  QColor detectionColor1_;
  QColor detectionColor2_;
  QColor detectionColor3_;
  QColor detectionColor4_;
  QColor detectionColor5_;


  Detector detector_;
  void refetchImage();
};

#endif // MAINWINDOW_H
