#include <stdlib.h>

typedef unsigned int uint;

typedef struct {
  uint position_x, position_y;
} Player;

typedef struct {
  float position_x, position_y;
  float velocity_x, velocity_y;
  float azimuth_rad, cone_rad;
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

const uint ARENA_SIZE = 16;

int main() {
  Arena arena;
  Player player;
  Lurker* lurkers;

  arena.player = &player;

  arena.lurker_count = 0;
  arena.lurker_capacity = 16;
  arena.lurkers = malloc(arena.lurker_capacity * sizeof(Lurker));

  arena.size_x = ARENA_SIZE;
  arena.size_y = ARENA_SIZE;
  arena.data = malloc(ARENA_SIZE * ARENA_SIZE * sizeof(uint));

  /* TODO: Map generation
  // NOTES:
  // - I was initially imagining going for caves
  // - Rectangular office style could be more interesting
  // - Rectangle walls can be mangled to fall back to the cave idea
  // - (perlin is hard in C !!!)
  */

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
  }

  free(arena.data);
  free(arena.lurkers);

  return 0;
}
