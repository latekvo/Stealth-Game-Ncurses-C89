#include "debug.h"

#include <ncurses.h>

#include "colors.h"
#include "lurker.h"
#include "player.h"
#include "utils.h"

void print_fps(float time_delta) {
  uint hz = 1 / time_delta;
  move(0, 1);
  attron(COLOR_PAIR(TEXT_COLOR_CODE));
  printw("fps: %d", hz);
}

void print_frame_number() {
  static uint frame_number = 0;
  move(0, 12);
  attron(COLOR_PAIR(TEXT_COLOR_CODE));
  printw("frame: %d", frame_number);
  frame_number = (frame_number + 1) % 9999;
}

void print_player_data(Player* player) {
  move(player->position_y - 1, player->position_x * 2 + 2);
  attron(COLOR_PAIR(TEXT_COLOR_CODE));
  printw("x: %d y: %d", player->position_x, player->position_y);
}

void print_lurker_data(Lurker* lurkers, uint lurker_count) {
  uint i;
  for (i = 0; i < lurker_count; i++) {
    Lurker lurker = lurkers[i];
    move(lurker.position_y - 1, lurker.position_x * 2 + 1);
    attron(COLOR_PAIR(TEXT_COLOR_CODE));
    printw("x: %u y: %u, r_t: %f, r_c: %f", (uint)lurker.position_x,
           (uint)lurker.position_y, lurker.azimuth_target_rad,
           lurker.azimuth_current_rad);
  }
}
