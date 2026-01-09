typedef unsigned int uint;

typedef struct {
  uint position_x, position_y;
} Player;

typedef struct {
  uint position_x, position_y;
} Lurker;

typedef struct {
  uint size_x, size_y;
  uint* data;
  Player player;
  Lurker* lurkers;
} Arena;

typedef struct {
  uint size_x, size_y;
  uint player_x, player_y;  // may differ from Arena, needed for View
  uint enable_fog_of_war;
  uint enable_coloring;
  uint enable_shading;
  char* data;
} Canvas;

typedef struct {
  uint size_x, size_y;
  uint offset_x, offset_y;
} View;

// Arena:
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

int main() {}
