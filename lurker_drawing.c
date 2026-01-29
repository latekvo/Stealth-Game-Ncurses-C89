#include "lurker_drawing.h"

#include "colors.h"

void draw_lurkers(Canvas* canvas, Lurker* lurkers, uint lurker_count,
                  float time_delta) {
  uint i, pos, pos_x, pos_y;
  Lurker* lurker;
  CanvasTile* tile;
  for (i = 0; i < lurker_count; i++) {
    lurker = &lurkers[i];

    /* TODO: Draw over 4 tiles, not just 1 */

    pos_x = lurker->position_x * canvas->scale_x;
    pos_y = lurker->position_y * canvas->scale_y;

    pos = pos_x + pos_y * canvas->size_x;
    tile = &canvas->data[pos];
    tile->can_light_pass = 0;
    tile->color_code = EXIT_COLOR_CODE;
    tile->display_char = '@';
  }
}
