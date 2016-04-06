#ifndef DETECTION_H
#define DETECTION_H

// Qt includes
#include <QRect>

class Detection
{
public:
  Detection();
  Detection(QRect box, int confidence);

  bool operator ==(const Detection &b) const;
  bool operator <(const Detection &b) const;

public:
  QRect box_;
  int confidence_;
};

#endif // DETECTION_H
