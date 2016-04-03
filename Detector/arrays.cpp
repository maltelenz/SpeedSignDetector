#include "arrays.h"

// std includes
#include <stdlib.h>

Array2D::Array2D() :
  data_(NULL)
{

}

Array2D::Array2D(int xSize, int ySize) :
  xSize_(xSize),
  ySize_(ySize),
  data_(NULL)
{

}

Array2D::~Array2D()
{
  if (data_ != NULL) {
    free(data_);
    data_ = NULL;
  }
}

bool Array2D::init()
{
  clear();
  data_ = (int*) calloc(xSize_ * ySize_, sizeof(int));
  if (data_ == NULL) {
    // Failed to allocate memory, abort nicely
    return false;
  }
  return true;
}

bool Array2D::init(int xSize, int ySize)
{
  xSize_ = xSize;
  ySize_ = ySize;
  return init();
}

void Array2D::clear()
{
  if (data_ != NULL) {
    free(data_);
    data_ = NULL;
  }
}

int Array2D::get(int x, int y)
{
  return data_[offset(x, y)];
}

void Array2D::set(int x, int y, int val)
{
  data_[offset(x, y)] = val;
}

void Array2D::increment(int x, int y)
{
  data_[offset(x, y)]++;
}

int Array2D::xSize()
{
  return xSize_;
}

int Array2D::ySize()
{
  return ySize_;
}

int Array2D::offset(int x, int y)
{
  return (y * xSize_) + x;
}


Array3D::Array3D() :
  data_(NULL)
{

}

Array3D::Array3D(int xSize, int ySize, int zSize) :
  xSize_(xSize),
  ySize_(ySize),
  zSize_(zSize),
  data_(NULL)
{

}

Array3D::~Array3D()
{
  if (data_ != NULL) {
    free(data_);
    data_ = NULL;
  }
}

bool Array3D::init()
{
  clear();
  data_ = (int*) calloc(xSize_ * ySize_ * zSize_, sizeof(int));
  if (data_ == NULL) {
    // Failed to allocate memory, abort nicely
    return false;
  }
  return true;
}

bool Array3D::init(int xSize, int ySize, int zSize)
{
  xSize_ = xSize;
  ySize_ = ySize;
  zSize_ = zSize;
  return init();
}

void Array3D::clear()
{
  if (data_ != NULL) {
    free(data_);
    data_ = NULL;
  }
}

int Array3D::get(int x, int y, int z)
{
  return data_[offset(x, y, z)];
}

void Array3D::set(int x, int y, int z, int val)
{
  data_[offset(x, y, z)] = val;
}

void Array3D::increment(int x, int y, int z)
{
  data_[offset(x, y, z)]++;
}

int Array3D::xSize()
{
  return xSize_;
}

int Array3D::ySize()
{
  return ySize_;
}

int Array3D::zSize()
{
  return zSize_;
}

int Array3D::offset(int x, int y, int z)
{
  return (z * xSize_ * ySize_) + (y * xSize_) + x;
}
