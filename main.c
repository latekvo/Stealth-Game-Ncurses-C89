#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned int uint;

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
  uint size_x, size_y;
  uint* data;
  Player* player;
  Lurker* lurkers;
  uint lurker_count;
  uint lurker_capacity;
} Arena;

typedef struct {
  uint size_x, size_y;
  uint player_x, player_y; /* may differ from Arena, needed for View */
  uint enable_fog_of_war;
  uint enable_coloring;
  uint enable_shading;
  char* data;
} Canvas;

typedef struct {
  uint size_x, size_y;
  uint offset_x, offset_y;
} View;

/* TODO: Tile as a struct with properties */
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

typedef struct {
  uint center_x, center_y;
  uint growth_vel_x, growth_vel_y;
  uint radius_x, radius_y;
  uint total_door_count;
  uint block_l, block_r, block_t, block_b;
  uint is_player_spawn;
  uint is_end_objective_room;
} RoomSeed;

const uint ARENA_SIZE = 60;
const float PI = 3.14159;

void init_arena(Arena* arena, Player* player) {
  arena->player = player;

  arena->lurker_count = 0;
  arena->lurker_capacity = 16;
  arena->lurkers = malloc(arena->lurker_capacity * sizeof(Lurker));

  arena->size_x = ARENA_SIZE;
  arena->size_y = ARENA_SIZE;
  arena->data = malloc(ARENA_SIZE * ARENA_SIZE * sizeof(uint));
}

void generate_arena(Arena* arena) {
  uint total_v = arena->size_x * arena->size_y;
  uint avg_room_v = 12 * 12;
  float assumed_wall_ratio = 0.30;
  uint seed_count = round((float)total_v / avg_room_v * assumed_wall_ratio);
  uint edge_padding = 10;

  memset(arena->data, WALL, total_v * sizeof(uint));

  // TODO: Make sure points aren't too close together, will ignore this for now

  int i;
  for (i = 0; i < seed_count; i++) {
    uint pos_x = rand() % (arena->size_x - edge_padding * 2) + edge_padding;
    uint pos_y = rand() % (arena->size_y - edge_padding * 2) + edge_padding;
  }
}

void init_lurkers(Arena* arena) {
  int i;
  for (i = 0; i < arena->size_x * arena->size_y; i++) {
    if (arena->data[i] != LURKER_SPAWN) {
      continue;
    }

    // TODO: Move entire lurker init logic to map generation
    // - Removes need for LURKER_SPAWN, PLAYER_SPAWN, etc
    // - Just as simple in handling if we isolate to add_lurker

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
  // (do we need p.? it will change direction quickly either way)
  */

  int i;
  for (i = 0; i < lurker_count; i++) {
    Lurker* lurker = &lurkers[i];
    /* TBD */
  }
}

int main() {
  Arena arena;
  Player player;

  init_arena(&arena, &player);
  generate_arena(&arena);
  init_lurkers(&arena);

  float time_delta = 1.0;

  while (0) {
    /* TODO:
    // - Calc time_delta (but using it for sleep is probs good enough)
    // - User inputs
    // - Draw Canvas
    // - Draw View
    // - Add coloring according to ascii (probs simpler than metadata)
    // - Render (see if overwriting rather than redrawing is feasible)
    */

    update_lurkers(arena.lurkers, arena.lurker_count, time_delta);
  }

  /* DEBUG: Single frame render */
  int x, y;
  for (y = 0; y < ARENA_SIZE; y++) {
    for (x = 0; x < ARENA_SIZE; x++) {
      uint element = arena.data[x + y * ARENA_SIZE];
      char repr = '?';

      switch (element) {
        case FLOOR:
          repr = ' ';
          break;
        case WALL:
          repr = '#';
          break;
        case PLAYER_SPAWN:
          repr = 'P';
          break;
        case LURKER_SPAWN:
          repr = 'E';
          break;
        case SIDE_OBJECTIVE:
          repr = '$';
          break;
        case END_OBJECTIVE:
          repr = '@';
          break;
      }

      /* 2x as workaround for 1:2 font dimensions
      // This will be redundant once Canvas works
      */
      putchar(repr);
      putchar(repr);
    }

    putchar('\n');
  }

  free(arena.data);
  free(arena.lurkers);

  return 0;
}

// -=-=-=- Development note archive -=-=-=-

/* Map generation:
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
