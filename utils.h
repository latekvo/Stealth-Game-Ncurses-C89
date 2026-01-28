#ifndef UTILS_H
#define UTILS_H

typedef unsigned int uint;
typedef unsigned char byte;

extern const uint ARENA_SIZE;
extern const float PI;
extern const uint AVG_ROOM_SIDE;

float rand_f(float min, float max);
float rand_ui(uint min, uint max);

float clampf(float value, float min, float max);

#endif
