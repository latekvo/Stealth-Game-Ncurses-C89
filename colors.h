#ifndef COLORS_H
#define COLORS_H

typedef enum {
  RAY_COLOR_CODE = 1,
  FLOOR_COLOR_CODE,
  WALL_COLOR_CODE,
  EXIT_COLOR_CODE,
  SIDE_GOAL_COLOR_CODE,
  ERROR_COLOR_CODE,
  TEXT_COLOR_CODE,
} ColorCode;

void init_colors();

#endif
