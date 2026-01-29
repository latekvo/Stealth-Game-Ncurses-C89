#include "arena_drawing.h"

#include "colors.h"

void draw_arena(Canvas* canvas, Arena* arena) {
  int x, y;
  for (y = 0; y < arena->size_x; y++) {
    for (x = 0; x < arena->size_x; x++) {
      uint element = arena->data[x + y * arena->size_x];
      char repr = '?';
      byte can_light_pass = 0;
      byte color_code = FLOOR_COLOR_CODE;

      switch (element) {
        case FLOOR:
          repr = ' ';
          can_light_pass = 1;
          break;
        case WALL:
          repr = '#';
          color_code = WALL_COLOR_CODE;
          break;
        case PLAYER_SPAWN:
          repr = 'P';
          can_light_pass = 1;
          break;
        case LURKER_SPAWN:
          repr = 'E';
          can_light_pass = 1;
          break;
        case SIDE_OBJECTIVE:
          repr = '$';
          color_code = SIDE_GOAL_COLOR_CODE;
          break;
        case END_OBJECTIVE:
          repr = '%';
          color_code = EXIT_COLOR_CODE;
          break;
      }

      uint c_pos = x * canvas->scale_x + y * canvas->scale_y * canvas->size_x;

      /* TODO: Same for y */
      uint x_off;
      for (x_off = 0; x_off < canvas->scale_x; x_off++) {
        canvas->data[c_pos + x_off].display_char = repr;
        canvas->data[c_pos + x_off].can_light_pass = can_light_pass;
        canvas->data[c_pos + x_off].color_code = color_code;
      }
    }
  }
}

