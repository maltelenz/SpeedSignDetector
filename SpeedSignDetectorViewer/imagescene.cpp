#include "imagescene.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsRectItem>
#include <QDebug>

ImageScene::ImageScene(QWidget *parent) :
  selectionItem_(0)
{

}

void ImageScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  if (!selectionItem_) {
    selection_.setLeft(event->scenePos().x());
    selection_.setTop(event->scenePos().y());
    selection_.setRight(event->scenePos().x());
    selection_.setBottom(event->scenePos().y());

    selectionItem_ = new QGraphicsRectItem(selection_);
    selectionItem_->setPen(QPen(Qt::black, 3, Qt::SolidLine));
    addItem(selectionItem_);
    // Could probably do something more intelligent here instead of redrawing the complete thing...
    update();
  } else {
    QGraphicsScene::mousePressEvent(event);
  }
}

void ImageScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  emit mouseMoved(event->scenePos());
  if (selectionItem_) {
    selection_.setRight(event->scenePos().x());
    selection_.setBottom(event->scenePos().y());
    selectionItem_->setRect(selection_);
    // Could probably do something more intelligent here instead of redrawing the complete thing...
    update();
  } else {
    QGraphicsScene::mouseMoveEvent(event);
  }
}

void ImageScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  if (selectionItem_) {
    removeItem(selectionItem_);
    // Could probably do something more intelligent here instead of redrawing the complete thing...
    update();
    delete selectionItem_;
    selectionItem_ = 0;
    emit mouseReleased(selection_);
  } else {
    QGraphicsScene::mouseReleaseEvent(event);
  }
}
