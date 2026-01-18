#include <assert.h>
#include <math.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned int uint;
typedef unsigned char byte;

typedef struct {
  uint position_x, position_y;
} Player;

typedef struct {
  float position_x, position_y;
  float velocity, min_velocity, max_velocity;
  float detection_cone_halfangle_rad;
  float azimuth_target_rad, azimuth_current_rad;
  uint status;
} Lurker;

enum LurkerStatus {
  /* Placeholder */
  STANDING = 0,
  /* Azimuth unaligned, changing frequently (~1s?) */
  WALKING_CAVE,
  /* Azimuth aligned to N*45d, changing rarely (~3s?) */
  WALKING_OFFICE,
};

typedef struct {
  uint center_x, center_y;
  float growth_vel_x, growth_vel_y;
  uint total_door_count;
  float radius_l, radius_r, radius_t, radius_b;
  byte block_l, block_r, block_t, block_b;
  byte is_player_spawn;
  byte is_end_objective_room;
  byte is_room_finished;
  /* For connectivity (doors) verification logic
  // TODO: TBD
  */
  byte is_reachable;
  byte needs_more_doors;
} RoomSeed;

typedef struct {
  /* Not implementing >1 doors per room for now;
  // uint room_a_id, room_b_id;
  */
  float a_x, a_y, b_x, b_y;
} DoorwaySeed;

typedef struct {
  uint size_x, size_y;
  byte* data;
  Player* player;
  Lurker* lurkers;
  uint lurker_count;
  uint lurker_capacity;
  RoomSeed* room_seeds;
  uint room_seed_count;
  uint room_seeds_finished;
} Arena;

typedef enum {
  RAY_COLOR_CODE = 1,
  FLOOR_COLOR_CODE,
  WALL_COLOR_CODE,
  EXIT_COLOR_CODE,
  SIDE_GOAL_COLOR_CODE,
  ERROR_COLOR_CODE,
} ColorCode;

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

typedef struct {
  uint size_x, size_y;
  uint offset_x, offset_y;
} View;

/* TODO: Tile as a struct with properties, not enum */
enum ArenaTile {
  FLOOR = 0,
  FLOOR_MOSS,
  FLOOR_ROCKY,
  FLOOR_SMOOTH,
  FLOOR_WATER,
  NO_SPAWN_FLOOR,
  WALL,
  WALL_MOSS,
  WALL_ROCKY,
  WALL_SMOOTH,
  PLAYER_SPAWN,
  LURKER_SPAWN,
  SIDE_OBJECTIVE,
  END_OBJECTIVE,
};

const uint ARENA_SIZE = 60;
const float PI = 3.14159;
const uint AVG_ROOM_SIDE = 12;

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

float rand_f(float min, float max) {
  return ((float)rand() / RAND_MAX) * (max - min) + min;
}

float rand_ui(uint min, uint max) { return rand() % (max - min) + min; }

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

    new_lurker.detection_cone_halfangle_rad = PI / 3;
    new_lurker.min_velocity = 1.5;
    new_lurker.max_velocity = 3.0;
    new_lurker.position_x = pos_x;
    new_lurker.position_y = pos_y;
    new_lurker.velocity = 1.5;
    new_lurker.status = WALKING_OFFICE;

    arena->lurkers[arena->lurker_count] = new_lurker;
    arena->lurker_count += 1;
  }
}

void update_lurkers(Lurker* lurkers, uint lurker_count, float time_delta) {
  /* NOTE: Need to use raycasting to prevent wall penetration
  // Would be pretty if rays were higher-res than walls, so directly to canvas
  // In patrol mode, add preference for the lurker to snap to multiples of 45d
  // Add jitter to lurker's rotation, this way it's more realistic and prettier
  // Energy remains constant, when moving slower, it will turns faster.
  // 1. When azimuth target far off from actual, slow down
  // 2. As lurker is slow, it can rotate faster
  // 3. As alignment increases, so does speed
  // NOTE: Using accel would yield more realistic look, i can imagine stiff rot
  // looking weird, unnatural, will fix this with azimuth target jitter
  // Is p-derived rebound needed? No, direction changes quickly either way
  */

  int i;
  for (i = 0; i < lurker_count; i++) {
    Lurker* lurker = &lurkers[i];
    /* TBD */
  }
}

/* Hardcoding 50 rays is fine for now */
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
    float ray_step = 0.3;

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
      }
    }
  }
}

void draw_lurkers(Canvas* canvas, Arena* arena, float time_delta) {
  /* TBD
  //
  */
  Lurker* lurkers = arena->lurkers;
}

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

void print_canvas(Canvas* canvas) {
  int x, y;
  for (y = 0; y < canvas->size_y; y++) {
    move(y, 0);
    for (x = 0; x < canvas->size_x; x++) {
      CanvasTile* tile = &canvas->data[x + y * canvas->size_x];
      printf("%c\n", tile->display_char);
      attron(COLOR_PAIR(tile->color_code));
      addch(tile->display_char);
    }
  }
};

