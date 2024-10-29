#include "utils.h"

static const int percMin = 1;
static const int percMax = 100;
static const int byteMin = 1;
static const int byteMax = 254;

// These functions are written such that
// brightnessPercentageToByte(brightnessByteToPercentage(n)) == n and vice versa

int brightnessPercentageToByte(int brightnessPercentage) {
  return (int)(((brightnessPercentage - percMin) * (byteMax - byteMin) +
                (percMax - percMin) / 2) /
                   (percMax - percMin) +
               byteMin);
}

int brightnessByteToPercentage(int brightnessByte) {
  return (int)(((brightnessByte - byteMin) * (percMax - percMin) +
                (byteMax - byteMin) / 2) /
                   (byteMax - byteMin) +
               percMin);
}

int clamp(int value, int min, int max) {
  if (value < min) {
    return min;
  } else if (value > max) {
    return max;
  } else {
    return value;
  }
}