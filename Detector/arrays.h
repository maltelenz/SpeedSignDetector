#ifndef ARRAYS_H
#define ARRAYS_H


class Array2D
{
public:
  Array2D();
  Array2D(int xSize, int ySize);

  ~Array2D();

  bool init();
  bool init(int xSize, int ySize);

  void clear();

  int get(int x, int y);

  void set(int x, int y, int val);
  void increment(int x, int y);

  int xSize();
  int ySize();

private:
  int offset(int x, int y);

private:
  int xSize_;
  int ySize_;
  int* data_;
};


class Array3D
{
public:
  Array3D();
  Array3D(int xSize, int ySize, int zSize);

  ~Array3D();

  bool init();
  bool init(int xSize, int ySize, int zSize);

  void clear();

  int get(int x, int y, int z);

  void set(int x, int y, int z, int val);
  void increment(int x, int y, int z);

  int xSize();
  int ySize();
  int zSize();

private:
  int offset(int x, int y, int z);

private:
  int xSize_;
  int ySize_;
  int zSize_;
  int* data_;
};

#endif // ARRAYS_H