int main() {
  Arena arena;
  Player player;
  Canvas canvas;
  View view;

  /* TODO: Remove this, but for now this simplifies debug */
  setvbuf(stdout, NULL, _IONBF, 0);

  initscr();
  cbreak();
  noecho();
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

  init_arena(&arena, &player);
  init_canvas(&canvas, &arena);
  generate_arena(&arena);
  init_lurkers(&arena);

  float time_delta = 1.0;

  while (1) {
    /* TODO:
    // - Calc time_delta (but using it for sleep is probs good enough)
    // - User inputs (i'm reading you can add timeout to getch())
    // - Draw Canvas
    // - Draw View
    // - Add coloring according to ascii (probs simpler than metadata)
    // - Render (see if overwriting rather than redrawing is feasible)
    */

    update_lurkers(arena.lurkers, arena.lurker_count, time_delta);

    draw_arena(&canvas, &arena);
    draw_lurker_rays(&canvas, &arena);
    draw_lurkers(&canvas, &arena, time_delta);

    print_canvas(&canvas);

    /* FIXME: Temporary replacement for timer (╥﹏╥) */
    refresh();
    getch();
  }

  /* TODO: Check if we really have to free if we're exiting anyways */
  free(arena.data);
  free(arena.lurkers);
  free(canvas.data);

  getch();
  endwin();

  return 0;
}

/* -=-=-=- Development note archive -=-=-=-

// Map generation:
// - I was initially imagining going for caves
// - Rectangular office style could be more interesting
//   - Rectangle walls can be mangled to fall back to the cave idea
//   - This would actually make for an interesting "neglected underground lab"
//   - Office elements could be mixed with rocky elements, water and flora
// - How do we want to place the rooms?
//   - One at start, one at end
//   - Random all over the place
//   - Connect with corridors (3-5w for n+1-long, 1w for 1-long)
//   - Play with density to minimize corridors
//   - Place enemies and objectives randomly around
// - [Perlin is hard in C !!!] so don't do it, log rand is enough for rocks
//   - Poor man's perlin is 3 layers of variable smoothing 1d (!!!) rnd
// - Room spec
//   - Will start with 5x5 for both entry and exit
//   - Supposing total volume V, aiming for 12x12 (V=144) rooms on avg.
//   - Could do a "stack-on" algo? Where we have 1 room in center,
//     then assuming N padding, we start the next room N tiles from it.
//     We grow it out until we reach the map boundary.
//   - Alternatively, we lay out some kind of reference points,
//     connect them with variable width wall, and the rest is open office?
//   - EVEN BETTER, a seed growth algo
//     - We plant N seeds in the map
//     - Each seed growths with variable speed
//     - Once seed encounters a wall (another seed's growth), it stops
//       growing in that particular direction, but continues in others.
//     - At the end, we place 1 door for every seed, but min 2 per room
//     - While "meet halfway" would be optimal, I have no idea how
//       complex it is to implement, so i will just do a slow-stepping iter.
//     - Aiming for 144 V avg. Under ideal conditions, avail space
//       is 3600 (for 60x60), that's 25 room seeds.
//     - If we assume walls take 30% of the total V, seeds move down to 18.
//     - Perimeter can flood the walls (up to 1 thickness) with cave floor.
//       - We can force this behaviour by removing seeds outside an inner oval
//       - While also preventing growth outside of said oval
//       - Expanding on the story, collect keys/intel then escape?
*/

/* Arena:
// - walls
// - entity registrar
// Canvas:
// - """shaded""" Arena view (TBD, i've done this in rust before, need to port)
// - fog of war effect
// - all enemies and their effects
// View:
// - a cutout of the Canvas centered on Player
// Player:
// - metadata for player
// Lurker
// - metaata for enemy
*/

/* Lurker:
//  Office movement aligned to walls (0, 45deg, 90deg, ...)
//  Cave movement slow and curvy, slaloming constantly
//  Rotation is an alignment change in office space
*/

/* TBD
// Classic bounds check against all other seeds
// - First check lower's low vs higher's high, then lower's high etc
// - If that aligns (collides), check in the other direction
// Exampel
// - Checking horizontal axis collisions
// - Filter all others for [self.x;self.x+self.vel.x] bounds
// - Check filtered for: 1 of 4 points between 2 opposite points:
// - In total always <6 checks
//
//       ck1
//      |-v-|
//      | v |
//      | o---- -
// - ---o |
//      | |
// - ---o |
//        o---- -
//
//     --o
// - --  |
//       |
// - --o<|<< ck2
//     --o
//
//     --o
// - --o<|<< ck3
//       |
// - --  |
//     --o
//
//     <<o<< ck4
// - --o--
//     |
// - --o--
//
// - --o--
//     |
// - --o--
//     <<o<< ck5
*/
