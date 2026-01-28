#include "utils.h"

#include <stdlib.h>

const uint ARENA_SIZE = 60;
const float PI = 3.14159;
const uint AVG_ROOM_SIDE = 12;

float rand_f(float min, float max) {
  return ((float)rand() / RAND_MAX) * (max - min) + min;
}

float rand_ui(uint min, uint max) { return rand() % (max - min) + min; }

float clampf(float value, float min, float max) {
  const float t = value < min ? min : value;
  return t > max ? max : t;
}

