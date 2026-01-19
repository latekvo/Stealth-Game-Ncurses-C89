#include "canvas.h"

#include <ncurses.h>
#include <stdlib.h>

#include "colors.h"

void init_canvas(Canvas* canvas, Arena* arena) {
  canvas->size_x = arena->size_x * 2;
  canvas->size_y = arena->size_y;
  canvas->scale_x = 2.0;
  canvas->scale_y = 1.0;

  /* TODO: Implement both, they should both be v. simple to do */
  canvas->enable_coloring = 1;
  canvas->enable_fog_of_war = 0;

  uint max_round_err = 2;
  uint tilecount = (uint)(canvas->size_x * canvas->scale_x * canvas->size_y *
                          canvas->scale_y);

  canvas->data = malloc((tilecount + max_round_err) * sizeof(CanvasTile));

  /* FIXME: This is redundant, only useful for debugging */
  uint x, y, pos;
  for (y = 0; y < canvas->size_x; y++) {
    for (x = 0; x < canvas->size_x; x++) {
      pos = x + y * canvas->size_x;
      canvas->data[pos].can_light_pass = 0;
      canvas->data[pos].color_code = ERROR_COLOR_CODE;
      canvas->data[pos].display_char = '?';
    }
  }
}

void print_canvas(Canvas* canvas) {
  int x, y;
  for (y = 0; y < canvas->size_y; y++) {
    move(y, 0);
    for (x = 0; x < canvas->size_x; x++) {
      CanvasTile* tile = &canvas->data[x + y * canvas->size_x];
      attron(COLOR_PAIR(tile->color_code));
      addch(tile->display_char);
    }
  }
};

