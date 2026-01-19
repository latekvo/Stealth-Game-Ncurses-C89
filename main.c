#include <assert.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "arena.h"
#include "canvas.h"
#include "colors.h"
#include "debug.h"
#include "lurker.h"
#include "player.h"
#include "rays.h"
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
    new_lurker.velocity = 1.5;
    new_lurker.status = WALKING_OFFICE;

    arena->lurkers[arena->lurker_count] = new_lurker;
    arena->lurker_count += 1;
  }
}

void draw_lurkers(Canvas* canvas, Lurker* lurkers, uint lurker_count,
                  float time_delta) {
  uint i, pos;
  Lurker* lurker;
  CanvasTile* tile;
  for (i = 0; i < lurker_count; i++) {
    lurker = &lurkers[i];

    /* TODO: Draw over 4 tiles, not just 1 */

    pos = lurker->position_x * canvas->scale_x +
          lurker->position_y * canvas->scale_y * canvas->size_x;
    tile = &canvas->data[pos];
    tile->can_light_pass = 0;
    tile->color_code = EXIT_COLOR_CODE;
    tile->display_char = '@';
  }
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

int main() {
  Arena arena;
  Player player;
  Canvas canvas;
  /* View view; */

  /* TODO: Remove this, but for now this simplifies debug */
  setvbuf(stdout, NULL, _IONBF, 0);

  initscr();
  cbreak();
  noecho();

  init_colors();
  init_arena(&arena, &player);
  init_canvas(&canvas, &arena);
  generate_arena(&arena);
  init_lurkers(&arena);

  /* TD logic copied from:
  https://sourceware.org/glibc/manual/latest/html_mono/libc.html#Calculating-Elapsed-Time
  */

  clock_t start, end;
  float time_delta = 1.0;

  while (1) {
    start = clock();
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
    draw_lurkers(&canvas, arena.lurkers, arena.lurker_count, time_delta);

    print_canvas(&canvas);
    print_fps(time_delta);
    print_frame_number();

    refresh();

    end = clock();
    time_delta = ((float)(end - start)) / CLOCKS_PER_SEC;
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

/*
// In C, i have struct Foo in file foo.h, as well as init_foo(Foo *foo);
// Struct Foo references a list of Bar
*/
