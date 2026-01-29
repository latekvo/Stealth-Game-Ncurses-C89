#include "lurker_logic.h"

#include <malloc.h>
#include <math.h>
#include <ncurses.h>

#include "arena.h"
#include "lurker.h"
#include "utils.h"

void init_lurkers(Arena* arena) {
  int i;
  for (i = 0; i < arena->size_x * arena->size_y; i++) {
    if (arena->data[i] != LURKER_SPAWN) {
      continue;
    }

    /* TODO: Move entire lurker init logic to map generation
    // - Removes need for LURKER_SPAWN, PLAYER_SPAWN, etc
    // - Just as simple in handling if we isolate to add_lurker
    */

    arena->data[i] = FLOOR;

    if (arena->lurker_count == arena->lurker_capacity) {
      arena->lurker_capacity *= 2;
      arena->lurkers = realloc(arena->lurkers, arena->lurker_capacity);
    }

    uint pos_x = i % arena->size_x;
    uint pos_y = (i - pos_x) / arena->size_y;

    Lurker new_lurker;

    new_lurker.detection_cone_halfangle_rad = PI / 4;
    new_lurker.min_velocity = 1.5;
    new_lurker.max_velocity = 3.0;
    new_lurker.position_x = pos_x;
    new_lurker.position_y = pos_y;
    new_lurker.status = WALKING_OFFICE;
    new_lurker.azimuth_current_rad = 0;
    new_lurker.azimuth_target_rad = PI;
    new_lurker.patrol_direction_timer = 0;

    arena->lurkers[arena->lurker_count] = new_lurker;
    arena->lurker_count += 1;
  }
}

void update_lurkers(Arena* arena, float time_delta) {
  const float EPSILON_FOR_JITTER = PI / 18;
  const float JITTER_RADIUS = PI / 12;
  const float MAX_CHANGE_PER_S = PI / 2;

  /*
  // Lurker has fixed energy (total of axial + positional velocity)
  // 1. Axial velocity is t_delta * diff_to_target_azimuth
  // 1.1. RNG for turn left, RNG for turn right, add them together
  //      (%% This sounds the simplest)
  // 2. Remaining energy is spent on positional velocity
  // 2.1. No RNG just fill total velocity to 100%
  */

  int i;
  for (i = 0; i < arena->lurker_count; i++) {
    Lurker* lurker = &arena->lurkers[i];
    /* TODO: Add velocity system, it's not trivial due to overshooting etc,
    //       but it would look much better than the current linear approach.
    //       ^^^ But it is neccessary for the energy thing to even make sense.
    */

    float az_curr = lurker->azimuth_current_rad;
    float az_diff = lurker->azimuth_target_rad - az_curr;
    float delta = fmodf(az_diff + PI, PI * 2) - PI;

    float change_per_s = clampf(delta, -MAX_CHANGE_PER_S, MAX_CHANGE_PER_S);

    if (fabsf(delta) < EPSILON_FOR_JITTER) {
      change_per_s = rand_f(-JITTER_RADIUS, JITTER_RADIUS);
    }

    float energy_ratio_remaining = 1 - fabsf(change_per_s) / MAX_CHANGE_PER_S;
    float move_speed_per_s =
        energy_ratio_remaining * (lurker->max_velocity - lurker->min_velocity) +
        lurker->min_velocity;
    float velocity = move_speed_per_s * time_delta;

    /* TODO: Getting weird random behaviour with OOB,
     * check if velocity maybe is getting to some extremes */
    float vel_x = cos(az_curr) * velocity;
    float vel_y = sin(az_curr) * velocity;

    uint pos_x = lurker->position_x + vel_x;
    uint pos_y = lurker->position_y + vel_y;
    uint pos_i = pos_x + pos_y * arena->size_x;

    if (pos_i >= 0 && pos_i < arena->size_x * arena->size_y &&
        arena->data[pos_i] == FLOOR) {
      lurker->position_x = pos_x;
      lurker->position_y = pos_y;
    }

    float change_per_frame = change_per_s * time_delta;
    lurker->azimuth_current_rad = fmodf(az_curr + change_per_frame, PI * 2);
    lurker->patrol_direction_timer += time_delta;

    /* DEBUG: TODO remove this */
    lurker->azimuth_target_rad += time_delta;
  }
}

