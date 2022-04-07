#ifndef BUFFER_H
#define BUFFER_H

union UInt16
{
  uint16_t val;
  uint8_t bytes[2];
};

union UInt32
{
  int32_t val;
  uint8_t bytes[4];
};

//12byte
struct Angles
{
  uint16_t a1;
  uint16_t a2;
  uint16_t a3;
  uint16_t a4;
  uint16_t a5;
  uint16_t a6;
};

template <typename T>
class Buffers
{
    const static int maxLen = 200;
    const static float fullRatio = 0.9f;
  public:
    Buffers() = default;
    bool isFull() const {
      return _currentPrt > (int)(maxLen * fullRatio);
    }

    bool isEmpty() const {
      return _currentPrt == -1;
    }

    bool addOne(const T& angles)
    {
      if (isFull())
        return false;
      _buffer[++_currentPrt] = angles;
      return true;
    }

    bool getFront(T& angles)
    {
      if (isEmpty()) {
        return false;
      }
      angles = _buffer[0];
      shuffle();
      _currentPrt--;
      return true;
    }
  private:
    void shuffle()
    {
      for (int i = 0; i < maxLen - 1; i++) {
        _buffer[i] = _buffer[i + 1];
      }
      _buffer[maxLen - 1] = {};
    }

    T _buffer[maxLen] {};
    int _currentPrt = -1;
};

#endif
