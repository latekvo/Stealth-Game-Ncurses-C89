#ifndef ARENA_H
#define ARENA_H

#include "lurker.h"
#include "player.h"
#include "utils.h"

/* FIXME: Split Arena into ArenaGenerationState and ArenaState */

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

void init_arena(Arena* arena, Player* player);

void generate_arena(Arena* arena);

#endif
