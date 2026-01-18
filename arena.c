#include "arena.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

byte are_rooms_overlapping(RoomSeed* a, RoomSeed* b, float pad_t, float pad_r,
                           float pad_b, float pad_l) {
  /* TODO: These could (should?) be cached */
  float a_r = a->center_x + a->radius_r + pad_r;
  float a_l = a->center_x - a->radius_l - pad_l;
  float a_t = a->center_y + a->radius_t + pad_t;
  float a_b = a->center_y - a->radius_b - pad_b;
  float b_r = b->center_x + b->radius_r;
  float b_l = b->center_x - b->radius_l;
  float b_t = b->center_y + b->radius_t;
  float b_b = b->center_y - b->radius_b;

  /* FIXME: Bounds checks shouldn't be here, but it's the simplest for now. */
  if (a_r >= ARENA_SIZE || a_l <= 0 || a_t >= ARENA_SIZE || a_b < 0) {
    return 1;
  }

  /* Each of these cases guarantees separation */
  if (a_r < b_l || a_l > b_r || a_t < b_b || a_b > b_t) {
    return 0;
  }

  return 1;
}

void init_arena(Arena* arena, Player* player) {
  arena->player = player;

  arena->lurker_count = 0;
  arena->lurker_capacity = 16;
  arena->lurkers = malloc(arena->lurker_capacity * sizeof(Lurker));
  assert(arena->lurkers);

  arena->size_x = ARENA_SIZE;
  arena->size_y = ARENA_SIZE;
  arena->data = malloc(ARENA_SIZE * ARENA_SIZE * sizeof(uint));
  assert(arena->data);

  uint total_v = arena->size_x * arena->size_y;
  uint avg_room_v = AVG_ROOM_SIDE * AVG_ROOM_SIDE;
  float assumed_wall_ratio = 0.35;
  float seed_count_f = (float)total_v / avg_room_v * assumed_wall_ratio;
  /* This avoids roundf(), it is broken on my setup */
  uint seed_count = (uint)(seed_count_f + 0.5f);

  arena->room_seed_count = seed_count;
  arena->room_seeds_finished = 0;
  arena->room_seeds = malloc(arena->room_seed_count * sizeof(RoomSeed));
  assert(arena->room_seeds);
}

void generate_arena(Arena* arena) {
  uint total_v = arena->size_x * arena->size_y;
  uint edge_padding = 10;
  uint max_usable_rng_x = arena->size_x - edge_padding;
  uint max_usable_rng_y = arena->size_y - edge_padding;

  memset(arena->data, WALL, total_v * sizeof(uint));

  uint i, j, pos_x, pos_y;
  for (i = 0; i < arena->room_seed_count; i++) {
  retry_point_gen:
    pos_x = rand_ui(edge_padding, max_usable_rng_x);
    pos_y = rand_ui(edge_padding, max_usable_rng_y);

    for (j = 0; j < i; j++) {
      RoomSeed checked_seed = arena->room_seeds[j];
      uint dist_x = abs((int)checked_seed.center_x - (int)pos_x);
      uint dist_y = abs((int)checked_seed.center_y - (int)pos_y);
      if (dist_x < 3 || dist_y < 3) {
        goto retry_point_gen;
      }
    }

    RoomSeed new_seed;

    new_seed.block_b = 0;
    new_seed.block_t = 0;
    new_seed.block_l = 0;
    new_seed.block_r = 0;

    new_seed.center_x = pos_x;
    new_seed.center_y = pos_y;

    new_seed.growth_vel_x = rand_f(.2, .5);
    new_seed.growth_vel_y = rand_f(.2, .5);

    /* TODO: Go through all and set the ones on opposite edges */
    new_seed.is_player_spawn = 0;
    new_seed.is_end_objective_room = 0;
    new_seed.is_room_finished = 0;

    new_seed.radius_l = 1;
    new_seed.radius_r = 1;
    new_seed.radius_t = 1;
    new_seed.radius_b = 1;

    new_seed.total_door_count = 0;

    arena->room_seeds[i] = new_seed;
  }

  while (arena->room_seeds_finished < arena->room_seed_count) {
    for (i = 0; i < arena->room_seed_count; i++) {
      RoomSeed* seed = &arena->room_seeds[i];

      if (seed->is_room_finished) {
        continue;
      }

      float dir_pad = 1.5;
      float pad = 1.1;

      uint j;
      for (j = 0; j < arena->room_seed_count; j++) {
        if (i == j) {
          continue;
        }

        RoomSeed* ck_seed = &arena->room_seeds[j];

        /* Note: Could set block_* status for the ck_seed too, but currently
        //       doing that doesn't skip any logic, so there is no point.
        */

        if (are_rooms_overlapping(seed, ck_seed, dir_pad, pad, pad, pad)) {
          seed->block_t = 1;
        }

        if (are_rooms_overlapping(seed, ck_seed, pad, dir_pad, pad, pad)) {
          seed->block_r = 1;
        }

        if (are_rooms_overlapping(seed, ck_seed, pad, pad, dir_pad, pad)) {
          seed->block_b = 1;
        }

        if (are_rooms_overlapping(seed, ck_seed, pad, pad, pad, dir_pad)) {
          seed->block_l = 1;
        }
      }

      if (!seed->block_t) {
        seed->radius_t += seed->growth_vel_y;
      }

      if (!seed->block_b) {
        seed->radius_b += seed->growth_vel_y;
      }

      if (!seed->block_l) {
        seed->radius_l += seed->growth_vel_x;
      }

      if (!seed->block_r) {
        seed->radius_r += seed->growth_vel_x;
      }

      if (seed->block_b && seed->block_t && seed->block_l && seed->block_r) {
        arena->room_seeds_finished += 1;
        seed->is_room_finished = 1;
      }
    }
  }

  /* Keep iterating over all until all have req doorways.
  // TODO: This algo.
  */

  for (i = 0; i < arena->room_seed_count; i++) {
    RoomSeed seed = arena->room_seeds[i];
  }

  for (i = 0; i < arena->room_seed_count; i++) {
    RoomSeed seed = arena->room_seeds[i];

    float x_start_f = seed.center_x - seed.radius_l;
    float x_end_f = seed.center_x + seed.radius_r;
    float y_start_f = seed.center_y - seed.radius_b;
    float y_end_f = seed.center_y + seed.radius_t;

    uint x_start = (uint)(x_start_f + 1.f);
    uint x_end = (uint)x_end_f;
    uint y_start = (uint)(y_start_f + 1.f);
    uint y_end = (uint)y_end_f;

    uint x_span = x_end - x_start;

    uint y;
    for (y = y_start; y <= y_end; y++) {
      uint offset = x_start + y * arena->size_x;
      memset(arena->data + offset, FLOOR, x_span);
    }

    uint lurker_spawn_idx = seed.center_x + seed.center_y * arena->size_x;
    arena->data[lurker_spawn_idx] = LURKER_SPAWN;
  }
}
