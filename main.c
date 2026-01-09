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
  float azimuth_uvec_x, azimuth_uvec_y;
  float detection_cone_halfangle_rad;
  uint status;
} Lurker;

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

const uint ARENA_SIZE = 16;

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
  /* Map generation NOTES:
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
  */
  memset(arena->data, WALL, arena->size_x * arena->size_y * sizeof(uint));
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

    arena->lurkers[arena->lurker_count] = new_lurker;
    arena->lurker_count += 1;
  }
}

void update_lurkers(Lurker* lurkers, float time_delta) {
  /* NOTE: Need to use raycasting to prevent wall penetration
  // Would be pretty if rays were higher-res than walls, so directly to canvas
  // In patrol mode, add preference for the lurker to snap to multiples of 45d
  // Add jitter to lurker's rotation, this way it's more realistic and prettier
  // Energy remains constant, when moving slower, it will turns faster.
  */
}

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

    update_lurkers(arena.lurkers, time_delta);
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

      /* FIXME: 2x as workaround for 1:2 font dimensions
      // NOTE: This will be redundant once Canvas works
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
