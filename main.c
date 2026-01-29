#include <assert.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "arena.h"
#include "arena_drawing.h"
#include "canvas.h"
#include "colors.h"
#include "debug.h"
#include "input_handling.h"
#include "lurker_drawing.h"
#include "lurker_logic.h"
#include "player.h"
#include "player_drawing.h"
#include "rays.h"

int main() {
  Arena arena;
  Player player = {10, 10};
  Canvas canvas;
  /* View view; */

  /* TODO: Remove this, but for now this simplifies debug */
  setvbuf(stdout, NULL, _IONBF, 0);

  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);

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

    int input;
    while ((input = getch()) != ERR) {
      switch (input) {
        case 'q':
          goto end_game_loop;
          break;
        default:
          handle_input(&arena, input);
          break;
      }
    }

    /* TODO:
    // - Calc time_delta (but using it for sleep is probs good enough)
    // - User inputs (i'm reading you can add timeout to getch())
    // - Draw Canvas
    // - Draw View
    // - Add coloring according to ascii (probs simpler than metadata)
    // - Render (see if overwriting rather than redrawing is feasible)
    */

    update_lurkers(&arena, time_delta);

    draw_arena(&canvas, &arena);
    draw_player(&canvas, &arena);
    draw_lurker_rays(&canvas, &arena);
    draw_lurkers(&canvas, arena.lurkers, arena.lurker_count, time_delta);

    print_canvas(&canvas);
    print_fps(time_delta);
    print_frame_number();
    print_player_data(&player);

    refresh();

    end = clock();
    time_delta = ((float)(end - start)) / CLOCKS_PER_SEC;
  }

end_game_loop:

  /* TODO: Check if we really have to free if we're exiting anyways */
  free(arena.data);
  free(arena.lurkers);
  free(canvas.data);

  getch();
  endwin();

  printf("\nGame ended by player input.\n");
  fflush(stdout);

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

/* In patrol mode, add preference for the lurker to snap to multiples of 45d
// Add jitter to lurker's rotation, this way it's more realistic and prettier
// Energy remains constant, when moving slower, it will turns faster.
// 1. When azimuth target far off from actual, slow down
// 2. As lurker is slow, it can rotate faster
// 3. As alignment increases, so does speed
// NOTE: Using accel would yield more realistic look, i can imagine stiff rot
// looking weird, unnatural, will fix this with azimuth target jitter
// Is p-derived rebound needed? No, direction changes quickly either way
// NOTE: Also try playing around with halfangle variance
// - During rotations, decreasing it make things easier
// - Decreasing it with speed/total-energy would make thing more realistic
*/
