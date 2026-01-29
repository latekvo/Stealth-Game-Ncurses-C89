#include "player_drawing.h"

#include "colors.h"
#include "utils.h"

void draw_player(Canvas* canvas, Arena* arena) {
  uint c_pos = arena->player->position_x * canvas->scale_x +
               arena->player->position_y * canvas->scale_y * canvas->size_x;

  /* TODO: Same for y */
  uint x_off;
  for (x_off = 0; x_off < canvas->scale_x; x_off++) {
    canvas->data[c_pos + x_off].display_char = '%';
    canvas->data[c_pos + x_off].can_light_pass = 0;
    canvas->data[c_pos + x_off].color_code = EXIT_COLOR_CODE;
  }
}
