#include "detection.h"

Detection::Detection() :
  confidence_(0)
{

}

Detection::Detection(QRect box, int confidence) :
  box_(box),
  confidence_(confidence)
{

}

bool Detection::operator ==(const Detection &b) const
{
  return b.box_ == box_ && b.confidence_ == confidence_;
}

bool Detection::operator <(const Detection &b) const
{
  return confidence_ < b.confidence_;
}
