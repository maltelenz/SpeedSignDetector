#ifndef IMAGESCENE_H
#define IMAGESCENE_H

#include <QGraphicsView>

class ImageScene : public QGraphicsScene
{
  Q_OBJECT

public:
  ImageScene(QWidget * parent = 0);

protected:
  void mousePressEvent(QGraphicsSceneMouseEvent *event);
  void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

signals:
  void mouseReleased(QRectF selection);
  void mouseMoved(QPointF pos);

private:
  bool dragging_;
  QRectF selection_;

  QGraphicsRectItem* selectionItem_;
};

#endif // IMAGESCENE_H
