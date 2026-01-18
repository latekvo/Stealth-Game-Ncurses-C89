#ifndef CANVAS_H
#define CANVAS_H

#include "arena.h"
#include "utils.h"

typedef struct {
  byte can_light_pass;
  char color_code;
  char display_char;
} CanvasTile;

typedef struct {
  uint size_x, size_y;
  float scale_x, scale_y;
  byte enable_fog_of_war;
  byte enable_coloring;
  CanvasTile* data;
} Canvas;

void init_canvas(Canvas* canvas, Arena* arena);
void print_canvas(Canvas* canvas);

#endif
