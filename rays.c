#include "rays.h"

#include <math.h>

#include "arena.h"
#include "canvas.h"
#include "colors.h"

const float DETECTION_RAYS = 50;

void draw_lurker_rays(Canvas* canvas, Arena* arena) {
  Lurker* lurkers = arena->lurkers;
  uint lurker_count = arena->lurker_count;

  /* Lurker rays have special drawing requirements:
  // - They have to operate on the final canvas for higher resolution
  // - They have to collide with visual walls, not their abstract notations.
  //   - Otherwise we'll have the rays clip fine edges (e.g. 0.1 pos diff).
  */

  uint i;
  for (i = 0; i < lurker_count; i++) {
    Lurker* lurker = &lurkers[i];
    float pos_x = lurker->position_x;
    float pos_y = lurker->position_y;
    float heading = lurker->azimuth_current_rad;

    /* TODO: sin and cos of delta between rays can be precomputed,
    //       then applied to a unit vec of the heading
    */

    float min = heading - lurker->detection_cone_halfangle_rad;
    float max = heading + lurker->detection_cone_halfangle_rad;
    float span = max - min;
    float delta = span / DETECTION_RAYS;
    float ray_step = 0.4;

    float angle;
    for (angle = min; angle < max; angle += delta) {
      float ray_x = pos_x * canvas->scale_x;
      float ray_y = pos_y * canvas->scale_y;
      float vel_x = cos(angle) * ray_step;
      float vel_y = sin(angle) * ray_step;

      CanvasTile* tile =
          &canvas->data[(uint)ray_x + (uint)ray_y * canvas->size_x];

      /* TODO: Maybe add max beam range? Don't do iter (!), just compute diag
      //
      */

      while (tile->can_light_pass) {
        tile->display_char = '+';
        tile->color_code = RAY_COLOR_CODE;

        tile = &canvas->data[(uint)ray_x + (uint)ray_y * canvas->size_x];
        ray_x += vel_x * canvas->scale_x;
        ray_y += vel_y * canvas->scale_y;

        if (ray_x <= 0 || ray_x >= arena->size_x - 1 || ray_y <= 0 ||
            ray_y >= arena->size_y - 1) {
        }
      }
    }
  }
}
