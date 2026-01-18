#include "colors.h"

#include <ncurses.h>
#include <stdlib.h>

void init_colors() {
  start_color();

  /* TODO: Isolate this guard and init_pair(s) */
  if (has_colors() == 0) {
    endwin();
    printf("Color not supported. Color support is required\n");
    exit(1);
  }

  init_pair(RAY_COLOR_CODE, COLOR_RED, COLOR_RED);
  init_pair(WALL_COLOR_CODE, COLOR_BLACK, COLOR_WHITE);
  init_pair(FLOOR_COLOR_CODE, COLOR_WHITE, COLOR_BLACK);
  init_pair(EXIT_COLOR_CODE, COLOR_CYAN, COLOR_YELLOW);
  init_pair(SIDE_GOAL_COLOR_CODE, COLOR_YELLOW, COLOR_CYAN);
  init_pair(ERROR_COLOR_CODE, COLOR_YELLOW, COLOR_MAGENTA);
}
